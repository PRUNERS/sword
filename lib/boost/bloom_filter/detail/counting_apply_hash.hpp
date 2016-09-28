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

#ifndef BOOST_BLOOM_FILTER_COUNTING_APPLY_HASH_HPP
#define BOOST_BLOOM_FILTER_COUNTING_APPLY_HASH_HPP

#include <boost/mpl/at.hpp>

#include <boost/bloom_filter/detail/exceptions.hpp>

#include <boost/foreach.hpp>

#include <stdio.h>

namespace boost {
namespace bloom_filters {
namespace detail {

struct decrement {
	size_t operator()(const size_t val, const size_t limit) {
		if (val == limit)
			throw bin_underflow_exception();

		return val - 1;
	}
};

struct increment {
	size_t operator()(const size_t val, const size_t limit) {
		if (val == limit)
			throw bin_overflow_exception();

		return val+1;
	}
};

struct assign {
	size_t operator()(const size_t val, const size_t limit) {
		if (val == limit)
			throw bin_overflow_exception();

		return val;
	}
};


template <size_t N, class CBF, class Op = void>
struct BloomOp {
	typedef typename boost::mpl::at_c<typename CBF::hash_function_type, 
			N>::type Hash;

public:
	BloomOp(const typename CBF::value_type& t,
			const typename CBF::bucket_type& slots,
			const size_t num_bins)
:
	hash_val(hasher(t) % num_bins),
	pos(hash_val / CBF::bins_per_slot()),
	offset_bits((hash_val % CBF::bins_per_slot()) * CBF::bits_per_bin()),
	target_bits((slots[pos] >> offset_bits) & CBF::mask())
{}

	BloomOp(const typename CBF::value_type& t,
			const typename CBF::bucket_type& slots,
			const size_t num_bins, std::vector<size_t>& hash_values)
	:
		hash_val(hasher(t) % num_bins),
		pos(hash_val / CBF::bins_per_slot()),
		offset_bits((hash_val % CBF::bins_per_slot()) * CBF::bits_per_bin()),
		target_bits((slots[pos] >> offset_bits) & CBF::mask())
	{
		hash_values[N] = hash_val;
	}

	BloomOp(const typename CBF::value_type& t,
			const typename CBF::bucket_type& slots,
			const size_t num_bins, const size_t hash_value)
	:
		hash_val(hash_value),
		pos(hash_val / CBF::bins_per_slot()),
		offset_bits((hash_val % CBF::bins_per_slot()) * CBF::bits_per_bin()),
		target_bits((slots[pos] >> offset_bits) & CBF::mask())
	{}

	void update(typename CBF::bucket_type& slots,
			const size_t limit) const {
		static Op op;

		const size_t final_bits = op(target_bits, limit);
		slots[pos] &= ~(CBF::mask() << offset_bits);
		slots[pos] |= (final_bits << offset_bits);
	}

	void update(uint64_t val, typename CBF::bucket_type& slots,
			const size_t limit) const {
		printf("Target Bits: %lu\n", target_bits);
		const size_t final_bits = (size_t) val;
		slots[pos] &= ~(CBF::mask() << offset_bits);
		slots[pos] |= (final_bits << offset_bits);
	}

	bool check() const {
		return (target_bits != 0);
	}

	size_t check_val() const {
		return (target_bits);
	}

	Hash hasher;
	const size_t hash_val;
	const size_t pos;
	const size_t offset_bits;
	const size_t target_bits;
};

// CBF : Counting Bloom Filter
template <size_t N,
class CBF>
struct counting_apply_hash
{
	static void insert(const typename CBF::value_type& t, 
			typename CBF::bucket_type& slots,
			const size_t num_bins)
	{
		BloomOp<N, CBF, increment> inserter(t, slots, num_bins);
		inserter.update(slots, (1ull << CBF::bits_per_bin()) - 1ull);

		counting_apply_hash<N-1, CBF>::insert(t, slots, num_bins);
	}

	static void insert(const typename CBF::value_type& t, uint64_t val,
			typename CBF::bucket_type& slots,
			const size_t num_bins)
	{
		BloomOp<N, CBF> inserter(t, slots, num_bins);
		inserter.update(val, slots, (1ull << CBF::bits_per_bin()) - 1ull);

		counting_apply_hash<N-1, CBF>::insert(t, val, slots, num_bins);
	}

	static void insert(const typename CBF::value_type& t, uint64_t val,
			typename CBF::bucket_type& slots,
			const size_t num_bins, std::vector<size_t>& hash_values)
	{
		BloomOp<N, CBF> inserter(t, slots, num_bins, hash_values[N]);
		inserter.update(val, slots, (1ull << CBF::bits_per_bin()) - 1ull);

		counting_apply_hash<N-1, CBF>::insert(t, val, slots, num_bins, hash_values);
	}

	static void remove(const typename CBF::value_type& t, 
			typename CBF::bucket_type& slots,
			const size_t num_bins)
	{
		BloomOp<N, CBF, decrement> remover(t, slots, num_bins);
		remover.update(slots, 0);

		counting_apply_hash<N-1, CBF>::remove(t, slots);
	}

	static bool contains(const typename CBF::value_type& t, 
			const typename CBF::bucket_type& slots,
			const size_t num_bins)
	{
		BloomOp<N, CBF> checker(t, slots, num_bins);
		return (checker.check() &&
				counting_apply_hash<N-1, CBF>::contains(t, slots, num_bins));
	}

