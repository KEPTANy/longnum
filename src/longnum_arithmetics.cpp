#include "longnum.hpp"

namespace ln {

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
    auto bigger = abs_compare(num);
    if (bigger == std::strong_ordering::equal) {
      *this = Longnum(0);
      return *this;
    }

    if (bigger == std::strong_ordering::less) {
      negative = num.negative;
    }

    DoubleDigit carry{0};
    digits.resize(std::max(digits.size(), num.digits.size()) + 1, 0);
    for (std::size_t i{0}; i < digits.size(); i++) {
      DoubleDigit val{carry};
      if (bigger == std::strong_ordering::greater) {
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

} // namespace ln
