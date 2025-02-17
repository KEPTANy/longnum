#include "longnum.hpp"

namespace ln {

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

  Longnum aligned_other{other};
  align_with(aligned_other);

  digits.resize(std::max(digits.size(), aligned_other.digits.size()) + 1, 0);
  Digit carry{0};
  for (std::size_t i{0}; i < digits.size(); i++) {
    DoubleDigit val{carry};
    val += digits[i];
    if (i < aligned_other.digits.size()) {
      val += aligned_other.digits[i];
    }
    digits[i] = static_cast<Digit>(val);
    carry = val >> digit_bits;
  }

  remove_leading_zeros();
  return *this;
}

Longnum Longnum::operator-() const {
  Longnum x{*this};
  x.flip_sign();
  return x;
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

  Longnum aligned_other{other};
  align_with(aligned_other);

  auto cmp = abs_compare(aligned_other);
  if (cmp == 0) {
    digits.clear();
    negative = false;
    return *this;
  }

  if (cmp < 0) {
    flip_sign();
  }

  auto &a{cmp < 0 ? aligned_other.digits : digits};
  auto &b{cmp < 0 ? digits : aligned_other.digits};

  digits.resize(std::max(digits.size(), aligned_other.digits.size()), 0);
  Digit borrow{0};
  for (std::size_t i{0}; i < digits.size(); i++) {
    DoubleDigit val{a[i]};
    if (i < b.size()) {
      val -= b[i];
    }
    val -= borrow;
    digits[i] = static_cast<Digit>(val);
    borrow = (val >> digit_bits) ? 1 : 0;
  }

  remove_leading_zeros();
  return *this;
}

Longnum Longnum::operator*(const Longnum &other) const {
  if (sign() == 0 || other.sign() == 0) {
    return 0;
  }

  Longnum res{};
  res.digits.resize(digits.size() + other.digits.size(), 0);
  res.precision = get_precision() + other.get_precision();

  for (std::size_t i{0}; i < digits.size(); i++) {
    DoubleDigit carry{0};
    for (std::size_t j{0}; j < other.digits.size(); j++) {
      DoubleDigit val{carry + static_cast<DoubleDigit>(digits[i]) *
                                  static_cast<DoubleDigit>(other.digits[j])};
      val += res.digits[i + j];
      res.digits[i + j] = static_cast<Digit>(val);
      carry = val >> digit_bits;
    }

    if (carry != 0) {
      res.digits[i + other.digits.size()] = carry;
    }
  }

  res.negative = sign() != other.sign();
  res.set_precision(std::min(get_precision(), other.get_precision()));
  res.remove_leading_zeros();
  return res;
}

Longnum &Longnum::operator*=(const Longnum &other) {
  return *this = *this * other;
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
  quotient.set_precision(std::min(get_precision(), other.get_precision()));

  std::size_t bits{bits_in_absolute_value() + other.bits_in_absolute_value()};
  for (std::size_t bit{bits - 1}; bit < bits; bit--) {
    quotient.set_bit(bit + quotient.get_precision(), true);
    if (abs_compare(quotient * other) < 0) {
      quotient.set_bit(bit + quotient.get_precision(), false);
    }
  }

  quotient.negative = this_sign != other_sign;
  quotient.remove_leading_zeros();
  return {quotient, *this - quotient * other};
}

} // namespace ln