	static bool contains(const typename CBF::value_type& t, uint64_t val,
			const typename CBF::bucket_type& slots,
			const size_t num_bins)
	{
		BloomOp<N, CBF> checker(t, slots, num_bins);

		return(counting_apply_hash<N-1, CBF>::contains(t, val, slots, num_bins) &&
				(checker.check_val()) &&
				(checker.check_val() != (size_t) val));

		// Manage map to count element with highest occurrence
		//		++map[checker.check_val()];
		//		return(counting_apply_hash<N-1, CBF>::contains(t, val, slots, num_bins, map));
	}

	static bool contains(const typename CBF::value_type& t, uint64_t val,
			const typename CBF::bucket_type& slots,
			const size_t num_bins, std::vector<size_t>& hash_values)
	{
		BloomOp<N, CBF> checker(t, slots, num_bins, hash_values);

		return(counting_apply_hash<N-1, CBF>::contains(t, val, slots, num_bins, hash_values) &&
				(checker.check_val()) &&
				(checker.check_val() != (size_t) val));

		// Manage map to count element with highest occurrence
		//		++map[checker.check_val()];
		//		return(counting_apply_hash<N-1, CBF>::contains(t, val, slots, num_bins, map, hash_values));
	}

	static bool contains(const typename CBF::value_type& t, uint64_t val,
			const typename CBF::bucket_type& slots,
			std::vector<size_t>& hash_values, const size_t num_bins)
	{
		BloomOp<N, CBF> checker(t, slots, num_bins, hash_values[N]);

		return(counting_apply_hash<N-1, CBF>::contains(t, val, slots, hash_values, num_bins) &&
				(checker.check_val()) &&
				(checker.check_val() != (size_t) val));

		// Manage map to count element with highest occurrence
		//		++map[checker.check_val()];
		//		return(counting_apply_hash<N-1, CBF>::contains(t, val, slots, num_bins, hash_values, map));
	}
};

template <class CBF>
struct counting_apply_hash<0, CBF>
{
	static void insert(const typename CBF::value_type& t, 
			typename CBF::bucket_type& slots,
			const size_t num_bins)
	{
		BloomOp<0, CBF, increment> inserter(t, slots, num_bins);
		inserter.update(slots, (1ull << CBF::bits_per_bin()) - 1ull);
	}

	static void insert(const typename CBF::value_type& t, uint64_t val,
			typename CBF::bucket_type& slots,
			const size_t num_bins)
	{
		BloomOp<0, CBF> inserter(t, slots, num_bins);
		inserter.update(val, slots, (1ull << CBF::bits_per_bin()) - 1ull);
	}

	static void insert(const typename CBF::value_type& t, uint64_t val,
			typename CBF::bucket_type& slots,
			const size_t num_bins, std::vector<size_t>& hash_values)
	{
		BloomOp<0, CBF> inserter(t, slots, num_bins, hash_values[0]);
		inserter.update(val, slots, (1ull << CBF::bits_per_bin()) - 1ull);
	}

	static void remove(const typename CBF::value_type& t, 
			typename CBF::bucket_type& slots,
			const size_t num_bins)
	{
		BloomOp<0, CBF, decrement> remover(t, slots, num_bins);
		remover.update(slots, 0);
	}

	static bool contains(const typename CBF::value_type& t, 
			const typename CBF::bucket_type& slots,
			const size_t num_bins)
	{
		BloomOp<0, CBF> checker(t, slots, num_bins);
		return (checker.check());
	}

	static bool contains(const typename CBF::value_type& t, uint64_t val,
			const typename CBF::bucket_type& slots,
			const size_t num_bins)
	{
		BloomOp<0, CBF> checker(t, slots, num_bins);

		return((checker.check_val()) &&
				(checker.check_val() != (size_t) val));

		// Get element with highest occurrence
		//		++map[checker.check_val()];
		//		size_t maxItem = 0;
		//		size_t maxValue = 0;
		//		BOOST_FOREACH(boost_map::value_type& i, map) {
		//			if(i.second > maxValue) {
		//				maxItem = i.first;
		//				maxValue = i.second;
		//			}
		//		}

		//		return(maxItem && (maxItem != (size_t) val));
	}

	static bool contains(const typename CBF::value_type& t, uint64_t val,
			const typename CBF::bucket_type& slots,
			const size_t num_bins, std::vector<size_t>& hash_values)
	{
		BloomOp<0, CBF> checker(t, slots, num_bins, hash_values);

		return((checker.check_val()) &&
				(checker.check_val() != (size_t) val));

		//		 Get element with highest occurrence
		//		++map[checker.check_val()];
		//		size_t maxItem = 0;
		//		size_t maxValue = 0;
		//		BOOST_FOREACH(boost_map::value_type& i, map) {
		//			if(i.second > maxValue) {
		//				maxItem = i.first;
		//				maxValue = i.second;
		//			}
		//		}
		//
		//		return(maxItem && (maxItem != (size_t) val));
	}

	static bool contains(const typename CBF::value_type& t, uint64_t val,
			const typename CBF::bucket_type& slots,
			std::vector<size_t>& hash_values, const size_t num_bins)
	{
		BloomOp<0, CBF> checker(t, slots, num_bins, hash_values[0]);

		return((checker.check_val()) &&
				(checker.check_val() != (size_t) val));

		// Get element with highest occurrence
		//		++map[checker.check_val()];
		//		size_t maxItem = 0;
		//		size_t maxValue = 0;
		//		BOOST_FOREACH(boost_map::value_type& i, map) {
		//			if(i.second > maxValue) {
		//				maxItem = i.first;
		//				maxValue = i.second;
		//			}
		//		}
		//
		//		return(maxItem && (maxItem != (size_t) val));
	}
};

} // namespace detail
} // namespace bloom_filter
} // namespace boost
#endif
