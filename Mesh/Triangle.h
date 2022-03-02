#ifndef TRIANGLE_H
#define TRIANGLE_H

#include <memory>
#include <array>
#include <unordered_set>

#include <clmUtil/clm_util.h>
#include <clmUtil/clm_err.h>

#include "MeshUtil.h"

namespace clm {
	class Triangle {
	public:
		using point_index_t = size_t;
		using edge_ptr_t = edge_t*;
		using triangle_ptr_t = triangle_t*;

		Triangle() noexcept = default;
		Triangle(point_index_t,
				 point_index_t,
				 point_index_t);
		~Triangle() noexcept;
		Triangle(const Triangle&) noexcept = default;
		Triangle(Triangle&&) noexcept = default;
		Triangle& operator=(const Triangle&) noexcept = default;
		Triangle& operator=(Triangle&&) noexcept = default;

		void set_points(point_index_t, point_index_t, point_index_t) noexcept;
		void set_edges(edge_ptr_t, edge_ptr_t, edge_ptr_t) noexcept;
		//void set_triangles(triangle_ptr_t, triangle_ptr_t, triangle_ptr_t) noexcept;

		const std::array<point_index_t, 3>& get_points() const noexcept;
		const std::array<edge_ptr_t, 3>& get_edges() const noexcept;
		//const std::array<triangle_ptr_t, 3>& get_triangles() const noexcept;

		void remove_triangle(const triangle_ptr_t)noexcept;
		void remove_edge(const edge_ptr_t)noexcept;
	private:
		std::array<point_index_t, 3> m_points;
		std::array<edge_ptr_t, 3> m_edges;
		std::array<triangle_ptr_t, 3> m_triangles;
	};
}

#endif