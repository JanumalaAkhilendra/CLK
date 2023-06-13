//
//  RangeDispatcher.hpp
//  Clock Signal
//
//  Created by Thomas Harte on 29/05/2023.
//  Copyright © 2023 Thomas Harte. All rights reserved.
//

#ifndef RangeDispatcher_hpp
#define RangeDispatcher_hpp

#include <algorithm>

namespace Reflection {

// Yes, macros, yuck. But I want an actual switch statement for the dispatch to start
// and to allow a simple [[fallthrough]] through all subsequent steps up until end.
// So I don't think I have much in the way of options here.
//
// Sensible choices by the optimiser are assumed.
#define index2(n)		index(n);		index(n+1);
#define index4(n)		index2(n);		index2(n+2);
#define index8(n)		index4(n);		index4(n+4);
#define index16(n)		index8(n);		index8(n+8);
#define index32(n)		index16(n);		index16(n+16);
#define index64(n)		index32(n);		index32(n+32);
#define index128(n)		index64(n);		index64(n+64);
#define index256(n)		index128(n);	index128(n+128);
#define index512(n)		index256(n);	index256(n+256);
#define index1024(n)	index512(n);	index512(n+512);
#define index2048(n)	index1024(n);	index1024(n+1024);

/// Provides glue for a run of calls like:
///
/// 	SequencerT.perform<0>(...)
/// 	SequencerT.perform<1>(...)
/// 	SequencerT.perform<2>(...)
/// 	...etc...
///
/// Allowing the caller to execute any subrange of the calls.
template <typename SequencerT>
struct RangeDispatcher {
	static_assert(SequencerT::max < 2048);

	/// Perform @c target.perform<n>() for the input range `begin <= n < end`.
	template <typename... Args>
	static void dispatch(SequencerT &target, int begin, int end, Args&&... args) {

		// Minor optimisation: do a comparison with end once outside the loop and if it implies so
		// then do no further comparisons within the loop. This is somewhat targetted at expected
		// use cases.
		if(end < SequencerT::max) {
			dispatch<true>(target, begin, end, args...);
		} else {
			dispatch<false>(target, begin, end, args...);
		}
	}

	private:
		template <bool use_end, typename... Args> static void dispatch(SequencerT &target, int begin, int end, Args&&... args) {
#define index(n)											\
	case n:													\
		if constexpr (n <= SequencerT::max) {				\
			if constexpr (n == SequencerT::max) return;		\
			if constexpr (n < SequencerT::max) {			\
				if(use_end && end == n) return;				\
			}												\
			target.template perform<n>(args...);			\
		}													\
	[[fallthrough]];

			switch(begin) {
				default: 	assert(false);
				index2048(0);
			}

#undef index
		}

};

/// An optional target for a RangeDispatcher which uses a classifier to divide the input region into typed ranges, issuing calls to the target
/// only to begin and end each subrange, and for the number of cycles spent within.
template <typename ClassifierT, typename TargetT>
struct SubrangeDispatcher {
	static_assert(ClassifierT::max < 2048);

	template <typename... Args> static void dispatch(TargetT &target, int begin, int end, Args&&... args) {
#define index(n)																\
	case n:																		\
		if constexpr (n <= ClassifierT::max) {									\
			constexpr auto region = ClassifierT::region(n);						\
			if constexpr (n == find_begin(n)) {									\
				if(n >= end) {													\
					return;														\
				}																\
				target.template begin<region>(n);								\
			}																	\
			if constexpr (n == find_end(n) - 1) {								\
				const auto clipped_begin = std::max(begin, find_begin(n));		\
				const auto clipped_end = std::min(end, find_end(n));			\
				target.template advance<region>(clipped_end - clipped_begin);	\
																				\
				if(clipped_end == n + 1) {										\
					target.template end<region>(n + 1);							\
				}																\
			}																	\
		}																		\
	[[fallthrough]];

			switch(begin) {
				default: 	assert(false);
				index2048(0);
			}

#undef index
	}

	private:
		static constexpr int find_begin(int n) {
			const auto type = ClassifierT::region(n);
			while(n && ClassifierT::region(n - 1) == type) --n;
			return n;
		}

		static constexpr int find_end(int n) {
			const auto type = ClassifierT::region(n);
			while(n < ClassifierT::max && ClassifierT::region(n) == type) ++n;
			return n;
		}
};

#undef index2
#undef index4
#undef index8
#undef index16
#undef index32
#undef index64
#undef index128
#undef index256
#undef index512
#undef index1024
#undef index2048

}

#endif /* RangeDispatcher_hpp */
