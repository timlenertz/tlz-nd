#include <catch.hpp>
#include "../src/opaque/ndarray_wraparound_opaque_view.h"
#include "../src/opaque/ndarray_timed_wraparound_opaque_view.h"
#include "../src/opaque/ndarray_timed_wraparound_opaque_view_cast.h"
#include "support/ndarray.h"

using namespace tff;
using namespace tff::test;

TEST_CASE("ndarray_timed_wraparound_opaque_view", "[nd][ndarray_timed_wraparound_opaque_view]") {
	auto shp = make_ndsize(10, 3, 4);
	auto len = shp.product();
	std::vector<std::uint32_t> raw(len);
	for(int i = 0; i < len; ++i) raw[i] = i;

	opaque_ndarray_format frm = default_opaque_ndarray_format<int>(make_ndsize(2, 3));
	ndarray_opaque_view<3, true, opaque_ndarray_format> vw(
		raw.data(),
		shp,
		ndarray_opaque_view<3, true, opaque_ndarray_format>::default_strides(shp, frm),
		frm
	);

	ndarray_wraparound_opaque_view<3, true, opaque_ndarray_format> vw_w = wraparound(
		vw,
		make_ndptrdiff(-3, 2, 1),
		make_ndptrdiff(2, 7, 3),
		make_ndptrdiff(1, 2, -1)
	);

	ndarray_timed_wraparound_opaque_view<3, true, opaque_ndarray_format> vw_w_t = timed(vw_w, 100);
	
	SECTION("basics") {
		REQUIRE(vw_w_t.start_time() == 100);
		REQUIRE(vw_w_t.end_time() == 105);
		REQUIRE(vw_w_t.duration() == 5);
		
		ndarray_wraparound_opaque_view<3, true, opaque_ndarray_format> vw_w_re = vw_w_t.non_timed();
		REQUIRE(same(vw_w, vw_w_re));
		
		REQUIRE(vw_w_t.time_to_coordinate(102) == 2);
		REQUIRE(vw_w_t.coordinate_to_time(2) == 102);
		
	}
	
	
	SECTION("same, reset") {
		ndarray_wraparound_opaque_view<3, true, opaque_ndarray_format> vw_w2 = wraparound(
			vw,
			make_ndptrdiff(-2, 2, 1),
			make_ndptrdiff(3, 7, 3),
			make_ndptrdiff(1, 2, -1)
		);

		ndarray_timed_wraparound_opaque_view<3, true, opaque_ndarray_format> vw_w_t2 = timed(vw_w, 200);
		ndarray_timed_wraparound_opaque_view<3, true, opaque_ndarray_format> vw_w_t3 = timed(vw_w2, 100);
		REQUIRE_FALSE(same(vw_w_t, vw_w_t2));
		REQUIRE_FALSE(same(vw_w_t, vw_w_t3));
		vw_w_t2.reset(vw_w_t);
		REQUIRE(same(vw_w_t, vw_w_t2));
		vw_w_t3.reset(vw_w_t);
		REQUIRE(same(vw_w_t, vw_w_t3));
	}


	SECTION("section, indexing") {
		REQUIRE(same(vw_w_t.at_time(101), vw_w[1]));
		REQUIRE(same(vw_w_t.at_time(104), vw_w[4]));
		
		REQUIRE(same(vw_w_t.tsection(101, 104), vw_w(1, 4)));
		REQUIRE(vw_w_t(2, 4)()(0, 1).start_time() == 102);
		REQUIRE(vw_w_t(3)(0)(1).start_time() == 103);
		REQUIRE(vw_w_t()(1, 3)(1).start_time() == 100);
	}
	
	
	SECTION("cast") {
		ndarray_timed_wraparound_view<5, int> vw_w_t_conc = from_opaque<5, int>(vw_w_t);
		REQUIRE(vw_w_t_conc.start() == vw_w_t.start());
		REQUIRE(vw_w_t_conc.start_time() == vw_w_t.start_time());
		ndarray_timed_wraparound_opaque_view<3, true, opaque_ndarray_format> vw_w_t_re = to_opaque<3>(vw_w_t_conc, frm);
		REQUIRE(same(vw_w_t, vw_w_t_re));
		REQUIRE(vw_w_t.start_time() == vw_w_t_re.start_time());
	}
}

	
