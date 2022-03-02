#ifndef FONT_H
#define FONT_H
#include <vector>
#include <string>
#include <format>
#include <unordered_map>
#include <exception>
#include <algorithm>
#include <bit>

#include <clmMath/clm_vector.h>
#include <clmUtil/clm_util.h>

#include <File.h>
#include <Keyboard.h>
#include <Delaunay.h>

typedef unsigned long DWORD;

constexpr const uint8_t ON_CURVE_POINT = 0x01;
constexpr const uint8_t X_SHORT_VECTOR = 0x02;
constexpr const uint8_t Y_SHORT_VECTOR = 0x04;
constexpr const uint8_t REPEAT_FLAG = 0x08;
constexpr const uint8_t X_IS_SAME_OR_POSITIVE_X_SHORT_VECTOR = 0x10;
constexpr const uint8_t Y_IS_SAME_OR_POSITIVE_Y_SHORT_VECTOR = 0x20;
constexpr const uint8_t OVERLAP_SIMPLE = 0x40;

namespace clm {
	using font_triangle_t = std::tuple<point_t, point_t, point_t>;

	struct FontPoint {
		uint8_t flag;
		math::Point<int16_t, 2> data;
	};

	using GFXPointType = math::Point2f;
	struct GFXFontPoint {
		bool onCurve;
		GFXPointType data;
	};
	using PointList = std::vector<GFXFontPoint>;
	using CurveSet = std::vector<PointList>;

	class Font {
	public:
		Font() noexcept = default;
		Font(std::string, const float);
		~Font() = default;
		Font(const Font&) noexcept = default;
		Font(Font&&) noexcept = default;
		Font& operator=(Font&) noexcept = default;
		Font& operator=(Font&&) noexcept = default;

		void set_pointsize(const float pointSize) noexcept { m_pointSize = pointSize; }
		CurveSet get_glyph(const wchar_t) noexcept;
		std::vector<font_triangle_t> get_triangles(const wchar_t) noexcept;
	private:
		uint32_t calc_checksum(const File&, uint32_t, size_t) const noexcept;
		uint32_t get_checksum_adjustment(const File&, size_t) const;
		void validate_font(const File&);
		void create_offset_table(const File&);
		void verify_offset_table_vals();
		void create_table_records(const File&) noexcept(util::release);
		void create_maximum_profile_table(const File&) noexcept(util::release);
		void create_font_header_table(const File&) noexcept(util::release);

		void triangulate_characters();
		DelaunayMesh get_on_curve_mesh(const CurveSet&) const;

		struct CharacterGlyphIndexMappingTable {
			uint16_t version;
			uint16_t numTables;
			struct EncodingRecord {
				uint16_t platformID;
				uint16_t encodingID;
				uint32_t subtableOffset;
			};
			std::vector<EncodingRecord> encodingRecords;
		};
		using CGIMT = CharacterGlyphIndexMappingTable;
		CGIMT create_cgmit(const File&) noexcept(util::release);

		struct CMapSubtable4 {
			uint16_t format;
			uint16_t length;
			uint16_t language;
			uint16_t segCountX2;
			uint16_t searchRange;
			uint16_t entrySelector;
			uint16_t rangeShift;
			std::vector<uint16_t> endCodes;
			uint16_t reservedPad;
			std::vector<uint16_t> startCodes;
			std::vector<int16_t> idDeltas;
			std::vector<uint16_t> idRangeOffsets;
			std::vector<uint16_t> glyphIDArray;
			size_t rangeOffsetStartAddr;
		};
		CMapSubtable4 create_cmap_subtable(const File&, const CGIMT&);

		struct IndexLocationTable {
			bool shortVersion = false;
			std::vector<uint32_t> offsets{};
		};
		IndexLocationTable create_index_location_table(const File&) noexcept(util::release);

		void create_glyph_mapping(const File&) noexcept(util::release);

		struct TableRecord;
		using TRIter = std::vector<TableRecord>::iterator;
		TRIter read_from_record_table(std::string);

		std::string m_fontName;
		std::string m_fileName;
		float m_pointSize;
		struct TableRecord {
			std::string tableTag = "";
			uint32_t checksum = 0;
			uint32_t offset = 0;
			uint32_t length = 0;
		};
		std::vector<TableRecord> m_tableRecords;
		struct MaximumProfileTable {
			uint32_t version = 0;
			uint16_t numGlyphs = 0;
			uint16_t maxPoints = 0;
			uint16_t maxContours = 0;
			uint16_t maxCompositePoints = 0;
			uint16_t maxCompositeContours = 0;
			uint16_t maxZones = 0;
			uint16_t maxTwilightPoints = 0;
			uint16_t maxStorage = 0;
			uint16_t maxFunctionDefs = 0;
			uint16_t maxInstructionDefs = 0;
			uint16_t maxStackElements = 0;
			uint16_t maxSizeOfInstructions = 0;
			uint16_t maxComponentElements = 0;
			uint16_t maxComponentDepth = 0;
		} m_maximumProfileTable;
		struct FontHeaderTable {
			uint16_t majorVersion = 0;
			uint16_t minorVersion = 0;
			int32_t fontRevision = 0;
			uint32_t checksumAdjustment = 0;
			uint32_t magicNumber = 0;
			uint16_t flags = 0;
			uint16_t unitsPerEm = 0;
			int64_t created = 0;
			int64_t modified = 0;
			int16_t xMin = 0;
			int16_t yMin = 0;
			int16_t xMax = 0;
			int16_t yMax = 0;
			uint16_t macStyle = 0;
			uint16_t lowestRecPPEM = 0;
			int16_t fontDirectionHint = 0;
			int16_t indexToLocFormat = 0;
			int16_t glyphDataFormat = 0;
		} m_fontHeaderTable;
		struct GlyphDesc {
			struct GlyphHeader {
				int16_t numberOfContours{};
				int16_t xMin{};
				int16_t yMin{};
				int16_t xMax{};
				int16_t yMax{};
			} header;

			struct SimpleGlyphDesc {
				bool byteCoordinates{ true };
				std::vector<uint16_t> endPtsOfContours{};
				uint16_t instructionLength{};
				std::vector<uint8_t> instructions{};
				std::vector<FontPoint> points{};

				//SimpleGlyphDesc() = default;
				//~SimpleGlyphDesc() = default;
			} desc;
		} m_missingGlyph;
		std::unordered_map<wchar_t, GlyphDesc> m_charGlyphMap;
		CurveSet get_curve_set(const GlyphDesc::SimpleGlyphDesc&) const noexcept;

		struct OffsetTable {
			std::uint32_t scalarType = 0;
			std::uint16_t numTables = 0;
			std::uint16_t searchRange = 0;
			std::uint16_t entrySelector = 0;
			std::uint16_t rangeShift = 0;
		} m_offsetTable;

		std::unordered_map<wchar_t, DelaunayMesh> m_characterMeshMap;
		DelaunayMesh m_missingGlyphMesh;
	};
}
#endif