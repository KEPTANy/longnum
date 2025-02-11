#include "longnum.hpp"

namespace ln {

Longnum::Longnum() : digits{}, precision{0}, sign{false} {}

int Longnum::get_precision() const { return precision; }
bool Longnum::get_sign() const { return sign; }

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

bool Longnum::operator<(const Longnum &other) {
  if (this->sign != other.sign) {
    return this->sign;
  }

  if (this->digits.size() != other.digits.size()) {
    return this->sign != this->digits.size() < other.digits.size();
  }

  for (std::size_t i = this->digits.size() - 1; i < this->digits.size(); i--) {
    if (this->digits[i] != other.digits[i]) {
      return this->sign != this->digits[i] < other.digits[i];
    }
  }

  return false;
}

bool Longnum::operator>(const Longnum &other) {
  if (this->sign != other.sign) {
    return other.sign;
  }

  if (this->digits.size() != other.digits.size()) {
    return this->sign != this->digits.size() > other.digits.size();
  }

  for (std::size_t i = this->digits.size() - 1; i < this->digits.size(); i--) {
    if (this->digits[i] != other.digits[i]) {
      return this->sign != this->digits[i] > other.digits[i];
    }
  }

  return false;
}

bool Longnum::operator<=(const Longnum &other) { return !(*this > other); }

bool Longnum::operator>=(const Longnum &other) { return !(*this < other); }

bool Longnum::operator==(const Longnum &other) {
  if (this->sign != other.sign || this->digits.size() != other.digits.size()) {
    return false;
  }
  for (std::size_t i = 0; i < this->digits.size(); i++) {
    if (this->digits[i] != other.digits[i]) {
      return false;
    }
  }
  return true;
}

bool Longnum::operator!=(const Longnum &other) { return !(*this == other); }

Longnum Longnum::operator-(const Longnum &x) {
  auto y = x;
  if (!y.digits.empty()) {
    y.sign = !y.sign;
  }
  return y;
}

void Longnum::remove_leading_zeros() {
  while (!digits.empty() && digits.back() == 0) {
    digits.pop_back();
  }
}

namespace lits {

Longnum operator""_longnum(long double other) {
  return Longnum(static_cast<double>(other));
}

Longnum operator""_longnum(unsigned long long other) { return Longnum(other); }

} // namespace lits

} // namespace ln
