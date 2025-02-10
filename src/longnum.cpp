#include "longnum.hpp"

namespace ln {

Longnum::Longnum() : digits{}, precision{0}, sign{false} {}

int Longnum::get_precision() const { return precision; }
bool Longnum::get_sign() const { return sign; }

void ln::Longnum::set_digits(std::uintmax_t num) {
  constexpr auto bits{std::numeric_limits<std::uintmax_t>::digits};

  const auto digits_needed{(bits + digit_bits - 1) / digit_bits};
  digits.reserve(digits_needed);

  for (std::size_t i{0}; i < digits_needed; i++) {
    const auto shift{i * digit_bits};
    constexpr auto mask{std::numeric_limits<Digit>::max()};

    digits.emplace_back((num >> shift) & mask);
  }
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
