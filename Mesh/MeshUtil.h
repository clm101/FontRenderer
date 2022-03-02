#ifndef MESH_UTIL_H
#define MESH_UTIL_H

#include <memory>
#include <tuple>
#include <type_traits>
#include <clmMath/clm_vector.h>
#include <clmMath/clm_matrix.h>

namespace clm {
	class Edge;
	class Triangle;

	using point_t = math::Point2f;
	using edge_t = Edge;
	using triangle_t = Triangle;

	inline float cross_product(const point_t& p1,
							   const point_t& p2,
							   const point_t& p3)
	{
		using vector_t = point_t;
		const vector_t vec12 = {p2[0] - p1[0], p2[1] - p1[1]};
		const vector_t vec13 = {p3[0] - p1[0], p3[1] - p1[1]};

		const float crossProduct = vec12[0] * vec13[1] - vec12[1] * vec13[0];
		return crossProduct;
	}
	inline float triangle_area(const point_t& p1,
							   const point_t& p2,
							   const point_t& p3)
	{
		return std::abs(cross_product(p1, p2, p3)) / 2.0f;
	}

	inline bool is_counterclockwise(const point_t& p1,
									const point_t& p2,
									const point_t& p3)
	{
		const float crossProduct = cross_product(p1, p2, p3);
		return (crossProduct >= 0.0f) || std::isnan(crossProduct);
	}


	// Code from https://stackoverflow.com/questions/35985960/c-why-is-boosthash-combine-the-best-way-to-combine-hash-values
	// and http://burtleburtle.net/bob/hash/evahash.html

	template<typename T>
	void hash_combine_single(size_t& seed, const T& value) noexcept
	{
		std::hash<T> hasher;
		seed ^= (hasher(value) + 0x9E3779B97F4A7C13LL + (seed << 6) + (seed >> 2));
	}
	template<typename...Ts>
	size_t hash_combine(const Ts&...values) {
		size_t result = 0;
		(hash_combine_single(result, values), ...);
		return result;
	};

	template<typename tuple_t, size_t...Ns>
	size_t hash_combine_tuple(const tuple_t& tuple, std::index_sequence<Ns...>)
	{
		size_t result = 0;
		((hash_combine_single(result, std::get<Ns>(tuple))),...);
		return result;
	}

	template<typename tuple_t>
	size_t hash_combine(const tuple_t& tuple)
	{
		return hash_combine_tuple(tuple, std::make_index_sequence<std::tuple_size_v<tuple_t>>());
	}
	
	class tuple_hash{
	public:
		template<typename tuple_t>
		size_t operator()(const tuple_t& tuple) const
		{
			return hash_combine(tuple);
		}
	};


}

#endif