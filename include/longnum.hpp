#ifndef LONGNUM_HPP
#define LONGNUM_HPP

#include <cmath>
#include <compare>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ln {

// An arbitrary precision fixed-point type.
class Longnum {
public:
#if defined(__x86_64__) || defined(_WIN64)
  using Digit = std::uint32_t;
  using DoubleDigit = std::uint64_t;
#elif defined(__i386__) || defined(_WIN32)
  using Digit = std::uint16_t;
  using DoubleDigit = std::uint32_t;
#else
#error "The system is not 64-bit niether 32-bit and therefore not supported"
#endif

  static constexpr auto digit_bits{std::numeric_limits<Digit>::digits};

  using Precision = std::int32_t;

  ~Longnum() = default;
  Longnum(const Longnum &other) = default;
  Longnum &operator=(const Longnum &other) = default;
  Longnum(Longnum &&other) = default;
  Longnum &operator=(Longnum &&other) = default;

  // Initialization with 0 and precision of 0.
  Longnum();

  // Initialization with any primitive integral value and (optionally) given
  // precision.
  template <std::integral T> Longnum(T other, Precision precision = 0);

  // Initialization with any primitive floating-point value. Precision is
  // derived from the given number. `other` must be a finite number. Throws
  // if nan or inf given.
  template <std::floating_point T> Longnum(T other);

  // Converts to a string with `fp_digits` decimal places after the floating
  // point.
  std::string to_string(std::uint32_t fp_digits) const;

  // How many bits are needed to represent the absolute value of the number.
  std::size_t bits_in_absolute_value() const;

  // How many bits are used for fraction.
  Precision get_precision() const;

  // Sets how many bits are used for fraction. Keep in mind, it works in O(n).
  Longnum &set_precision(Precision prec);

  // Returns an int that:
  // 1. is 0 if a number is 0.
  // 2. is negative if a number is negative.
  // 3. is positive if a number is positive.
  int sign() const;

  // Same as multiplying the number by -1.
  Longnum &flip_sign();

  // The usual spaceship operator, nothing crazy.
  std::strong_ordering operator<=>(const Longnum &other) const;

  // Checks if the numbers are equal
  bool operator==(const Longnum &other) const;

  // Checks if the numbers are different
  bool operator!=(const Longnum &other) const;

  // Adds two numbers. Max precision of the operands is kept.
  Longnum operator+(const Longnum &other) const;

  // Adds two numbers. Max precision of the operands is kept.
  Longnum &operator+=(const Longnum &other);

  // Unary minus, just makes a copy with an opposite sign.
  Longnum operator-() const;

  // Subtracts one number from another. Max precision of the operands is kept.
  Longnum operator-(const Longnum &other) const;

  // Subtracts one number from another. Max precision of the operands is kept.
  Longnum &operator-=(const Longnum &other);

  // Multiplies two numbers. Max precision of the operands is kept.
  Longnum operator*(const Longnum &other) const;

  // Multiplies two numbers. Max precision of the operands is kept.
  Longnum &operator*=(const Longnum &other);

  // Divides one number by another. Max precision of the operands is kept.
  // Throws if `other` is 0.
  Longnum operator/(const Longnum &other) const;

  // Divides one number by another. Max precision of the operands is kept.
  // Throws if `other` is 0.
  Longnum &operator/=(const Longnum &other);

  // One number modulo another. Max precision of the operands is kept.
  // Throws if `other` is 0.
  Longnum operator%(const Longnum &other) const;

  // One number modulo another. Max precision of the operands is kept.
  // Throws if `other` is 0.
  Longnum &operator%=(const Longnum &other);

