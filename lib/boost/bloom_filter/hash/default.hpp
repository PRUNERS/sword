//////////////////////////////////////////////////////////////////////////////
//
// (C) Copyright Alejandro Cabrera 2011.
// Distributed under the Boost
// Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or
// copy at http://www.boost.org/LICENSE_1_0.txt)
//
// See http://www.boost.org/libs/bloom_filter for documentation.
//
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_BLOOM_FILTER_HASH_HPP_
#define BOOST_BLOOM_FILTER_HASH_HPP_ 1
/**
 * \author Alejandro Cabrera
 * \brief A default Hasher to be used with the Bloom filter.
 */
#include <boost/functional/hash.hpp>

extern "C" uint64_t memhash(void const* key, size_t n, uint64_t seed);

namespace boost {
namespace bloom_filters {
template <typename T, size_t Seed = 0>
struct boost_hash {
	size_t operator()(const T& t) {
		size_t key = t + Seed;
		key = (~key) + (t << 21); // key = (key << 21) - key - 1;
		key = key ^ (key >> 24);
		key = (key + (key << 3)) + (key << 8); // key * 265
		key = key ^ (key >> 14);
		key = (key + (key << 2)) + (key << 4); // key * 21
		key = key ^ (key >> 28);
		key = key + (key << 31);
		return key;
	}
};
}
}
#endif
