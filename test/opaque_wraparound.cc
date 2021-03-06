#include <catch.hpp>
#include "../src/opaque/ndarray_wraparound_opaque_view.h"
#include "../src/opaque/ndarray_opaque_view_cast.h"
#include "../src/opaque/ndarray_wraparound_opaque_view_cast.h"
#include "../src/opaque_format/ndarray.h"
#include "../src/opaque_format/raw.h"
#include "support/ndarray.h"

using namespace tlz;
using namespace tlz::test;

TEST_CASE("ndarray_wraparound_opaque_view", "[nd][ndarray_wraparound_opaque_view]") {
	SECTION("basics") {
		auto shp = make_ndsize(10, 3, 4);
		auto len = shp.product();
		std::vector<std::uint32_t> raw(len);
		for(int i = 0; i < len; ++i) raw[i] = i;

		opaque_raw_format frm(4);
		ndarray_opaque_view<3, true, opaque_raw_format> vw(
			raw.data(),
			shp,
			ndarray_opaque_view<3, true, opaque_raw_format>::default_strides(shp, frm),
			frm
		);

		ndarray_wraparound_opaque_view<3, true, opaque_raw_format> vw_w1 = wraparound(
			vw,
			make_ndptrdiff(-3, 2, 1),
			make_ndptrdiff(2, 7, 3),
			make_ndptrdiff(1, 2, -1)
		);
		
		REQUIRE(vw_w1.shape() == make_ndsize(5, 3, 2));
		
		REQUIRE(vw_w1[0][0][0].start() == vw[7][2][2].start());
		REQUIRE(vw_w1[1][0][0].start() == vw[8][2][2].start());
		REQUIRE(vw_w1[0][1][0].start() == vw[7][1][2].start());
		REQUIRE(vw_w1[0][0][1].start() == vw[7][2][1].start());
	}
	
	
	SECTION("cast") {
		auto concrete_shp = make_ndsize(2, 3, 3, 4);
		std::size_t concrete_len = concrete_shp.product();
		auto concrete_str = ndarray_view<4, int>::default_strides(concrete_shp, sizeof(int));
		
		std::vector<int> raw(2*concrete_len);
		
		ndarray_view<4, int> arr(raw.data(), concrete_shp, concrete_str);
		ndarray_wraparound_view<4, int> arr_w = wraparound(
			arr,
			make_ndptrdiff(-1, -2, 0, 0),
			make_ndptrdiff(3, 2, 3, 4),
			make_ndptrdiff(1, -1, 1, 1)
		);
		REQUIRE(axis_wraparound(arr_w, 0));
		REQUIRE(axis_wraparound(arr_w, 1));
		REQUIRE_FALSE(axis_wraparound(arr_w, 2));
		REQUIRE_FALSE(axis_wraparound(arr_w, 3));
		
		ndarray_wraparound_opaque_view<2, true, opaque_ndarray_format> arr_w_op = to_opaque<2>(arr_w);

		REQUIRE(reinterpret_cast<int*>(arr_w_op[1][2].start()) == arr_w[1][2].start());
		
		ndarray_wraparound_view<4, int> arr_w_re = from_opaque<4, int>(arr_w_op);
		REQUIRE(same(arr_w_re, arr_w));
	}
}
