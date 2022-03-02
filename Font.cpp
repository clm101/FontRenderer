#include "Font.h"

#include <iostream>

#include <clmUtil/clm_err.h>
#include <clmUtil/clm_concepts_ext.h>
#include <clmMath/clm_geo.h>

#include <Mesh.h>

namespace clm {
	Font::Font(std::string fontName, const float pointSize)
		:
		Font()
	{
		// fontName should be the name of the requested font sans .ttf
		m_fontName = fontName;
		m_pointSize = pointSize;
		m_fileName = std::format("C:/Windows/Fonts/{}.ttf", fontName);

		File fontFile{};
		try
		{
			fontFile = std::move(File::open_file(m_fileName, Endian::Big));
			if (fontFile.size() < (sizeof(OffsetTable) + 8 * sizeof(TableRecord)))
			{
				throw std::runtime_error{ std::format("Font file for {} is malformed.\nFile: {}\n", m_fontName, m_fileName) };
			}
		}
		catch (DWORD d)
		{
			throw std::runtime_error{ std::format("Error opening font: {}\nFile name: {}\nError: {}\n", m_fontName, m_fileName, d) };
		}

		create_offset_table(fontFile);
		create_table_records(fontFile);
		validate_font(fontFile);
		create_maximum_profile_table(fontFile);
		create_font_header_table(fontFile);
		create_glyph_mapping(fontFile);
		triangulate_characters();
	}

	CurveSet Font::get_glyph(const wchar_t character) noexcept
	{
		auto glyphMapIter = m_charGlyphMap.find(character);
		const GlyphDesc::SimpleGlyphDesc& glyphDesc = (glyphMapIter != m_charGlyphMap.end() ? glyphMapIter->second.desc : m_missingGlyph.desc);

		return get_curve_set(glyphDesc);
	}

	CurveSet Font::get_curve_set(const GlyphDesc::SimpleGlyphDesc& glyphDesc) const noexcept
	{
		CurveSet glyph{};
		uint16_t startIndex = 0;
		float unitsPerEm = static_cast<float>(m_fontHeaderTable.unitsPerEm);
		for (size_t i = 0; i < glyphDesc.endPtsOfContours.size(); i++)
		{
			uint16_t lastIndex = glyphDesc.endPtsOfContours[i];
			glyph.emplace_back();
			for (uint16_t j = startIndex; j <= lastIndex; j++)
			{
				glyph.rbegin()->emplace_back(glyphDesc.points[j].flag & ON_CURVE_POINT,
											 GFXPointType{static_cast<float>(glyphDesc.points[j].data[0]) / unitsPerEm,
											 -1.0f * static_cast<float>(glyphDesc.points[j].data[1]) / unitsPerEm});
			}
			startIndex = lastIndex + 1;
		}
		return glyph;
	}

	Font::TRIter Font::read_from_record_table(std::string tableName)
	{
		TRIter table = std::find_if(m_tableRecords.begin(),
									m_tableRecords.end(),
									[&tableName](const TableRecord& tr)
									{
										return util::compare(tr.tableTag, tableName);
									});
		if (table == m_tableRecords.end())
		{
			throw std::runtime_error{ std::format("Error finding {} table for font: {}\n", tableName, m_fontName) };
		}

		return table;
	}

	void Font::create_offset_table(const File& fontFile)
	{
		// Get header
		fontFile >> m_offsetTable.scalarType;
		fontFile >> m_offsetTable.numTables;
		fontFile >> m_offsetTable.searchRange;
		fontFile >> m_offsetTable.entrySelector;
		fontFile >> m_offsetTable.rangeShift;

		verify_offset_table_vals();
	}

	void Font::verify_offset_table_vals()
	{
		uint16_t entrySelectorCheck = static_cast<uint16_t>(floor(log2(m_offsetTable.numTables)));
		uint16_t searchRangeCheck = (1 << entrySelectorCheck) * 16;
		uint16_t rangeShiftCheck = m_offsetTable.numTables * 16 - searchRangeCheck;

		if (entrySelectorCheck != m_offsetTable.entrySelector ||
			searchRangeCheck != m_offsetTable.searchRange ||
			rangeShiftCheck != m_offsetTable.rangeShift)
		{
			throw std::runtime_error{ "Offset table mismatch, potential malformed font.\n" };
		}
	}

