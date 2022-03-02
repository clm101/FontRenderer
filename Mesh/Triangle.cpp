#include "Triangle.h"

#include <exception>

#include "Point.h"
#include "Edge.h"

#include <clmMath/clm_matrix.h>
#include <clmUtil/clm_err.h>

namespace clm {
	Triangle::Triangle(point_index_t p0,
					   point_index_t p1,
					   point_index_t p2)
		:
		m_edges({}),
		m_triangles({})
	{
		m_points[0] = p0;
		m_points[1] = p1;
		m_points[2] = p2;
	}
	Triangle::~Triangle()
	{
		for (triangle_ptr_t triangle : m_triangles)
		{
			if (triangle)
			{
				triangle->remove_triangle(this);
			}
		}

		for (edge_ptr_t edge : m_edges)
		{
			if (edge)
			{
				edge->remove_triangle(this);
			}
		}
	}
	void Triangle::set_points(point_index_t p0, point_index_t p1, point_index_t p2) noexcept
	{
		m_points[0] = p0;
		m_points[1] = p1;
		m_points[2] = p2;
	}
	void Triangle::set_edges(edge_ptr_t e0, edge_ptr_t e1, edge_ptr_t e2) noexcept
	{
		m_edges[0] = e0;
		m_edges[1] = e1;
		m_edges[2] = e2;
	}
	/*void Triangle::set_triangles(triangle_ptr_t t0, triangle_ptr_t t1, triangle_ptr_t t2) noexcept
	{
		m_triangles[0] = t0;
		m_triangles[1] = t1;
		m_triangles[2] = t2;
	}*/

	const std::array<Triangle::point_index_t, 3>& Triangle::get_points() const noexcept
	{
		return m_points;
	}
	const std::array<Triangle::edge_ptr_t, 3>& Triangle::get_edges() const noexcept
	{
		return m_edges;
	}
	/*const std::array<Triangle::triangle_ptr_t, 3>& Triangle::get_triangles() const noexcept
	{
		return m_triangles;
	}*/

	void Triangle::remove_triangle(const triangle_ptr_t triangle) noexcept
	{
		for (size_t i = 0; i < m_triangles.size(); i++)
		{
			if (m_triangles[i] == triangle)
			{
				m_triangles[i] = nullptr;
				return;
			}
		}
	}

	void Triangle::remove_edge(const edge_ptr_t edge) noexcept
	{
		for (size_t i = 0; i < m_edges.size(); i++)
		{
			if (m_edges[i] == edge)
			{
				m_edges[i] = nullptr;
				return;
			}
		}
	}
}