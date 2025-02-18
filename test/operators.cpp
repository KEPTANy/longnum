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

        pos += numeric_limits<Longnum::Digit>::max();
        pos += 1;

        CHECK(pos.abs_compare(neg) > 0);
        CHECK(neg.abs_compare(pos) < 0);
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

TEST_CASE("Multiplication") {
    SUBCASE("Basic multiplication") {
        Longnum a(5), b(3);
        CHECK((a * b).to_string(1) == "15.0");

        Longnum c(10), d(0);
        CHECK((c * d).to_string(1) == "0.0");
    }

    SUBCASE("Compound multiplication (*=)") {
        Longnum a(7);
        a *= Longnum(6);
        CHECK(a.to_string(1) == "42.0");

        Longnum b(10);
        b *= Longnum(0);
        CHECK(b.to_string(1) == "0.0");
    }

    SUBCASE("Negative numbers") {
        Longnum a(-4), b(5);
        CHECK((a * b).to_string(1) == "-20.0");

        Longnum c(-3), d(-2);
        CHECK((c * d).to_string(1) == "6.0");
    }

    SUBCASE("Precision handling") {
        Longnum a(3, 2);
        Longnum b(2, 3);
        Longnum result = a * b;
        
        CHECK(result.to_string(1) == "6.0");
        CHECK(result.get_precision() == 3);
    }

    SUBCASE("Large numbers") {
        using namespace lits;
        Longnum big1 = 1000000000_longnum;
        Longnum big2 = 2000000000_longnum;
        CHECK((big1 * big2).to_string(1) == "2000000000000000000.0");
    }

    SUBCASE("Identity operations") {
        Longnum a(123);
        CHECK((a * Longnum(1)).to_string(1) == "123.0");
        CHECK((a * Longnum(0)).to_string(1) == "0.0");
    }

    SUBCASE("Zero handling") {
        Longnum zero;
        Longnum num(5);
        CHECK((zero * num).to_string(1) == "0.0");
        CHECK((num * zero).to_string(1) == "0.0");
    }

    SUBCASE("Fractional numbers") {
        Longnum a(1, 1);
        Longnum b(3, 2);
        a /= 2;
        b /= 2;
        Longnum result = a * b;
        
        CHECK(result.to_string(3) == "0.750");
        CHECK(result.get_precision() == 2);
    }

    SUBCASE("Mixed signs") {
        Longnum a(-5), b(4);
        CHECK((a * b).to_string(1) == "-20.0");

        Longnum c(-3), d(-2);
        CHECK((c * d).to_string(1) == "6.0");
    }

    SUBCASE("Huge numbers") {
        using namespace lits;
        Longnum big1 = 1000000000000000000_longnum; // 1e18
        Longnum big2 = 1000000000000000000_longnum; // 1e18
        CHECK((big1 * big2).to_string(1) ==
                "1000000000000000000000000000000000000.0");
    }
}

TEST_CASE("Division and Modulo") {
    SUBCASE("Basic division") {
        Longnum a(10), b(3);
        Longnum q = a / b;
        CHECK(q.to_string(2) == "3.00");
        
        Longnum c(100), d(25);
        CHECK((c / d).to_string(1) == "4.0");
    }

    SUBCASE("Division precision") {
        Longnum a(10, 1), b(4);
        Longnum result = a / b; // 2.5
        CHECK(result.to_string(2) == "2.50");
        CHECK(result.get_precision() == 1);
    }

    SUBCASE("Modulo operations") {
        Longnum a(10), b(3);
        CHECK((a % b).to_string(1) == "1.0");
        
        Longnum c(25), d(7);
        CHECK((c % d).to_string(1) == "4.0");
    }

    SUBCASE("Div_Mod function") {
        auto [q1, r1] = Longnum(17).div_mod(Longnum(5));
        CHECK(q1.to_string(1) == "3.0");
        CHECK(r1.to_string(1) == "2.0");

        auto [q2, r2] = Longnum(100).div_mod(Longnum(25));
        CHECK(q2.to_string(1) == "4.0");
        CHECK(r2.to_string(1) == "0.0");
    }

    SUBCASE("Negative numbers") {
        SUBCASE("Negative dividend") {
            Longnum a(-10), b(3);
            CHECK((a / b).to_string(1) == "-4.0");
            CHECK((a % b).to_string(1) == "2.0");
        }

        SUBCASE("Negative divisor") {
            Longnum a(10), b(-3);
            CHECK((a / b).to_string(1) == "-3.0");
            CHECK((a % b).to_string(1) == "1.0");
        }

        SUBCASE("Both negative") {
            Longnum a(-10), b(-3);
            CHECK((a / b).to_string(1) == "4.0");
            CHECK((a % b).to_string(1) == "2.0");
        }
    }

    SUBCASE("Zero handling") {
        Longnum zero;
        Longnum num(5);
        
        SUBCASE("Division by zero") {
            CHECK_THROWS(num / zero);
            CHECK_THROWS(num % zero);
            CHECK_THROWS(zero.div_mod(zero));
        }

        SUBCASE("Zero dividend") {
            auto [q, r] = zero.div_mod(num);
            CHECK(q.to_string(1) == "0.0");
            CHECK(r.to_string(1) == "0.0");

            num.flip_sign();
            CHECK(q.to_string(1) == "0.0");
            CHECK(r.to_string(1) == "0.0");
        }
    }

    SUBCASE("Identity operations") {
        Longnum a(123, 142);
        CHECK((a / a).to_string(1) == "1.0");
        CHECK((a % a).to_string(1) == "0.0");
    }
}
