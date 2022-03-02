#include <iostream>
#include <format>
#include <string>

#include "Mesh.h"

namespace clm {
	Mesh::Mesh(const std::vector<point_t>& points)
	{
		add_points(points);
	}

	const std::vector<point_t>& Mesh::get_points() const noexcept
	{
		return m_points;
	}

#if USE_TRIANGLE_VECTOR
	std::vector<triangle_t> Mesh::get_triangles() const noexcept
	{
		std::vector<triangle_t> triangles{};
		triangles.reserve(m_triangles.size());
		for (const auto& triangle : m_triangles)
		{
			if (!triangle.deleted)
			{
				triangles.push_back(triangle.triangle);
			}
		}

		return triangles;
	}
#else
	const std::unordered_map<const triangle_t*, Mesh::triangle_ptr_t>& Mesh::get_triangles() const noexcept
	{
		return m_triangles;
	}
#endif

	void Mesh::add_points(const std::vector<point_t>& points)
	{
		m_points.reserve(m_points.size() + points.size());
		for (const auto& point : points)
		{
			m_points.push_back(point);
		}
	}

	void Mesh::add_point(const point_t& point)
	{
		m_points.push_back(point);
	}

	void Mesh::add_triangle(const size_t p0, const size_t p1, const size_t p2)
	{
#if USE_TRIANGLE_VECTOR
		size_t newTriangleIndex{};
		if (m_openIndices.empty())
		{
			m_triangles.emplace_back(false, triangle_t{p0, p1, p2});
			newTriangleIndex = m_triangles.size() - 1;
		}
		else
		{
			newTriangleIndex = m_openIndices.front();
			m_openIndices.pop();
			if (!m_triangles[newTriangleIndex].deleted)
			{
				throw std::runtime_error{"Attempting to overwrite active triangle"};
			}
			m_triangles[newTriangleIndex] = TriangleInfo(false, triangle_t{p0, p1, p2});
		}

		m_pointTriangleMap[{p0, p1, p2}] = newTriangleIndex;
		m_pointTriangleMap[{p1, p2, p0}] = newTriangleIndex;
		m_pointTriangleMap[{p2, p0, p1}] = newTriangleIndex;

		m_edgeTriangleMap[{p0, p1}] = newTriangleIndex;
		m_edgeTriangleMap[{p1, p2}] = newTriangleIndex;
		m_edgeTriangleMap[{p2, p0}] = newTriangleIndex;
#else
		if (m_pointTriangleMap.contains({p0, p1, p2}))
		{
			return;
		}
		else
		{
			// Can probably do this without messing with pointers, consider just using std::vector and indices
			std::unique_ptr<triangle_t> newTriangle = std::make_unique<triangle_t>(p0, p1, p2);
			triangle_t* newTriangleRaw = newTriangle.get();

			m_pointTriangleMap.insert({{p0, p1, p2}, newTriangleRaw});
			m_pointTriangleMap.insert({{p1, p2, p0}, newTriangleRaw});
			m_pointTriangleMap.insert({{p2, p0, p1}, newTriangleRaw});

			edge_t* edge01 = nullptr;
			edge_t* edge12 = nullptr;
			edge_t* edge20 = nullptr;

			if (m_pointEdgeMap.contains({p0, p1}))
			{
				edge01 = m_pointEdgeMap.find({p0, p1})->second;
			}
			else
			{
				edge_ptr_t newEdge = std::make_unique<edge_t>(p0, p1);
				edge01 = newEdge.get();
				m_edges.emplace(edge01, std::move(newEdge));
			}

			if (m_pointEdgeMap.contains({p1, p2}))
			{
				edge12 = m_pointEdgeMap.find({p1, p2})->second;
			}
			else
			{
				edge_ptr_t newEdge = std::make_unique<edge_t>(p1, p2);
				edge12 = newEdge.get();
				m_edges.emplace(edge12, std::move(newEdge));
			}

			if (m_pointEdgeMap.contains({p2, p0}))
			{
				edge20 = m_pointEdgeMap.find({p2, p0})->second;
			}
			else
			{
				edge_ptr_t newEdge = std::make_unique<edge_t>(p2, p0);
				edge20 = newEdge.get();
				m_edges.emplace(edge20, std::move(newEdge));
			}

			if (m_pointEdgeMap.contains({p1, p0}))
			{
				edge01->set_dual(m_pointEdgeMap.find({p1, p0})->second);
			}
			if (m_pointEdgeMap.contains({p2, p1}))
			{
				edge12->set_dual(m_pointEdgeMap.find({p2, p1})->second);
			}
			if (m_pointEdgeMap.contains({p0, p2}))
			{
				edge20->set_dual(m_pointEdgeMap.find({p0, p2})->second);
			}

			newTriangle->set_edges(edge01, edge12, edge20);
			edge01->set_triangle(newTriangleRaw);
			edge12->set_triangle(newTriangleRaw);
			edge20->set_triangle(newTriangleRaw);

			m_pointTriangleMap.insert({{p0, p1, p2}, newTriangleRaw});
			m_pointTriangleMap.insert({{p1, p2, p0}, newTriangleRaw});
			m_pointTriangleMap.insert({{p2, p0, p1}, newTriangleRaw});

			m_edgeTriangleMap.insert({{p0, p1}, newTriangleRaw});
			m_edgeTriangleMap.insert({{p1, p2}, newTriangleRaw});
			m_edgeTriangleMap.insert({{p2, p0}, newTriangleRaw});

			m_pointEdgeMap.insert({{p0, p1}, edge01});
			m_pointEdgeMap.insert({{p1, p2}, edge12});
			m_pointEdgeMap.insert({{p2, p0}, edge20});

			m_triangles.emplace(newTriangleRaw, std::move(newTriangle));
		}
#endif
	}

