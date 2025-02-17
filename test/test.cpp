#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#define LONGNUM_TEST_PRIVATE
#include "longnum.hpp"

using namespace ln;

TEST_CASE("Default constructor") {
    Longnum num{};

    CHECK(num.sign() == 0);
    num.flip_sign();
    CHECK(num.sign() == 0);

    CHECK(num.get_precision() == 0);
    CHECK(num.to_string(0) == "0");
    CHECK(num.to_string(5) == "0.00000");
    CHECK(num.digits.size() == 0);
}
