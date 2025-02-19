#include "longnum.hpp"

namespace ln {

static std::vector<Longnum::Digit>
mul_naive(const std::vector<Longnum::Digit> &a,
          const std::vector<Longnum::Digit> &b) {
  std::vector<Longnum::Digit> res(a.size() + b.size(), 0);

  for (std::size_t i{0}; i < a.size(); i++) {
    Longnum::DoubleDigit carry{0};
    if (a[i] == 0) {
      continue;
    }
    for (std::size_t j{0}; j < b.size(); j++) {
      Longnum::DoubleDigit val{carry +
                               static_cast<Longnum::DoubleDigit>(a[i]) *
                                   static_cast<Longnum::DoubleDigit>(b[j])};
      val += res[i + j];
      res[i + j] = static_cast<Longnum::Digit>(val);
      carry = val >> Longnum::digit_bits;
    }

    if (carry != 0) {
      res[i + b.size()] = carry;
    }
  }

  return res;
}

Longnum Longnum::operator+(const Longnum &other) const {
  Longnum x{*this};
  return x += other;
}

Longnum &Longnum::operator+=(const Longnum &other) {
  if (other.sign() == 0) {
    return *this;
  }

  if (sign() == 0) {
    return *this = other;
  }

  if (sign() != other.sign()) {
    flip_sign();
    *this -= other;
    flip_sign();
    return *this;
  }

  set_precision(std::max(get_precision(), other.get_precision()));

  Digit carry{0};

  auto start{min_digit_index()};
  auto end{std::max(max_digit_index(), other.max_digit_index())};
  for (std::intmax_t i{start}; i <= end; i++) {
    DoubleDigit val{carry};
    val += get_digit(i);
    val += other.get_digit(i);

    set_digit(i, static_cast<Digit>(val));
    carry = val >> digit_bits;
  }

  remove_leading_zeros();
  return *this;
}

Longnum Longnum::operator-() const {
  Longnum x{*this};
  return x.flip_sign();
}

Longnum Longnum::operator-(const Longnum &other) const {
  Longnum x{*this};
  return x -= other;
}

Longnum &Longnum::operator-=(const Longnum &other) {
  if (other.sign() == 0) {
    return *this;
  }

  if (sign() == 0) {
    return *this = -other;
  }

  if (sign() != other.sign()) {
    flip_sign();
    *this += other;
    flip_sign();
    return *this;
  }

  set_precision(std::max(get_precision(), other.get_precision()));

  auto cmp = abs_compare(other);
  if (cmp == 0) {
    digits.clear();
    negative = false;
    return *this;
  }

  if (cmp < 0) {
    flip_sign();
  }

  Digit borrow{0};

  auto start{min_digit_index()};
  auto end{std::max(max_digit_index(), other.max_digit_index())};
  for (std::intmax_t i{start}; i <= end; i++) {
    DoubleDigit val{cmp > 0 ? get_digit(i) : other.get_digit(i)};
    val -= cmp > 0 ? other.get_digit(i) : get_digit(i);
    val -= borrow;

    set_digit(i, static_cast<Digit>(val));
    borrow = (val >> digit_bits) ? 1 : 0;
  }

  remove_leading_zeros();
  return *this;
}

Longnum Longnum::operator*(const Longnum &other) const {
  Longnum x{*this};
  return x *= other;
}

Longnum &Longnum::operator*=(const Longnum &other) {
  if (sign() == 0 || other.sign() == 0) {
    digits.clear();
    negative = false;
    return *this;
  }

  auto new_prec{std::max(get_precision(), other.get_precision())};

  negative = sign() != other.sign();
  precision += other.precision;
  digits = mul_naive(digits, other.digits);

  set_precision(new_prec);
  remove_leading_zeros();
  return *this;
}

Longnum Longnum::operator/(const Longnum &other) const {
  return div_mod(other).first;
}

Longnum &Longnum::operator/=(const Longnum &other) {
  return *this = div_mod(other).first;
}

Longnum Longnum::operator%(const Longnum &other) const {
  return div_mod(other).second;
}

Longnum &Longnum::operator%=(const Longnum &other) {
  return *this = div_mod(other).second;
}

std::pair<Longnum, Longnum> Longnum::div_mod(const Longnum &other) const {
  auto this_sign{sign()};
  auto other_sign{other.sign()};

  if (other_sign == 0) {
    throw std::invalid_argument("Division by zero is not allowed");
  }

  if (this_sign == 0) {
    return {0, 0};
  }

  Longnum quotient{};
  quotient.set_precision(std::max(get_precision(), other.get_precision()));

  std::size_t bits{bits_in_absolute_value() + other.bits_in_absolute_value()};
  for (std::size_t bit{bits - 1}; bit < bits; bit--) {
    quotient.set_bit(bit - quotient.get_precision(), true);
    if (abs_compare(quotient * other) < 0) {
      quotient.set_bit(bit - quotient.get_precision(), false);
    }
  }

  quotient.negative = this_sign != other_sign;
  quotient.remove_leading_zeros();
  auto rem{*this - quotient * other};
  if (rem.sign() < 0) {
    if (other.sign() > 0) {
      rem += other;
      quotient -= 1;
    } else {
      rem -= other;
      quotient += 1;
    }
  }
  return {quotient, rem};
}

} // namespace ln
