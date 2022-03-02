#ifndef DELAUNAY_H
#define DELAUNAY_H
#include <array>
#include <memory>
#include <vector>
#include <exception>
#include <stdexcept>
#include <unordered_set>
#include <optional>

#include <clmMath/clm_vector.h>
#include <clmUtil/clm_util.h>
#include <clmUtil/clm_err.h>

#include <Mesh.h>
#include <DelaunayUtil.h>

namespace clm {
	class DelaunayMesh : public Mesh
	{
	public:
		DelaunayMesh() noexcept = default;
		DelaunayMesh(const std::vector<point_t>& points);
		DelaunayMesh(const DelaunayMesh&) = default;
		DelaunayMesh(DelaunayMesh&& mesh) noexcept = default;
		~DelaunayMesh() = default;
		DelaunayMesh& operator=(const DelaunayMesh&) = default;
		DelaunayMesh& operator=(DelaunayMesh&&) noexcept = default;
	private:
		std::tuple<size_t, size_t, size_t> get_enclosing_triangle(size_t) const noexcept(util::release);
		std::optional<size_t> get_adjacent(size_t, size_t) const noexcept(util::release);
		void triangulate_points() noexcept(util::release);
		void constrain_triangulation(const std::vector<std::vector<point_t>>& loops) noexcept (util::release);

		//std::unordered_set<std::shared_ptr<Triangle>> m_invalidTriangles;
		//std::unordered_set<std::shared_ptr<Triangle>> m_visitedTriangles;
		//std::unordered_set<std::weak_ptr<Edge>> m_hole;
		//std::unordered_set<std::shared_ptr<Triangle>> m_newTriangles;
		//bool m_isConstrained;
	};
}
#endif