	void Font::create_table_records(const File& fontFile) noexcept(util::release)
	{
		m_tableRecords.resize(m_offsetTable.numTables);
		for (size_t i = 0; i < m_tableRecords.size(); i++)
		{
			m_tableRecords[i].tableTag.resize(4);
			fontFile >> m_tableRecords[i].tableTag;
			fontFile >> m_tableRecords[i].checksum;
			fontFile >> m_tableRecords[i].offset;
			fontFile >> m_tableRecords[i].length;
		}
		std::sort(m_tableRecords.begin(),
				  m_tableRecords.end(),
				  [](const TableRecord& lhs, const TableRecord& rhs)
				  {
					  return lhs.offset < rhs.offset;
				  });
	}

	uint32_t Font::calc_checksum(const File& fontFile, uint32_t offset, size_t length) const noexcept
	{
		const size_t count = (length + 3) / sizeof(uint32_t);
		uint32_t val = 0;
		uint64_t sum = 0;
		[[unlikely]] if (offset == 0)
		{
			fontFile.set_position(0);
		}

		for (size_t i = 0; i < count; i++)
		{
			fontFile.get_data_raw(val, offset);
			change_endian(val);
			sum += static_cast<uint64_t>(val);
			offset += sizeof(uint32_t);
		}
		return static_cast<uint32_t>(sum);
	}

	uint32_t Font::get_checksum_adjustment(const File& fontFile, size_t headTableOffset) const
	{
		uint32_t checksumAdjustment = 0;
		fontFile.get_data(checksumAdjustment, headTableOffset + 8);
		return checksumAdjustment;
	}

	void Font::validate_font(const File& fontFile)
	{
		// Check checksums
		uint32_t checksumAdjustment = 0;
		for (const auto& tr : m_tableRecords)
		{
			[[unlikely]] if (util::compare(tr.tableTag, "head"))
			{
				checksumAdjustment = get_checksum_adjustment(fontFile, tr.offset);
				uint32_t checksum = calc_checksum(fontFile, tr.offset, tr.length);
				checksum -= checksumAdjustment;
				if (tr.checksum != checksum)
				{
					throw std::runtime_error{ std::format("Font file error for {}: mismatched checksum error.\n\
										Table: {}\nFile: {}", m_fontName, tr.tableTag, m_fileName) };
				}
			}
			else
			{
				if (tr.checksum != calc_checksum(fontFile, tr.offset, tr.length))
				{
					throw std::runtime_error{ std::format("Font file error for {}: mismatched checksum error.\n\
										Table: {}\nFile: {}", m_fontName, tr.tableTag, m_fileName) };
				}
			}
		}
		uint32_t fontChecksum = calc_checksum(fontFile, 0, fontFile.size());
		fontChecksum -= checksumAdjustment;
		if (checksumAdjustment != (0xB1B0AFBA - fontChecksum))
		{
			throw std::runtime_error{ std::format("Font file error for {}: mismatched font file checksum error.\n\
										File: {}", m_fontName, m_fileName) };
		}

		// Make sure the required tables are present
		using namespace std::string_literals;
		const std::vector<std::string> requiredTables{ "cmap"s,
		"head"s, "hhea"s, "hmtx"s, "maxp"s, "name"s, "OS/2"s, "post"s, "glyf"s, "loca"s };
		for (const auto& t : requiredTables)
		{
			if (std::find_if(m_tableRecords.begin(),
							 m_tableRecords.end(),
							 [&t](const TableRecord& tr)
							 {
								 return util::compare(t, tr.tableTag);
							 }) == m_tableRecords.end())
			{
				throw std::runtime_error{ std::format("Missing {} table in font {}\nFile: {}\n", t, m_fontName, m_fileName) };
			}
		}
	}

