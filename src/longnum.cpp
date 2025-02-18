#include "longnum.hpp"

#include <algorithm>
#include <ranges>

namespace ln {

Longnum::Longnum() : digits{}, precision{0}, negative{false} {}

std::string Longnum::to_string(std::uint32_t fp_digits) const {
  Longnum num{*this};
  num.negative = false;
  if (fp_digits) {
    for (std::size_t i{0}; i < fp_digits; i++) {
      num *= 10;
    }
  }
  num.set_precision(0);

  std::string res;
  while (num.sign() != 0) {
    auto [q, r] = num.div_mod(10);
    num = q;
    res += static_cast<char>(r.get_digit(0) + '0');

    if (res.size() == fp_digits) {
      res += '.';
    }
  }

  while (res.size() < fp_digits) {
    res += '0';
    if (res.size() == fp_digits) {
      res += '.';
    }
  }

  if (res.back() == '.' || res.empty()) {
    res += '0';
  }

  if (sign() < 0) {
    res += '-';
  }

  std::reverse(res.begin(), res.end());
  return res;
}

std::size_t Longnum::bits_in_absolute_value() const {
  return sign() == 0
             ? 0
             : digits.size() * digit_bits - std::countl_zero(digits.back());
}

Longnum::Precision Longnum::get_precision() const { return precision; }

Longnum &Longnum::set_precision(Longnum::Precision new_prec) {
  auto old_prec{get_precision()};
  if (new_prec == old_prec) {
    return *this;
  }

  if (new_prec > old_prec) {
    *this <<= (new_prec - old_prec);
  } else {
    *this >>= (old_prec - new_prec);
  }

  precision = new_prec;
  remove_leading_zeros();
  return *this;
}

int Longnum::sign() const {
  if (digits.empty()) {
    return 0;
  }
  return negative ? -1 : 1;
}

Longnum &Longnum::flip_sign() {
  if (sign() != 0) {
    negative = !negative;
  }
  return *this;
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

  auto max_digit{this_msb / digit_bits + 1};
  auto min_digit{std::min(-this_prec, -other_prec) / digit_bits - 1};
  for (std::intmax_t i{max_digit}; i >= min_digit; i--) {
    auto x{this->get_digit(i)};
    auto y{other.get_digit(i)};
    if (x != y) {
      return x > y ? std::strong_ordering::greater : std::strong_ordering::less;
    }
  }

  return std::strong_ordering::equal;
}

bool Longnum::operator==(const Longnum &other) const {
  return (*this <=> other) == 0;
}

bool Longnum::operator!=(const Longnum &other) const {
  return !(*this == other);
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

// TODO: can be sped up by getting two consecutive digits
Longnum::Digit Longnum::get_digit(std::intmax_t index) const {
  if (get_precision() % digit_bits == 0) {
    index += get_precision() / digit_bits;
    return (index >= 0 && static_cast<std::size_t>(index) < digits.size())
               ? digits[index]
               : 0;
  }

  auto real_index{index * digit_bits};
  Digit res{0};
  for (std::intmax_t bit{0}; bit < digit_bits; bit++) {
    if (get_bit(bit + real_index)) {
      res |= static_cast<Digit>(1) << bit;
    }
  }

  return res;
}

// TODO: can be sped up by getting two consecutive digits
void Longnum::set_digit(std::intmax_t index, Digit digit, bool remove_zeros) {
  if (get_precision() % digit_bits == 0) {
    index += get_precision() / digit_bits;
    if (index >= 0 && static_cast<std::size_t>(index) < digits.size()) {
      digits[index] = digit;
    }
    return;
  }

  auto real_index{index * digit_bits};
  for (std::intmax_t bit{0}; bit < digit_bits; bit++) {
    set_bit(bit + real_index, digit & (static_cast<Digit>(1) << bit));
  }

  if (remove_zeros) {
    remove_leading_zeros();
  }
}

bool Longnum::get_bit(std::intmax_t index) const {
  const auto real_index{index + get_precision()};
  if (real_index < 0 ||
      static_cast<std::size_t>(real_index) / digit_bits >= digits.size()) {
    return false;
  }
  return (digits[real_index / digit_bits] >> (real_index % digit_bits)) & 0x1;
}

void Longnum::set_bit(std::intmax_t index, bool bit, bool remove_zeros) {
  auto real_index{index + get_precision()};

  if (real_index < 0) {
    return;
  }

  const auto digits_needed{(real_index + digit_bits - 1) / digit_bits + 1};
  digits.resize(
      std::max(digits.size(), static_cast<std::size_t>(digits_needed)), 0);

  Digit &val{digits[real_index / digit_bits]};
  if (bit) {
    val |= static_cast<Digit>(1) << (real_index % digit_bits);
  } else {
    val &= ~(static_cast<Digit>(1) << (real_index % digit_bits));
  }

  if (remove_zeros) {
    remove_leading_zeros();
  }
}

namespace lits {

Longnum operator""_longnum(long double other) { return Longnum(other); }
Longnum operator""_longnum(unsigned long long other) { return Longnum(other); }

} // namespace lits

} // namespace ln
