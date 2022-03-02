#include "Delaunay.h"

#include <iostream>
#include <limits>
#include <stack>
#include <iterator>
#include <format>
#include <cmath>

#include <clmMath/clm_vector.h>
#include <clmMath/clm_matrix.h>
#include <clmMath/clm_geo.h>

namespace clm {
	DelaunayMesh::DelaunayMesh(const std::vector<point_t>& pointsIn)
		:
		DelaunayMesh()
	{
		m_points.push_back({std::numeric_limits<float>::quiet_NaN(),
						   std::numeric_limits<float>::quiet_NaN()});
		add_points(pointsIn);

		std::sort(m_points.begin() + 1, m_points.end(), [](const point_t& lhs, const point_t& rhs)
				  {
					  if (lhs[0] < rhs[0])
					  {
						  return true;
					  }
					  else if (lhs[0] > rhs[0])
					  {
						  return false;
					  }
					  else
					  {
						  if (lhs[1] < rhs[1])
						  {
							  return true;
						  }
						  else
						  {
							  return false;
						  }
					  }
				  });

		triangulate_points();
	}

	std::tuple<size_t, size_t, size_t> DelaunayMesh::get_enclosing_triangle(size_t p0) const noexcept(util::release)
	{
		for (const auto& triangle : m_triangles)
		{
			const std::array<size_t, 3>& points = triangle.triangle.get_points();
			if (encloses(m_points[p0],
						 m_points[points[0]],
						 m_points[points[1]],
						 m_points[points[2]]))
			{
				return {points[0], points[1], points[2]};
			}
		}

		throw std::runtime_error{"No enclosing triangle"};
	}

	std::optional<size_t> DelaunayMesh::get_adjacent(size_t p0,
											 size_t p1) const noexcept(util::release)
	{
		const auto triangleIter = m_edgeTriangleMap.find({p0, p1});
		if (triangleIter == m_edgeTriangleMap.end())
		{
			return {};
		}
		else
		{
			size_t triangleIndex = triangleIter->second;
			const std::array<size_t, 3>& points = m_triangles[triangleIndex].triangle.get_points();
			if (p0 == points[0] && p1 == points[1])
			{
				return {points[2]};
			}
			else if (p0 == points[1] && p1 == points[2])
			{
				return {points[0]};
			}
			else
			{
				if constexpr (util::release)
				{
					return {points[1]};
				}
				else
				{
					if (p0 == points[2] && p1 == points[0])
					{
						return {points[1]};
					}
					else
					{
						throw std::runtime_error{"Triangle not found."};
					}
				}
			}
		}
	}

	void DelaunayMesh::triangulate_points() noexcept(util::release)
	{
		{
			size_t p0 = 0, p1 = 1, p2 = 2, p3 = 3;
			if (!is_counterclockwise(m_points[1],
									 m_points[2],
									 m_points[3]))
			{
				size_t tmp = p1;
				p1 = p2;
				p2 = tmp;
			}
			add_triangle(p1, p2, p3);
			add_triangle(p0, p2, p1);
			add_triangle(p0, p3, p2);
			add_triangle(p0, p1, p3);
		}
		using vertices_t = std::tuple<size_t, size_t, size_t>;
		std::stack<vertices_t> stack{};
		for (size_t i = 4; i < m_points.size(); i += 1)
		{
			{
				// Prevent dangling reference below
				const vertices_t vertices = get_enclosing_triangle(i);
				const size_t v0 = std::get<0>(vertices);
				const size_t v1 = std::get<1>(vertices);
				const size_t v2 = std::get<2>(vertices);
				delete_triangle(v0, v1, v2);
				stack.push({i, v0, v1});
				stack.push({i, v1, v2});
				stack.push({i, v2, v0});
			}
			while (!stack.empty())
			{
				const vertices_t vertices = stack.top();
				const size_t v0 = std::get<0>(vertices);
				const size_t v1 = std::get<1>(vertices);
				const size_t v2 = std::get<2>(vertices);
				stack.pop();

				if (std::optional<size_t> v3 = get_adjacent(v2, v1))
				{
					size_t adjacent = *v3;
					if (encloses(m_points[i],
								 m_points[v2],
								 m_points[v1],
								 m_points[adjacent]))
					{
						delete_triangle(v2, v1, adjacent);
						stack.push({i, v1, adjacent});
						stack.push({i, adjacent, v2});
					}
					else
					{
						if (is_counterclockwise(m_points[v0], 
												m_points[v1],
												m_points[v2]))
						{
							add_triangle(v0, v1, v2);
						}
						else
						{
							add_triangle(v0, v2, v1);
						}
					}
				}
			}
		}
	}
}