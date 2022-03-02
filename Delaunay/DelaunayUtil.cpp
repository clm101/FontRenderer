#include <DelaunayUtil.h>

namespace clm {
	bool encloses(const point_t& newPoint, const point_t& p0, const point_t& p1, const point_t& p2) noexcept
	{
		if (is_ghost(p0))
		{
			return is_counterclockwise(p1, p2, newPoint);
		}
		else if (is_ghost(p1))
		{
			return is_counterclockwise(p2, p0, newPoint);
		}
		else if (is_ghost(p2))
		{
			return is_counterclockwise(p0, p1, newPoint);
		}
		else
		{
			const float p0xDiff = p0[0] - newPoint[0];
			const float p0yDiff = p0[1] - newPoint[1];
			const float p1xDiff = p1[0] - newPoint[0];
			const float p1yDiff = p1[1] - newPoint[1];
			const float p2xDiff = p2[0] - newPoint[0];
			const float p2yDiff = p2[1] - newPoint[1];
			math::Matrix<3> matrix{{ p0xDiff, p0yDiff, p0xDiff * p0xDiff + p0yDiff * p0yDiff },
									{ p1xDiff, p1yDiff, p1xDiff * p1xDiff + p1yDiff * p1yDiff },
									{ p2xDiff, p2yDiff, p2xDiff * p2xDiff + p2yDiff * p2yDiff }};
			const float determinant = matrix.determinant();
			return determinant > 0.0f;
		}
	}

	bool is_ghost(const point_t& point) noexcept(util::release)
	{
		return std::isnan(point[0]) && std::isnan(point[1]);
	}
}