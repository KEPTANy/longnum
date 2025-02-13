#ifndef LONGNUM_HPP
#define LONGNUM_HPP

#include <climits>
#include <cmath>
#include <compare>
#include <concepts>
#include <cstdint>
#include <cstring>
#include <limits>
#include <stdexcept>
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

  using Precision = std::int32_t;

  static constexpr auto digit_bits{std::numeric_limits<Digit>::digits};

  ~Longnum() = default;
  Longnum(const Longnum &other) = default;
  Longnum &operator=(const Longnum &other) = default;
  Longnum(Longnum &&other) = default;
  Longnum &operator=(Longnum &&other) = default;

  // Initializes with 0.
  Longnum(Precision precision = 0);

  // Initialization with any primitive integral value, works as expected.
  template <std::integral T> Longnum(T other, Precision precision = 0);

  // Initialization with any IEEE 754 primitive floating-point value, the
  // precision stays the same as in `other`. `other` must be a finite number.
  template <std::floating_point T> Longnum(T other);

  std::size_t bits_in_absolute_value() const;
  Precision get_precision() const;

  void flip_sign();
  void set_precision(Precision prec);

  int sign() const;

  std::strong_ordering operator<=>(const Longnum &other) const;

  Longnum &operator+=(const Longnum &other);
  Longnum &operator-=(const Longnum &other);
  Longnum &operator*=(const Longnum &other);
  Longnum &operator/=(const Longnum &other);

  Longnum operator-() const;

  Longnum operator+(const Longnum &other) const;
  Longnum operator-(const Longnum &other) const;
  Longnum operator*(const Longnum &other) const;
  Longnum operator/(const Longnum &other) const;

private:
  std::vector<Digit> digits{};
  Precision precision{};
  bool negative{};

  std::strong_ordering abs_compare(const Longnum &other) const;
  void align_with(Longnum &other);
  void remove_leading_zeros();
  void set_digits(std::uintmax_t digits);

  void operator<<(std::size_t sh);
  void operator>>(std::size_t sh);

  bool operator[](std::intmax_t index) const;
};

namespace lits {

Longnum operator""_longnum(long double other);
Longnum operator""_longnum(unsigned long long other);

} // namespace lits

} // namespace ln

template <std::integral T>
ln::Longnum::Longnum(T other, Precision precision)
    : precision{precision}, negative{other < 0} {
  using UnsignedT = std::make_unsigned_t<T>;

  const UnsignedT abs_value{negative ? -static_cast<UnsignedT>(other)
                                     : static_cast<UnsignedT>(other)};

  set_digits(static_cast<std::uintmax_t>(abs_value));
  remove_leading_zeros();
}

template <std::floating_point T>
ln::Longnum::Longnum(T other) : negative{std::signbit(other)} {
  static_assert(sizeof(T) * CHAR_BIT <= 64,
                "Value should be at most 64-bit wide");
  static_assert(std::numeric_limits<T>::is_iec559,
                "IEEE 754 compliant type required");

  const auto fp_type{std::fpclassify(other)};
  if (fp_type == FP_INFINITE || fp_type == FP_NAN) {
    throw std::invalid_argument("INF/NaN provided");
  }

  // HACK: there's probably a better way to do this with a std::bit_cast, but
  // i just couldn't get it to work properly
  std::uintmax_t bits{0};
  std::memcpy(&bits, &other, sizeof(T));

  constexpr auto total_bits{sizeof(T) * CHAR_BIT};
  constexpr auto mant_bits{std::numeric_limits<T>::digits - 1};
  constexpr auto exp_bits{total_bits - mant_bits - 1};
  constexpr auto exp_bias{(1ull << (exp_bits - 1)) - 1};

  const auto raw_mant{bits & ((1ull << mant_bits) - 1)};
  const auto raw_exp{(bits >> mant_bits) & ((1ull << exp_bits) - 1)};

  int exp{static_cast<int>(raw_exp - exp_bias - mant_bits)};
  std::uintmax_t mant{static_cast<std::uintmax_t>(raw_mant)};
  if (raw_exp == 0) {
    exp++;
  } else {
    // add implied bit back to mantissa
    mant |= 1ull << mant_bits;
  }

  set_digits(mant);
  precision = exp;
  remove_leading_zeros();
}

#endif
