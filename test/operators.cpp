#include "doctest.h"

#include "longnum.hpp"

using namespace std;
using namespace ln;
using namespace lits;

TEST_CASE("Comparison") {
    SUBCASE("Basic") {
        Longnum a(10), b(5);
        CHECK((a <=> b) > 0);
        CHECK((b <=> a) < 0);
        CHECK((a <=> a) == 0);

        CHECK(a > b);
        CHECK(b < a);
        CHECK(a != b);
        CHECK(a == a);

        b.set_precision(10);

        CHECK((a <=> b) > 0);
        CHECK((b <=> a) < 0);
        CHECK((a <=> a) == 0);

        CHECK(a > b);
        CHECK(b < a);
        CHECK(a != b);
        CHECK(a == a);

        CHECK(a.abs_compare(a) == 0);
        CHECK(a.abs_compare(b) > 0);

        a.flip_sign();
        b.flip_sign();

        CHECK((a <=> b) < 0);
        CHECK((b <=> a) > 0);
        CHECK((a <=> a) == 0);

        CHECK(a < b);
        CHECK(b > a);
        CHECK(a != b);
        CHECK(a == a);

        CHECK(a.abs_compare(a) == 0);
        CHECK(a.abs_compare(b) > 0);
    }

    SUBCASE("Different signs") {
        Longnum pos(30, 121341), neg(-30, -1);
        CHECK(pos > neg);
        CHECK(neg < pos);
        CHECK(pos.abs_compare(neg) == 0);
        CHECK(neg.abs_compare(pos) == 0);
    }

    SUBCASE("Zero comparisons") {
        Longnum pos(5, 4), neg(-3);
        
        CHECK(0 < pos);
        CHECK(pos > 0);
        CHECK(0 > neg);
        CHECK(neg < 0);
        CHECK(0_longnum == 0);
        CHECK((0_longnum <=> 0) == 0);
    }

    SUBCASE("Precision alignment") {
        Longnum c(2000000000, 1);
        Longnum d(2000000000, 2);
        CHECK((c <=> d) == 0);

        d.set_precision(124123);
        CHECK(c == d);
    }

    SUBCASE("Large numbers using literals") {
        auto big1 = 18446744073709551615.0_longnum;
        CHECK(big1.to_string(1) == "18446744073709551615.0");

        auto big2 = 18446744073709551614_longnum;

        big2 *= big1 * big1 * big1 * big1 * big1 * big1;
        big1 *= big1 * big1 * big1 * big1 * big1 * big1;
        
        CHECK(big1 > big2);
        CHECK(big2 < big1);
    }
}

