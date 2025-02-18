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

TEST_CASE("Addition and Subtraction") {
    SUBCASE("Basic addition") {
        Longnum a(10), b(5, 3);
        Longnum c = a + b;
        CHECK(c.to_string(0) == "15");
        CHECK(c.get_precision() == 3);

        Longnum d(-7, 1), e(3);
        CHECK((d + e).to_string(2) == "-4.00");

        Longnum A(100);
        A += Longnum(50);
        CHECK(A.to_string(1) == "150.0");
        
        Longnum B(-20);
        B += Longnum(30, 10);
        CHECK(B.to_string(0) == "10");
    }

    SUBCASE("Basic subtraction") {
        Longnum a(25), b(10);
        CHECK((a - b).to_string(0) == "15");
        CHECK((b - a).to_string(0) == "-15");

        Longnum c(-5), d(-3);
        CHECK((c - d).to_string(0) == "-2");

        Longnum A(50);
        A -= Longnum(30);
        CHECK(A.to_string(0) == "20");
        
        Longnum B(100);
        B -= Longnum(150);
        CHECK(B.to_string(0) == "-50");
    }

    SUBCASE("Zero handling") {
        Longnum num(42);
        
        CHECK(num + 0 == num);
        CHECK(num - 0 == num);
        CHECK(0_longnum + num == num);
        CHECK(0_longnum - num == -num);

        num.flip_sign();
        CHECK(num + 0 == num);
        CHECK(num - 0 == num);
        CHECK(0_longnum + num == num);
        CHECK(0_longnum - num == -num);
    }

    SUBCASE("Precision alignment") {
        Longnum a(8, 1);
        Longnum b(4, 102);
        
        Longnum sum = a + b;
        CHECK(sum.to_string(1) == "12.0");
        CHECK(sum.get_precision() == 102);

        Longnum diff = a - b * 2;
        CHECK(diff.to_string(1) == "0.0");
        CHECK(diff.get_precision() == 102);
    }

    SUBCASE("Subtraction with huge values") {
        auto pow10 = [](size_t exponent) {
            Longnum num(1);
            for(size_t i = 0; i < exponent; ++i) {
                num = num * 10_longnum;
            }
            return num;
        };

        Longnum a = pow10(100);
        a.set_precision(-1);
        Longnum b = pow10(99);
        b.set_precision(12303);
        Longnum c = a - b;
        Longnum expected = pow10(99) * 9_longnum;
        
        CHECK(c.to_string(0) == expected.to_string(0));
        CHECK(c.abs_compare(b) == std::strong_ordering::greater);
    }
}
