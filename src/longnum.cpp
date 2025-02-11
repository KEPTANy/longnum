#include "longnum.hpp"

#include <ranges>

namespace ln {

Longnum::Longnum() : digits{}, precision{0}, negative{false} {}

std::size_t Longnum::bits_in_absolute_value() const {
  return sign() == 0
             ? 0
             : digits.size() * digit_bits - std::countl_zero(digits.back());
}

Longnum::Precision Longnum::get_precision() const { return precision; }

void Longnum::flip_sign() {
  if (sign() != 0) {
    negative = !negative;
  }
}

void Longnum::set_precision(Longnum::Precision new_prec) {
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

int Longnum::sign() const {
  if (digits.empty()) {
    return 0;
  }
  return negative ? -1 : 1;
}

bool Longnum::operator<(const Longnum &other) const {
  Longnum a{*this}, b{other};
  a.align_with(b);

  if (a.negative != b.negative) {
    return a.negative;
  }

  if (a.digits.size() != b.digits.size()) {
    return a.negative != a.digits.size() < b.digits.size();
  }

  for (std::size_t i{a.digits.size() - 1}; i < a.digits.size(); i--) {
    if (a.digits[i] != b.digits[i]) {
      return a.negative != a.digits[i] < b.digits[i];
    }
  }

  return false;
}

bool Longnum::operator>(const Longnum &other) const { return other < *this; }

bool Longnum::operator<=(const Longnum &other) const {
  return !(*this > other);
}

bool Longnum::operator>=(const Longnum &other) const {
  return !(*this < other);
}

bool Longnum::operator==(const Longnum &other) const {
  Longnum a{*this}, b{other};
  a.align_with(b);

  if (a.negative != b.negative || a.digits.size() != b.digits.size()) {
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
  align_with(num);

  if (negative == num.negative) {
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
      negative = num.negative;
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
  negative = negative != other.negative;

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
  if (other.sign() == 0) {
    throw std::invalid_argument("Division by zero is not allowed");
  }

  Longnum res{0};
  res.set_precision(precision);

  Longnum buff{0};
  buff.set_precision(precision);

  std::size_t bits{bits_in_absolute_value() + other.bits_in_absolute_value()};
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

  res.negative = negative != other.negative;
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
  a.align_with(b);

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

void Longnum::align_with(Longnum &other) {
  const auto this_precision{get_precision()};
  const auto other_precision{other.get_precision()};

  if (this_precision < other_precision) {
    this->set_precision(other_precision);
  } else {
    other.set_precision(this_precision);
  }
}

void Longnum::remove_leading_zeros() {
  while (sign() != 0 && digits.back() == 0) {
    digits.pop_back();
  }
  if (sign() == 0) {
    negative = false;
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
  if (sign() == 0) {
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
  if (sign() == 0) {
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