	void Mesh::delete_triangle(size_t p0, size_t p1, size_t p2)
	{
#if USE_TRIANGLE_VECTOR
		const auto indexIterator = m_pointTriangleMap.find({p0, p1, p2});
		if (indexIterator == m_pointTriangleMap.end())
		{
			return;
		}
		m_triangles[indexIterator->second].deleted = true;
		m_openIndices.push(indexIterator->second);

		m_pointTriangleMap.erase({p0, p1, p2});
		m_pointTriangleMap.erase({p1, p2, p0});
		m_pointTriangleMap.erase({p2, p0, p1});

		m_edgeTriangleMap.erase({p0, p1});
		m_edgeTriangleMap.erase({p1, p2});
		m_edgeTriangleMap.erase({p2, p0});
#else
		err::assert<std::runtime_error>(m_pointTriangleMap.find({p0, p1, p2}) != m_pointTriangleMap.end() &&
										m_pointTriangleMap.find({p1, p2, p0}) != m_pointTriangleMap.end() &&
										m_pointTriangleMap.find({p2, p0, p1}) != m_pointTriangleMap.end(),
										"Attempting to delete already-deleted or non-existent triangle");

		if (m_pointTriangleMap.contains({p0, p1, p2}))
		{
			m_triangles.erase(m_pointTriangleMap.find({p0, p1, p2})->second);
		}
		else if (m_pointTriangleMap.contains({p1, p2, p0}))
		{
			m_triangles.erase(m_pointTriangleMap.find({p1, p2, p0})->second);
		}
		else if (m_pointTriangleMap.contains({p2, p0, p1}))
		{
			m_triangles.erase(m_pointTriangleMap.find({p2, p0, p1})->second);
		}

		m_pointTriangleMap.erase({p0, p1, p2});
		m_pointTriangleMap.erase({p1, p2, p0});
		m_pointTriangleMap.erase({p2, p0, p1});

		m_edgeTriangleMap.erase({p0, p1});
		m_edgeTriangleMap.erase({p1, p2});
		m_edgeTriangleMap.erase({p2, p0});

		if (m_pointEdgeMap.contains({p0, p1}))
		{
			m_edges.erase(m_pointEdgeMap.find({p0, p1})->second);
		}
		if (m_pointEdgeMap.contains({p1, p2}))
		{
			m_edges.erase(m_pointEdgeMap.find({p1, p2})->second);
		}
		if (m_pointEdgeMap.contains({p2, p0}))
		{
			m_edges.erase(m_pointEdgeMap.find({p2, p0})->second);
		}

		m_pointEdgeMap.erase({p0, p1});
		m_pointEdgeMap.erase({p1, p2});
		m_pointEdgeMap.erase({p2, p0});
#endif
	}

}