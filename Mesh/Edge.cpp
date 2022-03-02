#include "Edge.h"
#include "Point.h"
#include "Triangle.h"

namespace clm {
	Edge::Edge(point_index_t p0, point_index_t p1) noexcept
		:
		m_start(p0),
		m_end(p1),
		m_triangle(nullptr),
		m_dual(nullptr)
	{}

	Edge::~Edge() noexcept
	{
		if (m_triangle)
		{
			m_triangle->remove_edge(this);
		}
		if (m_dual)
		{
			m_dual->remove_edge(this);
		}
	}
	void Edge::set_start(point_index_t start) noexcept
	{
		m_start = start;
	}
	void Edge::set_end(point_index_t end) noexcept
	{
		m_end = end;
	}
	void Edge::set_triangle(triangle_ptr_t triangle) noexcept
	{
		m_triangle = triangle;
	}
	void Edge::set_dual(edge_ptr_t dual) noexcept
	{
		m_dual = dual;
	}

	Edge::point_index_t Edge::get_start() const noexcept
	{
		return m_start;
	}
	Edge::point_index_t Edge::get_end() const noexcept
	{
		return m_end;
	}
	Edge::triangle_ptr_t Edge::get_triangle() const noexcept
	{
		return m_triangle;
	}
	Edge::edge_ptr_t Edge::get_dual() const noexcept
	{
		return m_dual;
	}

	void Edge::remove_triangle(const triangle_ptr_t triangle) noexcept
	{
		if (m_triangle == triangle)
		{
			m_triangle = nullptr;
		}
	}

	void Edge::remove_edge(const edge_ptr_t edge) noexcept
	{
		if (m_dual == edge)
		{
			m_dual = nullptr;
		}
	}
}