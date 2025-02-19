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
    return this_msb <=> other_msb;
  }

  auto max_digit{std::max(max_digit_index(), other.max_digit_index())};
  auto min_digit{std::min(min_digit_index(), other.min_digit_index())};
  for (std::intmax_t i{max_digit}; i >= min_digit; i--) {
    auto x{this->get_digit(i)};
    auto y{other.get_digit(i)};
    if (x != y) {
      return x <=> y;
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
    Digit shifted{static_cast<Digit>((curr << sh) | carry)};
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
    Digit shifted{static_cast<Digit>((curr >> sh) | carry)};
    carry = curr << (digit_bits - sh);
    curr = shifted;
  }

  remove_leading_zeros();
  return *this;
}

Longnum::Digit Longnum::get_digit(std::intmax_t index) const {
  if (get_precision() % digit_bits == 0) {
    index += get_precision() / digit_bits;
    return (index >= 0 && static_cast<std::size_t>(index) < digits.size())
               ? digits[index]
               : 0;
  }

  index = index * digit_bits + get_precision();

  Digit lo{0};
  if (index >= 0 &&
      static_cast<std::size_t>(index / digit_bits) < digits.size()) {
    lo = digits[index / digit_bits];
  }

  index += digit_bits;

  Digit hi{0};
  if (index >= 0 &&
      static_cast<std::size_t>(index / digit_bits) < digits.size()) {
    hi = digits[index / digit_bits];
  }

  auto shift{(get_precision()) % digit_bits};
  if (shift < 0) {
    shift += digit_bits;
  }

  return (hi << (digit_bits - shift)) | (lo >> shift);
}

void Longnum::set_digit(std::intmax_t index, Digit digit, bool remove_zeros) {
  if (get_precision() % digit_bits == 0) {
    index += get_precision() / digit_bits;
    if (index >= 0) {
      digits.resize(
          std::max(static_cast<std::size_t>(index + 1), digits.size()), 0);
      digits[index] = digit;
    }
    return;
  }

  auto shift{(get_precision()) % digit_bits};
  if (shift < 0) {
    shift += digit_bits;
  }

  Digit lo{static_cast<Digit>(digit << shift)};
  Digit hi{static_cast<Digit>(digit >> (digit_bits - shift))};

  index = index * digit_bits + get_precision();

  Digit mx{std::numeric_limits<Digit>::max()};

  if (index >= 0) {
    index /= digit_bits;
    digits.resize(std::max(static_cast<std::size_t>(index + 2), digits.size()),
                  0);

    digits[index] = (digits[index] & (mx >> (digit_bits - shift))) | lo;

    index++;

    digits[index] = (digits[index] & (mx << shift)) | hi;
  } else if ((index += digit_bits) >= 0) {
    index /= digit_bits;
    digits.resize(std::max(static_cast<std::size_t>(index + 1), digits.size()),
                  0);

    digits[index] = (digits[index] & (mx << shift)) | hi;
  }

  if (remove_zeros) {
    remove_leading_zeros();
  }
}

std::intmax_t Longnum::max_digit_index() const {
  if (sign() == 0) {
    return std::numeric_limits<std::intmax_t>::min();
  }

  std::intmax_t max_bit{static_cast<std::intmax_t>(bits_in_absolute_value()) -
                        get_precision()};

  if (max_bit >= 0 || max_bit % digit_bits == 0) {
    return max_bit / digit_bits;
  }

  return max_bit / digit_bits - 1;
}

std::intmax_t Longnum::min_digit_index() const {
  if (sign() == 0) {
    return std::numeric_limits<std::intmax_t>::max();
  }

  std::intmax_t min_bit{-get_precision()};

  if (min_bit >= 0 || min_bit % digit_bits == 0) {
    return min_bit / digit_bits;
  }

  return min_bit / digit_bits - 1;
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