	void Font::create_maximum_profile_table(const File& fontFile) noexcept(util::release)
	{
		uint32_t offset = read_from_record_table("maxp")->offset;
		fontFile.set_position(offset);
		fontFile >> m_maximumProfileTable.version;
		fontFile >> m_maximumProfileTable.numGlyphs;
		fontFile >> m_maximumProfileTable.maxPoints;
		fontFile >> m_maximumProfileTable.maxContours;
		fontFile >> m_maximumProfileTable.maxCompositePoints;
		fontFile >> m_maximumProfileTable.maxCompositeContours;
		fontFile >> m_maximumProfileTable.maxZones;
		fontFile >> m_maximumProfileTable.maxTwilightPoints;
		fontFile >> m_maximumProfileTable.maxStorage;
		fontFile >> m_maximumProfileTable.maxFunctionDefs;
		fontFile >> m_maximumProfileTable.maxInstructionDefs;
		fontFile >> m_maximumProfileTable.maxStackElements;
		fontFile >> m_maximumProfileTable.maxSizeOfInstructions;
		fontFile >> m_maximumProfileTable.maxComponentElements;
		fontFile >> m_maximumProfileTable.maxComponentDepth;
	}

	void Font::create_font_header_table(const File& fontFile) noexcept(util::release)
	{
		uint32_t offset = read_from_record_table("head")->offset;
		fontFile.set_position(offset);
		fontFile >> m_fontHeaderTable.majorVersion;
		fontFile >> m_fontHeaderTable.minorVersion;
		fontFile >> m_fontHeaderTable.fontRevision;
		fontFile >> m_fontHeaderTable.checksumAdjustment;
		fontFile >> m_fontHeaderTable.magicNumber;
		fontFile >> m_fontHeaderTable.flags;
		fontFile >> m_fontHeaderTable.unitsPerEm;
		fontFile >> m_fontHeaderTable.created;
		fontFile >> m_fontHeaderTable.modified;
		fontFile >> m_fontHeaderTable.xMin;
		fontFile >> m_fontHeaderTable.yMin;
		fontFile >> m_fontHeaderTable.xMax;
		fontFile >> m_fontHeaderTable.yMax;
		fontFile >> m_fontHeaderTable.macStyle;
		fontFile >> m_fontHeaderTable.lowestRecPPEM;
		fontFile >> m_fontHeaderTable.fontDirectionHint;
		fontFile >> m_fontHeaderTable.indexToLocFormat;
		fontFile >> m_fontHeaderTable.glyphDataFormat;
	}

	Font::CGIMT Font::create_cgmit(const File& fontFile) noexcept(util::release)
	{
		uint32_t offset = read_from_record_table("cmap")->offset;
		fontFile.set_position(offset);
		CGIMT cmapTable{};
		fontFile >> cmapTable.version;
		fontFile >> cmapTable.numTables;
		cmapTable.encodingRecords.resize(cmapTable.numTables);
		for (auto& elem : cmapTable.encodingRecords)
		{
			fontFile >> elem.platformID;
			fontFile >> elem.encodingID;
			fontFile >> elem.subtableOffset;
		}
		return cmapTable;
	}

	Font::IndexLocationTable Font::create_index_location_table(const File& fontFile) noexcept(util::release)
	{
		const uint32_t offset = read_from_record_table("loca")->offset;
		const size_t offsetVectorSize = static_cast<size_t>(m_maximumProfileTable.numGlyphs) + 1;
		IndexLocationTable indexLocationTable{};
		indexLocationTable.offsets.resize(offsetVectorSize);
		indexLocationTable.shortVersion = m_fontHeaderTable.indexToLocFormat == 0;
		fontFile.set_position(offset);
		if (m_fontHeaderTable.indexToLocFormat == 0)
		{
			// Data is uint16_t, not uint32_t
			fontFile.fill_vec<uint16_t>(indexLocationTable.offsets, offset);
		}
		else
		{
			fontFile >> indexLocationTable.offsets;
		}
		return indexLocationTable;
	}

