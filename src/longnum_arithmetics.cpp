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
  Longnum x{*this};
  return x *= other;
}

Longnum &Longnum::operator*=(const Longnum &other) {
  if (sign() == 0 || other.sign() == 0) {
    return *this = 0;
  }

  negative = sign() != other.sign();

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

  digits = std::move(new_digits);
  precision += other.get_precision();
  set_precision(std::max(get_precision(), other.get_precision()));

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

Longnum Longnum::operator/(const Longnum &other) const {
  Longnum x{*this};
  return x /= other;
}

} // namespace ln
