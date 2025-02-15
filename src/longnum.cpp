#include "longnum.hpp"

#include <ranges>

namespace ln {

Longnum::Longnum(Precision precision)
    : digits{}, precision{precision}, negative{false} {}

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
  auto old_prec{get_precision()};
  if (new_prec == old_prec) {
    return;
  }

  if (new_prec > old_prec) {
    *this <<= (new_prec - old_prec);
  } else {
    *this >>= (precision - old_prec);
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

std::strong_ordering Longnum::operator<=>(const Longnum &other) const {
  auto this_sign{sign()};
  auto other_sign{other.sign()};

  if (this_sign != other_sign) {
    return this_sign <=> other_sign;
  }

  auto cmp{abs_compare(other)};
  return this_sign >= 0 ? cmp : 0 <=> cmp;
}

std::strong_ordering Longnum::abs_compare(const Longnum &other) const {
  auto this_bits{bits_in_absolute_value()};
  auto other_bits{other.bits_in_absolute_value()};

  auto this_prec{get_precision()};
  auto other_prec{other.get_precision()};

  auto this_msb{static_cast<std::intmax_t>(this_bits) - this_prec};
  auto other_msb{static_cast<std::intmax_t>(other_bits) - other_prec};
  if (this_msb != other_msb) {
    return this_msb < other_msb ? std::strong_ordering::less
                                : std::strong_ordering::greater;
  }

  for (std::intmax_t i{this_msb - 1}; i <= this_prec && i <= other_prec; i--) {
    auto x{(*this)[i]};
    auto y{other[i]};
    if (x != y) {
      return x ? std::strong_ordering::greater : std::strong_ordering::less;
    }
  }

  return std::strong_ordering::equal;
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

Longnum Longnum::operator<<(std::size_t sh) const {
  Longnum x{*this};
  return x <<= sh;
}

Longnum &Longnum::operator<<=(std::size_t sh) {
  if (sign() == 0) {
    return *this;
  }

  const auto full_digits{sh / digit_bits};

  digits.insert(digits.begin(), full_digits, 0);

  sh %= digit_bits;
  if (sh == 0) {
    return *this;
  }

  Digit carry{0};
  for (auto &curr : digits) {
    Digit shifted{(curr << sh) | carry};
    carry = curr >> (digit_bits - sh);
    curr = shifted;
  }

  if (carry != 0) {
    digits.push_back(carry);
  }

  remove_leading_zeros();
  return *this;
}

Longnum Longnum::operator>>(std::size_t sh) const {
  Longnum x{*this};
  return x >>= sh;
}

Longnum &Longnum::operator>>=(std::size_t sh) {
  if (sign() == 0) {
    return *this;
  }

  const auto full_digits{sh / digit_bits};
  if (full_digits >= digits.size()) {
    return (*this = Longnum(0));
  }

  digits.erase(digits.begin(), digits.begin() + full_digits);

  sh %= digit_bits;
  if (sh == 0) {
    return *this;
  }

  Digit carry{0};
  for (auto &curr : std::ranges::reverse_view(digits)) {
    Digit shifted{(curr >> sh) | carry};
    carry = curr << (digit_bits - sh);
    curr = shifted;
  }

  remove_leading_zeros();
  return *this;
}

bool Longnum::get_bit(std::intmax_t index) const {
  const auto real_index{index - get_precision()};
  if (real_index < 0 ||
      static_cast<std::size_t>(real_index) / digit_bits >= digits.size()) {
    return false;
  }
  return digits[real_index / digit_bits] >> (real_index % digit_bits);
}

void Longnum::set_bit(std::intmax_t index, bool bit) {
  auto real_index{index - get_precision()};

  if (real_index < 0) {
    return;
  }

  const auto digits_needed{(real_index + digit_bits - 1) / digit_bits};
  digits.reserve(digits_needed);

  Digit &val{digits[real_index / digit_bits]};
  if (bit) {
    val |= static_cast<Digit>(1) << (real_index % digit_bits);
  } else {
    val &= ~(static_cast<Digit>(1) << (real_index % digit_bits));
  }
}

namespace lits {

Longnum operator""_longnum(long double other) { return Longnum(other); }
Longnum operator""_longnum(unsigned long long other) { return Longnum(other); }

} // namespace lits

} // namespace ln
