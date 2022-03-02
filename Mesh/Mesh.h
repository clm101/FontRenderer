#ifndef MESH_H
#define MESH_H

#include <memory>
#include <vector>
#include <unordered_map>
#include <tuple>
#include <optional>
#include <queue>

#include "Point.h"
#include "Triangle.h"
#include "Edge.h"
#include "MeshUtil.h"

#define USE_TRIANGLE_VECTOR 1

namespace clm {
	class Mesh {
	public:
		using edge_ptr_t = std::unique_ptr<edge_t>;
		using triangle_ptr_t = std::unique_ptr<triangle_t>;

		using point_triangle_key_t = std::tuple<size_t, size_t, size_t>;
		using edge_triangle_key_t = std::tuple<size_t, size_t>;
		using point_edge_key_t = std::tuple<size_t, size_t>;

		Mesh() = default;
		Mesh(const std::vector<point_t>&);
		~Mesh() = default;
		Mesh(const Mesh&) = default;
		Mesh(Mesh&&) = default;
		Mesh& operator=(const Mesh&) = default;
		Mesh& operator=(Mesh&&) = default;

		const std::vector<point_t>& get_points() const noexcept;
		const std::unordered_set<edge_ptr_t>& get_edges() const noexcept;

#if USE_TRIANGLE_VECTOR
		std::vector<triangle_t> get_triangles() const noexcept;
#else
		const std::unordered_map<const triangle_t*, triangle_ptr_t>& get_triangles() const noexcept;
#endif

		void add_points(const std::vector<point_t>&);
		void add_point(const point_t&);
		void add_triangle(size_t, size_t, size_t);
		void delete_triangle(size_t, size_t, size_t);
	protected:
		void add_triangle_impl(const point_t*, const point_t*, const point_t*);
		void delete_triangle_impl(const point_t*, const point_t*, const point_t*);
		std::optional<const point_t*> get_adjacent_impl(const point_t*, const point_t*) const noexcept(util::release);

		std::vector<point_t> m_points;
#if USE_TRIANGLE_VECTOR
		struct TriangleInfo {
			bool deleted;
			triangle_t triangle;
		};

		std::vector<TriangleInfo> m_triangles;
		std::queue<size_t> m_openIndices;

		std::unordered_map<point_triangle_key_t, size_t, tuple_hash> m_pointTriangleMap;
		std::unordered_map<edge_triangle_key_t, size_t, tuple_hash> m_edgeTriangleMap;
#else
		std::unordered_map<const edge_t*, edge_ptr_t> m_edges;
		std::unordered_map<const triangle_t*, triangle_ptr_t> m_triangles;

		std::unordered_map<point_triangle_key_t, triangle_t*, tuple_hash> m_pointTriangleMap;
		std::unordered_map<edge_triangle_key_t, triangle_t*, tuple_hash> m_edgeTriangleMap;
		std::unordered_map<point_edge_key_t, edge_t*, tuple_hash> m_pointEdgeMap;
#endif	
	};
}

#endif