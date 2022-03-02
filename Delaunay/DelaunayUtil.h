#ifndef DELAUNAY_UTIL_H
#define DELAUNAY_UTIL_H

#include <MeshUtil.h>
#include <clmUtil/clm_concepts_ext.h>
#include <clmUtil/clm_util.h>

namespace clm {
	bool encloses(const point_t& newPoint, const point_t& p0, const point_t& p1, const point_t& p2) noexcept;
	bool is_ghost(const point_t& point) noexcept(util::release);
	template<util::all_same<point_t>...Ts>
	bool has_point_at_infinity(const Ts&...points) noexcept
	{
		return disjunction(std::isinf(points[0])...) || disjunction(std::isinf(points[1])...);
	}
}

#endif