	Font::CMapSubtable4 Font::create_cmap_subtable(const File& fontFile, const CGIMT& cmapTable)
	{
		using Iter = std::vector<Font::CharacterGlyphIndexMappingTable::EncodingRecord>::const_iterator;
		Iter subtableInfo = std::find_if(cmapTable.encodingRecords.begin(),
										 cmapTable.encodingRecords.end(),
										 [](const CGIMT::EncodingRecord& rec)
										 {
											 return rec.platformID == 3 && rec.encodingID == 1;
										 });
		if (subtableInfo == cmapTable.encodingRecords.end())
		{
			throw std::runtime_error{ std::format("Incompatible font(cmap subtable 4 not present): {}\nFile name: {}\n", m_fontName, m_fileName) };
		}
		uint32_t cmapOffset = read_from_record_table("cmap")->offset;
		uint32_t subtableOffset = cmapOffset + subtableInfo->subtableOffset;
		fontFile.set_position(subtableOffset);
		CMapSubtable4 cmapSubtable{};
		fontFile >> cmapSubtable.format;
		fontFile >> cmapSubtable.length;
		fontFile >> cmapSubtable.language;
		fontFile >> cmapSubtable.segCountX2;
		fontFile >> cmapSubtable.searchRange;
		fontFile >> cmapSubtable.entrySelector;
		fontFile >> cmapSubtable.rangeShift;

		uint16_t segCount = cmapSubtable.segCountX2 >> 1;
		uint16_t searchRangeCheck = 2 << static_cast<uint16_t>(floor(log2(segCount)));
		uint16_t entrySelectorCheck = static_cast<uint16_t>(log2(cmapSubtable.searchRange >> 1));
		uint16_t rangeShiftCheck = (segCount << 1) - cmapSubtable.searchRange;
		if (searchRangeCheck != cmapSubtable.searchRange ||
			entrySelectorCheck != cmapSubtable.entrySelector ||
			rangeShiftCheck != cmapSubtable.rangeShift)
		{
			throw std::runtime_error{ std::format("Error reading font while constructing cmap subtable 4 for font {}\nFile name: {}\n", m_fontName, m_fileName) };
		}

		cmapSubtable.endCodes.resize(segCount);
		cmapSubtable.startCodes.resize(segCount);
		cmapSubtable.idDeltas.resize(segCount);
		cmapSubtable.idRangeOffsets.resize(segCount);
		const size_t currentFilePos = fontFile.get_position();
		err::assert<std::runtime_error>(currentFilePos <= (static_cast<size_t>(cmapOffset) +
															static_cast<size_t>(subtableOffset) +
															static_cast<size_t>(cmapSubtable.length)),
										 std::format("Overshot font file for font {}\nFile name: {}\n",
													 m_fontName,
													 m_fileName));
		const size_t remainder = static_cast<size_t>(cmapSubtable.length) +
			static_cast<size_t>(cmapOffset) +
			static_cast<size_t>(subtableOffset) -
			currentFilePos;
		cmapSubtable.glyphIDArray.resize(remainder);

		fontFile >> cmapSubtable.endCodes;
		fontFile >> cmapSubtable.reservedPad;
		fontFile >> cmapSubtable.startCodes;
		fontFile >> cmapSubtable.idDeltas;
		cmapSubtable.rangeOffsetStartAddr = fontFile.get_position();
		fontFile >> cmapSubtable.idRangeOffsets;
		fontFile >> cmapSubtable.glyphIDArray;

		return cmapSubtable;
	}