  // Returns both quotient (first) and reminder (second). Max precision of the
  // operands is kept. Throws if `other` is 0.
  std::pair<Longnum, Longnum> div_mod(const Longnum &other) const;

#ifndef LONGNUM_TEST_PRIVATE
private:
#endif
  // A number is represented with three values:
  //
  // 1. `digits` contains a sequence of limbs and represent an absolute value
  //    of a number. Works pretty much as a veeeery big uint. In order for
  //    stuff to work, for time to be saved, and memory not to be spared, there
  //    should be no leading zeros (e. g. zero is an empty vector and some
  //    functionality might want to rely on this property).
  //
  // 2. `precision` is opposite of log2 of the difference between the two
  //    closest representable numbers.
  //
  //  Therefore, the absolute value of a number is `digits` * 2^(-`precision`)
  //
  // 3. `negative`, well, shows if a number is negative or non-negative.

  std::vector<Digit> digits{};
  Precision precision{};
  bool negative{};

  // Compares absolute values of two numbers.
  std::strong_ordering abs_compare(const Longnum &other) const;

  // If numbers have different precisions, increases the smaller one to make
  // them the same.
  void align_with(Longnum &other);

  // Removes leading zeros. Needed to save memory and handle zero.
  void remove_leading_zeros();

  // Bitshift to the left. Works the same as multiplying by 2^`sh`.
  Longnum operator<<(std::size_t sh) const;

  // Bitshift to the left. Works the same as multiplying by 2^`sh`.
  Longnum &operator<<=(std::size_t sh);

  // Bitshift to the right. Works the same as dividing by 2^`sh`.
  Longnum operator>>(std::size_t sh) const;

  // Bitshift to the right. Works the same as dividing by 2^`sh`.
  Longnum &operator>>=(std::size_t sh);

  // Get `i`'th digit in radix 2^`digit_bits`.
  Digit get_digit(std::intmax_t index) const;

  // Set `i`'th digit in radix 2^`digit_bits`.
  void set_digit(std::intmax_t index, Digit digit, bool remove_zeros = false);

  // Max place in radix 2^`digit_bits`.
  std::intmax_t max_digit_index() const;

  // Min place in radix 2^`digit_bits`.
  std::intmax_t min_digit_index() const;

  // Get `i`'th digit in radix 2.
  bool get_bit(std::intmax_t index) const;

  // Set `i`'th digit in radix 2.
  void set_bit(std::intmax_t index, bool bit, bool remove_zeros = false);
};

namespace lits {

// Constructs a number using Longnum(long double).
Longnum operator""_longnum(long double other);

// Constructs a number using Longnum(unsigned long long).
Longnum operator""_longnum(unsigned long long other);

} // namespace lits

} // namespace ln

template <std::integral T>
ln::Longnum::Longnum(T other, Precision precision)
    : precision{precision}, negative{other < 0} {
  using UnsignedT = std::make_unsigned_t<T>;

  const UnsignedT abs_value{
      negative ? static_cast<UnsignedT>(-static_cast<UnsignedT>(other))
               : static_cast<UnsignedT>(other)};

  constexpr auto bits{std::numeric_limits<UnsignedT>::digits};
  for (std::intmax_t i{0}; i < bits; i++) {
    set_bit(i, (abs_value >> i) & 1);
  }

  remove_leading_zeros();
}

template <std::floating_point T>
ln::Longnum::Longnum(T other) : negative{std::signbit(other)} {
  const auto fp_type{std::fpclassify(other)};
  if (fp_type == FP_INFINITE || fp_type == FP_NAN) {
    throw std::invalid_argument("INF/NaN provided");
  }

  constexpr auto mant_bits{std::numeric_limits<T>::digits - 1};
  constexpr auto min_exp{std::numeric_limits<T>::min_exponent - 1};

  const Precision exp{std::ilogb(other)};

  const auto normalized_float{std::abs(std::scalbn(other, mant_bits - exp))};
  const auto mantissa{static_cast<std::uintmax_t>(normalized_float)};

  precision = mant_bits - (mantissa == 0 ? min_exp : exp);

  constexpr auto bits{std::numeric_limits<std::uintmax_t>::digits};
  for (std::intmax_t i{0}; i < bits; i++) {
    set_bit(i - get_precision(), (mantissa >> i) & 1);
  }

  remove_leading_zeros();
}

#endif
