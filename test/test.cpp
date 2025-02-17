#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#define LONGNUM_TEST_PRIVATE
#include "longnum.hpp"

using namespace std;
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

TEST_CASE("Integral constructor") {
    SUBCASE("Zero") {
        Longnum num(0);

        CHECK(num.sign() == 0);
        num.flip_sign();
        CHECK(num.sign() == 0);

        CHECK(num.get_precision() == 0);
        CHECK(num.to_string(0) == "0");
        CHECK(num.to_string(5) == "0.00000");
        CHECK(num.digits.size() == 0);
    }

    SUBCASE("Positive numbers") {
        Longnum num1(5);
        CHECK(num1.sign() > 0);
        CHECK(num1.get_precision() == 0);
        CHECK(num1.to_string(0) == "5");

        Longnum num2(12345, -10);
        CHECK(num2.sign() > 0);
        CHECK(num2.get_precision() == -10);
        CHECK(num2.to_string(3) == "12345.000");

        Longnum num3(12345, 10);
        CHECK(num3.sign() > 0);
        CHECK(num3.get_precision() == 10);
        CHECK(num3.to_string(3) == "12288.000");
    }

    SUBCASE("Negative numbers") {
        Longnum num1(-3);
        CHECK(num1.sign() < 0);
        CHECK(num1.get_precision() == 0);
        CHECK(num1.to_string(0) == "-3");

        Longnum num2(-987, -34);
        CHECK(num2.sign() == -1);
        CHECK(num2.get_precision() == -34);
        CHECK(num2.to_string(10) == "-987.0000000000");

        Longnum num3(-12345, 10);
        CHECK(num3.sign() < 0);
        CHECK(num3.get_precision() == 10);
        CHECK(num3.to_string(3) == "-12288.000");
    }

    SUBCASE("Different integer types") {
        Longnum num1((char)-10);
        Longnum num2(LLONG_MIN);
        Longnum num3((unsigned char)255);
        Longnum num4(ULLONG_MAX);
        
        CHECK(num1.to_string(0) == "-10");
        CHECK(num2.to_string(1) == "-9223372036854775808.0");
        CHECK(num3.to_string(0) == "255");
        CHECK(num4.to_string(3) == "18446744073709551615.000");
    }

    #define SUBCASE_FOR_TYPE(T) \
    SUBCASE("Limits of T") { \
        auto mn = numeric_limits<T>::min(); \
        auto mx = numeric_limits<T>::max(); \
        Longnum num_min(mn); \
        Longnum num_max(mx); \
        CHECK(num_min.to_string(0) == to_string(mn)); \
        CHECK(num_max.to_string(0) == to_string(mx)); \
    }

    SUBCASE_FOR_TYPE(signed char)
    SUBCASE_FOR_TYPE(unsigned char)
    SUBCASE_FOR_TYPE(short)
    SUBCASE_FOR_TYPE(short int)
    SUBCASE_FOR_TYPE(signed short)
    SUBCASE_FOR_TYPE(signed short int)
    SUBCASE_FOR_TYPE(unsigned short)
    SUBCASE_FOR_TYPE(unsigned short int)
    SUBCASE_FOR_TYPE(signed)
    SUBCASE_FOR_TYPE(signed int)
    SUBCASE_FOR_TYPE(unsigned)
    SUBCASE_FOR_TYPE(unsigned int)
    SUBCASE_FOR_TYPE(long)
    SUBCASE_FOR_TYPE(long int)
    SUBCASE_FOR_TYPE(signed long)
    SUBCASE_FOR_TYPE(signed long int)
    SUBCASE_FOR_TYPE(unsigned long)
    SUBCASE_FOR_TYPE(unsigned long int)
    SUBCASE_FOR_TYPE(long long)
    SUBCASE_FOR_TYPE(long long int)
    SUBCASE_FOR_TYPE(signed long long)
    SUBCASE_FOR_TYPE(signed long long int)
    SUBCASE_FOR_TYPE(unsigned long long)
    SUBCASE_FOR_TYPE(unsigned long long int)
}

TEST_CASE("Floating-point constructor") {
    SUBCASE("Special values") {
        CHECK_THROWS(Longnum(numeric_limits<float>::infinity()));
        CHECK_THROWS(Longnum(numeric_limits<double>::infinity()));
        CHECK_THROWS(Longnum(numeric_limits<long double>::infinity()));

        CHECK_THROWS(Longnum(numeric_limits<float>::quiet_NaN()));
        CHECK_THROWS(Longnum(numeric_limits<double>::quiet_NaN()));
        CHECK_THROWS(Longnum(numeric_limits<long double>::quiet_NaN()));

        CHECK(Longnum(0.0).get_precision() ==
                Longnum(numeric_limits<double>::min()).get_precision());

        CHECK(Longnum(numeric_limits<float>::min()).to_string(43) ==
                "0.0000000000000000000000000000000000000117549");
    }

    SUBCASE("Different floating types") {
        Longnum num1(-2.5f);
        Longnum num2(3.25);
        Longnum num3(10.125L);
        
        CHECK(num1.to_string(3) == "-2.500");
        CHECK(num2.to_string(3) == "3.250");
        CHECK(num3.to_string(3) == "10.125");
    }
}

TEST_CASE("Literals") {
    using namespace lits;

    SUBCASE("Integer literals") {
        auto num1 = 123_longnum;
        CHECK(num1.to_string(0) == "123");
        CHECK(num1.get_precision() == 0);

        auto num2 = 18446744073709551615_longnum;
        CHECK(num2.sign() > 0);
        CHECK(num2.to_string(0) == "18446744073709551615");

        auto num3 = -18446744073709551615_longnum;
        CHECK(num3.sign() < 0);
        CHECK(num3.to_string(0) == "-18446744073709551615");
    }

    SUBCASE("Floating literals") {
        auto num1 = 12.5_longnum;
        CHECK(num1.to_string(1) == "12.5");
        CHECK(num1.get_precision() < 0);

        auto num2 = 3.141592653589793238_longnum;
        CHECK(num2.to_string(18) == "3.141592653589793238");

        auto num3 = -0.0_longnum;
        CHECK(num3.to_string(1) == "0.0");
        CHECK(num3.get_precision() < 0);
    }
}
