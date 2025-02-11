#include "longnum.hpp"

#include <ranges>

namespace ln {

static void align_precision(Longnum &a, Longnum &b) {
  if (a.get_precision() < b.get_precision()) {
    std::swap(a, b);
  }

  b.set_precision(a.get_precision());
}

Longnum::Longnum() : digits{}, precision{0}, sign{false} {}

std::int32_t Longnum::get_precision() const { return precision; }
bool Longnum::get_sign() const { return sign; }

void Longnum::set_precision(std::int32_t new_prec) {
  if (new_prec == precision) {
    return;
  }

  if (new_prec > precision) {
    *this << (new_prec - precision);
  } else {
    *this >> (precision - new_prec);
  }

  precision = new_prec;
  remove_leading_zeros();
}

bool Longnum::is_zero() const { return digits.empty(); }

bool Longnum::operator<(const Longnum &other) const {
  Longnum a{*this}, b{other};
  ln::align_precision(a, b);

  if (a.sign != b.sign) {
    return a.sign;
  }

  if (a.digits.size() != b.digits.size()) {
    return a.sign != a.digits.size() < b.digits.size();
  }

  for (std::size_t i{a.digits.size() - 1}; i < a.digits.size(); i--) {
    if (a.digits[i] != b.digits[i]) {
      return a.sign != a.digits[i] < b.digits[i];
    }
  }

  return false;
}

bool Longnum::operator>(const Longnum &other) const {
  Longnum a{*this}, b{other};
  ln::align_precision(a, b);

  if (a.sign != b.sign) {
    return b.sign;
  }

  if (a.digits.size() != b.digits.size()) {
    return a.sign != a.digits.size() > b.digits.size();
  }

  for (std::size_t i{a.digits.size() - 1}; i < a.digits.size(); i--) {
    if (a.digits[i] != b.digits[i]) {
      return a.sign != a.digits[i] > b.digits[i];
    }
  }

  return false;
}

bool Longnum::operator<=(const Longnum &other) const {
  return !(*this > other);
}

bool Longnum::operator>=(const Longnum &other) const {
  return !(*this < other);
}

bool Longnum::operator==(const Longnum &other) const {
  Longnum a{*this}, b{other};
  ln::align_precision(a, b);

  if (a.sign != b.sign || a.digits.size() != b.digits.size()) {
    return false;
  }

  for (std::size_t i{0}; i < a.digits.size(); i++) {
    if (a.digits[i] != b.digits[i]) {
      return false;
    }
  }

  return true;
}

bool Longnum::operator!=(const Longnum &other) const {
  return !(*this == other);
}

Longnum &Longnum::operator+=(const Longnum &other) {
  Longnum num{other};
  ln::align_precision(*this, num);

  if (sign == num.sign) {
    Digit carry{0};
    digits.resize(std::max(digits.size(), num.digits.size()) + 1, 0);
    for (std::size_t i{0}; i < digits.size(); i++) {
      DoubleDigit val{static_cast<DoubleDigit>(digits[i]) + carry};
      if (i < num.digits.size()) {
        val += num.digits[i];
      }
      carry = val >> digit_bits;
      digits[i] = static_cast<Digit>(val);
    }
  } else {
    int bigger = abs_compare(num);
    if (bigger == 0) {
      *this = Longnum(0);
      return *this;
    }

    if (bigger == -1) {
      sign = num.sign;
    }

    DoubleDigit carry{0};
    digits.resize(std::max(digits.size(), num.digits.size()) + 1, 0);
    for (std::size_t i{0}; i < digits.size(); i++) {
      DoubleDigit val{carry};
      if (bigger == 1) {
        val += static_cast<DoubleDigit>(digits[i]);
        if (i < num.digits.size()) {
          val -= static_cast<DoubleDigit>(num.digits[i]);
        }
      } else {
        val -= static_cast<DoubleDigit>(digits[i]);
        if (i < num.digits.size()) {
          val += static_cast<DoubleDigit>(num.digits[i]);
        }
      }

      digits[i] = static_cast<Digit>(val);
      carry = static_cast<Digit>(val >> digit_bits);
    }
  }

  remove_leading_zeros();
  return *this;
}

Longnum &Longnum::operator-=(const Longnum &other) {
  flip_sign();
  *this += other;
  flip_sign();
  return *this;
}

Longnum &Longnum::operator*=(const Longnum &other) {
  sign = sign != other.sign;

  std::vector<Digit> new_digits(digits.size() + other.digits.size(), 0);
  for (std::size_t i{0}; i < digits.size(); i++) {
    DoubleDigit carry{0};
    for (std::size_t j{0}; j < other.digits.size(); j++) {
      DoubleDigit val{carry + static_cast<DoubleDigit>(digits[i]) *
                                  static_cast<DoubleDigit>(other.digits[j])};
      carry = val >> digit_bits;
      new_digits[i + j] += static_cast<Digit>(val);
    }
  }

  std::swap(new_digits, digits);
  precision += other.precision;

  remove_leading_zeros();
  return *this;
}

Longnum &Longnum::operator/=(const Longnum &other) {
  if (other.is_zero()) {
    throw std::invalid_argument("Division by zero is not allowed");
  }

  Longnum res{0};
  res.set_precision(precision);

  Longnum buff{0};
  buff.set_precision(precision);

  std::size_t bits{bits_in_digits() + other.bits_in_digits()};
  res.digits.reserve((bits + digit_bits - 1) / digit_bits);
  buff.digits.reserve((bits + digit_bits - 1) / digit_bits);

  for (std::size_t bit{bits - 1}; bit < bits; bit--) {
    if (bit != bits - 1) {
      buff.digits[(bit + 1) / digit_bits] ^= static_cast<Digit>(1)
                                             << ((bit + 1) % digit_bits);
    }
    buff.digits[bit / digit_bits] ^= static_cast<Digit>(1)
                                     << (bit % digit_bits);

    if (buff * other <= *this) {
      res.digits[bit / digit_bits] ^= static_cast<Digit>(1)
                                      << (bit % digit_bits);
    }
  }

  res.sign = sign != other.sign;
  res.remove_leading_zeros();
  return *this = res;
}

Longnum Longnum::operator-() const {
  Longnum x{*this};
  x.flip_sign();
  return x;
}

Longnum Longnum::operator+(const Longnum &other) const {
  Longnum x{*this};
  return x += other;
}

Longnum Longnum::operator-(const Longnum &other) const {
  Longnum x{*this};
  return x -= other;
}

Longnum Longnum::operator*(const Longnum &other) const {
  Longnum x{*this};
  return x *= other;
}

Longnum Longnum::operator/(const Longnum &other) const {
  Longnum x{*this};
  return x /= other;
}

int Longnum::abs_compare(const Longnum &other) const {
  Longnum a{*this}, b{other};
  ln::align_precision(a, b);

  if (a.digits.size() != b.digits.size()) {
    return a.digits.size() < b.digits.size() ? -1 : 1;
  }

  for (std::size_t i{a.digits.size() - 1}; i < a.digits.size(); i--) {
    if (a.digits[i] != b.digits[i]) {
      return a.digits[i] < b.digits[i] ? -1 : 1;
    }
  }

  return 0;
}

std::size_t Longnum::bits_in_digits() const {
  return digits.size() * digit_bits -
         (digits.empty() ? 0 : std::countl_zero(digits.back()));
}

void Longnum::flip_sign() {
  if (!is_zero()) {
    sign = !sign;
  }
}

void Longnum::remove_leading_zeros() {
  while (!is_zero() && digits.back() == 0) {
    digits.pop_back();
  }
  if (is_zero()) {
    sign = false;
  }
}

void Longnum::set_digits(std::uintmax_t num) {
  constexpr auto bits{std::numeric_limits<std::uintmax_t>::digits};

  const auto digits_needed{(bits + digit_bits - 1) / digit_bits};
  digits.reserve(digits_needed);

  for (std::size_t i{0}; i < digits_needed; i++) {
    const auto shift{i * digit_bits};
    constexpr auto mask{std::numeric_limits<Digit>::max()};

    digits.emplace_back((num >> shift) & mask);
  }
}

void Longnum::operator<<(std::size_t sh) {
  if (is_zero()) {
    return;
  }

  const auto full_digits{sh / digit_bits};

  digits.insert(digits.begin(), full_digits, 0);

  sh %= digit_bits;
  if (sh == 0) {
    return;
  }

  DoubleDigit carry{0};
  for (auto &curr : digits) {
    DoubleDigit shifted = (static_cast<DoubleDigit>(curr) << sh) + carry;
    carry = shifted >> (digit_bits - sh);
    curr = static_cast<Digit>(shifted);
  }

  if (carry != 0) {
    digits.push_back(static_cast<Digit>(carry));
  }

  remove_leading_zeros();
}

void Longnum::operator>>(std::size_t sh) {
  if (is_zero()) {
    return;
  }

  const auto full_digits{sh / digit_bits};
  if (full_digits >= digits.size()) {
    *this = Longnum(0);
    return;
  }

  digits.erase(digits.begin(), digits.begin() + full_digits);

  sh %= digit_bits;
  if (sh == 0) {
    return;
  }

  DoubleDigit carry{0};
  for (auto &curr : std::ranges::reverse_view(digits)) {
    DoubleDigit shifted = (static_cast<DoubleDigit>(curr) >> sh) + carry;
    carry = shifted << (digit_bits - sh);
    curr = static_cast<Digit>(shifted);
  }

  remove_leading_zeros();
}

namespace lits {

Longnum operator""_longnum(long double other) {
  return Longnum(static_cast<double>(other));
}

Longnum operator""_longnum(unsigned long long other) { return Longnum(other); }

} // namespace lits

} // namespace ln
