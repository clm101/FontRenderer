#ifndef EDGE_H
#define EDGE_H

#include <memory>
#include <array>

#include <clmUtil/clm_util.h>
#include "MeshUtil.h"

namespace clm {
	enum class EdgeCategory {
		Constrained,
		Unconstrained
	};

	class Edge {
	public:
		using point_index_t = size_t;
		using edge_ptr_t = edge_t*;
		using triangle_ptr_t = triangle_t*;

		Edge() noexcept = default;
		Edge(point_index_t, point_index_t) noexcept;
		Edge(const Edge&) noexcept = default;
		Edge(Edge&&) noexcept = default;
		~Edge() noexcept;

		void set_start(point_index_t) noexcept;
		void set_end(point_index_t) noexcept;
		void set_triangle(triangle_ptr_t) noexcept;
		void set_dual(edge_ptr_t) noexcept;

		point_index_t get_start() const noexcept;
		point_index_t get_end() const noexcept;
		triangle_ptr_t get_triangle() const noexcept;
		edge_ptr_t get_dual() const noexcept;

		void remove_triangle(triangle_ptr_t) noexcept;
		void remove_edge(edge_ptr_t) noexcept;
	private:
		point_index_t m_start;
		point_index_t m_end;

		triangle_ptr_t m_triangle;

		edge_ptr_t m_dual;
	};
}
#endif