	void Font::create_glyph_mapping(const File& fontFile) noexcept(util::release)
	{
		IndexLocationTable locaTable = create_index_location_table(fontFile);
		const uint16_t locaTableMultiplier = locaTable.shortVersion ? 2 : 1;
		CGIMT cmapTable = create_cgmit(fontFile);
		CMapSubtable4 cmapSubtable = create_cmap_subtable(fontFile, cmapTable);
		const uint32_t glyfTableOffset = read_from_record_table("glyf")->offset;

		const auto create_glyph_desc = [&](uint32_t glyfTableEntryOffset) -> GlyphDesc
		{
			using flag_t = uint8_t;
			using coord_t = int16_t;
			using short_coord_t = uint8_t;
			using long_coord_t = int16_t;

			// Populate GlyphDesc variable
			GlyphDesc glyphData{};
			fontFile.set_position(static_cast<size_t>(glyfTableOffset) + static_cast<size_t>(glyfTableEntryOffset));
			fontFile >> glyphData.header.numberOfContours;
			fontFile >> glyphData.header.xMin;
			fontFile >> glyphData.header.yMin;
			fontFile >> glyphData.header.xMax;
			fontFile >> glyphData.header.yMax;
			err::assert<std::runtime_error>(glyphData.header.numberOfContours >= 0, "Composite glyph description is not supported yet");

			glyphData.desc.endPtsOfContours.resize(glyphData.header.numberOfContours);
			fontFile >> glyphData.desc.endPtsOfContours;
			fontFile >> glyphData.desc.instructionLength;
			if (glyphData.desc.instructionLength > 0)
			{
				glyphData.desc.instructions.resize(glyphData.desc.instructionLength);
				fontFile >> glyphData.desc.instructions;
			}

			std::vector<flag_t> flags{};
			flags.resize(static_cast<size_t>(*(glyphData.desc.endPtsOfContours.rbegin())) + 1);
			{
				// Get flags
				size_t i = 0;
				while (i < flags.size())
				{
					flag_t flag{};
					fontFile >> flag;
					uint8_t additionalIterations{};
					if (flag & REPEAT_FLAG)
					{
						fontFile >> additionalIterations;
						err::assert<std::runtime_error>(i + static_cast<size_t>(additionalIterations) <= flags.size(),
														 "More flags than there are points");
					}

					size_t j = 0;
					do
					{
						flags[i] = flag;
						i += 1;
					} while (++j <= additionalIterations);
				}
			}
			// Get points
			const auto parse_flag = [&fontFile](flag_t flag,
												coord_t& delta,
												coord_t& dest,
												const uint8_t byteFlag,
												const uint8_t deltaFlag)
			{
				if (flag & byteFlag)
				{
					short_coord_t val = 0;
					fontFile >> val;
					if (flag & deltaFlag)
					{
						delta += static_cast<coord_t>(val);
					}
					else
					{
						delta -= static_cast<coord_t>(val);
					}
					dest = delta;
				}
				else
				{
					if (flag & deltaFlag)
					{
						dest = delta;
					}
					else
					{
						long_coord_t val = 0;
						fontFile >> val;
						delta += val;
						dest = delta;
					}
				}
			};

			std::vector<coord_t> xCoords{};
			xCoords.resize(flags.size());
			coord_t xCoordTracker = 0;
			for (size_t i = 0; i < flags.size(); i += 1)
			{
				parse_flag(flags[i],
						   xCoordTracker,
						   xCoords[i],
						   X_SHORT_VECTOR,
						   X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR);
			}

			std::vector<coord_t> yCoords{};
			yCoords.resize(flags.size());
			coord_t yCoordTracker = 0;
			for (size_t i = 0; i < flags.size(); i += 1)
			{
				parse_flag(flags[i],
						   yCoordTracker,
						   yCoords[i],
						   Y_SHORT_VECTOR,
						   Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR);
			}

			const auto calculate_missing_on_curve_points_and_combine = [](std::vector<uint16_t>& endOfContourIndices,
																		  const std::vector<flag_t>& flags,
																		  const std::vector<coord_t>& xCoords,
																		  const std::vector<coord_t>& yCoords) -> std::vector<FontPoint>
			{
				//GlyphDesc::SimpleGlyphDesc desc = descInOut;
				//FontPointList& pointList = desc.points;

				std::vector<FontPoint> fontPoints; 
				fontPoints.reserve(flags.size());

				// May or may not be needed
				//{
				//	uint16_t startIndex = 0;
				//	uint16_t change = 0;
				//	// Why do I do this?
				//	for (auto& endIndex : desc.endPtsOfContours)
				//	{
				//		// If the first point is not on the curve
				//		[[unlikely]] if ((pointList[startIndex].flag & ON_CURVE_POINT) != ON_CURVE_POINT)
				//		{
				//			// If the last point is off-curve, make the first point in pointList be on curve
				//			if ((pointList[endIndex].flag & ON_CURVE_POINT) != ON_CURVE_POINT)
				//			{
				//				// Not sure why this uses midpoint
				//				pointList.emplace(pointList.begin() + startIndex,
				//								  ON_CURVE_POINT,
				//								  math::midpoint(pointList[startIndex].data, pointList[endIndex].data));
				//				++change;
				//			}
				//			else
				//			{
				//				// Otherwise, move the last point to be the first point
				//				pointList.insert(pointList.begin() + startIndex, *(pointList.begin() + endIndex + change));
				//				pointList.erase(pointList.begin() + endIndex + change + 1);
				//			}
				//		}
				//		endIndex += change;
				//		startIndex = endIndex + 1;
				//	}
				//}
				
				// Fill-in understood on-curve points
				size_t start = 0;
				uint16_t newVertexCount = 0;
				for (uint16_t& endIndex : endOfContourIndices)
				{
					const size_t end = static_cast<size_t>(endIndex) + 1;
					for (size_t i = start; i < (end - 1); i += 1)
					{
						fontPoints.emplace_back(flags[i],
												math::Point<int16_t, 2>{xCoords[i], yCoords[i]});
						if (((flags[i] & ON_CURVE_POINT) != ON_CURVE_POINT) &&
							((flags[i + 1] & ON_CURVE_POINT) != ON_CURVE_POINT))
						{
							const math::Point<int16_t, 2> p0{xCoords[i], yCoords[i]};
							const math::Point<int16_t, 2> p1{xCoords[i + 1], yCoords[i + 1]};
							fontPoints.emplace_back(ON_CURVE_POINT,
													math::midpoint(p0, p1));
							newVertexCount += 1;
						}
					}
					fontPoints.emplace_back(flags[endIndex],
											math::Point<int16_t, 2>{xCoords[endIndex], yCoords[endIndex]});
					if (((flags[start] & ON_CURVE_POINT) != ON_CURVE_POINT) &&
						((flags[endIndex] & ON_CURVE_POINT) != ON_CURVE_POINT))
					{
						const math::Point<int16_t, 2> p0{xCoords[start], yCoords[start]};
						const math::Point<int16_t, 2> p1{xCoords[endIndex], yCoords[endIndex]};
						fontPoints.emplace_back(ON_CURVE_POINT,
												math::midpoint(p0, p1));
						newVertexCount += 1;
					}
					endIndex += newVertexCount;
					start = end;
				}
				
				return fontPoints;
			};

			glyphData.desc.points = std::move(calculate_missing_on_curve_points_and_combine(glyphData.desc.endPtsOfContours,
																							flags,
																							xCoords,
																							yCoords));
			return glyphData;
		};

		const auto create_glyph_desc_and_insert = [&](const wchar_t letter)
		{
			[[unlikely]] if (letter == u'\0') return;
			uint16_t c = std::bit_cast<uint16_t>(letter);

			const auto get_index = [&]() -> size_t
			{
				size_t i = 0;
				for (i; i < cmapSubtable.endCodes.size(); i++)
				{
					if (c > cmapSubtable.endCodes[i]) continue;
					else break;
				}
				return i;
			};

			size_t i = get_index();
			if (i == cmapSubtable.endCodes.size() - 1)
			{
				return;
			}

			if (cmapSubtable.startCodes[i] <= c)
			{
				uint32_t glyfTableEntryOffset = 0;
				uint16_t locaTableOffset = 0;
				if (cmapSubtable.idRangeOffsets[i] != 0)
				{
					// Determine offset into glyphIDArray
					const uint16_t cOffset = c - cmapSubtable.startCodes[i];
					const uint16_t glyphIDOffset = (cmapSubtable.idRangeOffsets[i] >> 1) + cOffset +
						static_cast<const uint16_t>(i) - static_cast<uint16_t>(cmapSubtable.idRangeOffsets.size());
					locaTableOffset = cmapSubtable.glyphIDArray[glyphIDOffset];
					// If the offset is not 0, then add the corresponding idDelta value modulo 65536
					if (locaTableOffset != 0)
					{
						locaTableOffset += cmapSubtable.idDeltas[i];
					}
					else
					{
						// Missing glyph
						return;
					}
					locaTableOffset &= 0xFFFF;
				}
				else
				{
					locaTableOffset = (c + cmapSubtable.idDeltas[i]) & 0xFFFF;
				}
				glyfTableEntryOffset = locaTableMultiplier * locaTable.offsets[locaTableOffset];

				err::assert<std::runtime_error>(glyfTableEntryOffset != 0,
												 "Should not be adding missing glyph to charGlyphMap.");

				m_charGlyphMap.insert({ letter, create_glyph_desc(glyfTableEntryOffset) });
			}
		};

		m_missingGlyph = create_glyph_desc(0);
		for (const auto& elem : keyToWChar)
		{
			create_glyph_desc_and_insert(elem.second);
			create_glyph_desc_and_insert(shift_down(elem.first));
		}
	}

