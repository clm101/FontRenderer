#ifndef POINT_H
#define POINT_H

#include <vector>
#include <exception>
#include <functional>

#include <clmMath/clm_vector.h>
#include <clmUtil/clm_util.h>
#include <clmUtil/clm_err.h>

#include "MeshUtil.h"

namespace clm {
	struct point_hash {
		size_t operator()(const point_t& point) const
		{
			return hash_combine(point[0], point[1]);
		}
	};
}

template<>
struct std::hash<clm::point_t> {
	size_t operator()(const clm::point_t& key) const
	{
		return clm::point_hash()(key);
	}
};

namespace clm{
	inline float triangle_area(const point_t* p1,
							   const point_t* p2,
							   const point_t* p3)
	{
		clm::err::assert<std::runtime_error>(p1 != nullptr && p2 != nullptr && p3 != nullptr, "point_t pointer is null");
		return triangle_area(*p1,
							 *p2,
							 *p3);
	}

	inline bool is_counterclockwise(const point_t* p1,
									const point_t* p2,
									const point_t* p3) noexcept(util::release)
	{
		clm::err::assert<std::runtime_error>(p1 != nullptr && p2 != nullptr && p3 != nullptr, "point_t pointer is null");
		return is_counterclockwise(*p1,
								   *p2,
								   *p3);
	}
}

template<>
struct std::equal_to<clm::point_t> {
	bool operator()(const clm::point_t& lhs, const clm::point_t& rhs) const
	{
		return lhs == rhs;
	}
};

#endif