	void Font::triangulate_characters()
	{
		m_missingGlyphMesh = std::move(get_on_curve_mesh(get_curve_set(m_missingGlyph.desc)));
		for (const auto& elem : keyToWChar)
		{
			const Key characterKey = elem.first;
			const wchar_t character = elem.second;
			CurveSet outline = get_glyph(character);
			m_characterMeshMap.emplace(character, std::move(get_on_curve_mesh(outline)));
			outline = get_glyph(shift_down(characterKey));
			m_characterMeshMap.emplace(shift_down(characterKey), std::move(get_on_curve_mesh(outline)));
		}
	}

	DelaunayMesh Font::get_on_curve_mesh(const CurveSet& characterCurves) const
	{
		std::vector<point_t> points{};
		for (size_t i = 0; i < characterCurves.size(); i += 1)
		{
			if (characterCurves.size() != 0)
			{
				for (size_t j = 0; j < characterCurves[i].size(); j += 1)
				{
					const GFXFontPoint& fontPoint = characterCurves[i][j];
					if (fontPoint.onCurve)
					{
						if (points.size() == 0)
						{
							points.push_back(fontPoint.data);
						}
						else
						{
							// If they're close enough to compare equal...
							const point_t& meshPoint = *(points.rbegin());
							if (!(meshPoint[0] == fontPoint.data[0] &&
								  meshPoint[1] == fontPoint.data[1]))
							{
								points.push_back(fontPoint.data);
							}
						}
					}
				}
			}
		}

		return DelaunayMesh{points};
	};

	std::vector<font_triangle_t> Font::get_triangles(const wchar_t character) noexcept
	{
		const auto meshIter = m_characterMeshMap.find(character);
		const DelaunayMesh& mesh = meshIter != m_characterMeshMap.end() ? meshIter->second : m_missingGlyphMesh;
		std::vector<triangle_t> meshTriangles = std::move(mesh.get_triangles());
		const std::vector<point_t>& meshPoints = mesh.get_points();
		std::vector<font_triangle_t> fontTriangles{};
		fontTriangles.reserve(meshTriangles.size());

		const auto has_ghost_vertex = [](const point_t& p0,
										 const point_t& p1,
										 const point_t& p2) -> bool
		{
			return clm::util::disjunction(is_ghost(p0),
										  is_ghost(p1),
										  is_ghost(p2));
		};

		for (const auto& triangle : meshTriangles)
		{
			const auto& vertices = triangle.get_points();
			if (!has_ghost_vertex(meshPoints[vertices[0]],
								  meshPoints[vertices[1]],
								  meshPoints[vertices[2]]))
			{
				fontTriangles.emplace_back(meshPoints[vertices[0]],
										   meshPoints[vertices[1]],
										   meshPoints[vertices[2]]);
			}
		}

		return fontTriangles;
	}
}