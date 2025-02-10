#include "longnum.hpp"

longnum::Longnum longnum::literals::operator""_longnum(long double other) {
  return longnum::Longnum(static_cast<double>(other));
}

longnum::Longnum
longnum::literals::operator""_longnum(unsigned long long other) {
  return Longnum(other);
}

longnum::Longnum::Longnum() : digits{}, precision{0}, sign{false} {}

void longnum::Longnum::remove_leading_zeros() {
  while (!digits.empty() && digits.back() == 0) {
    digits.pop_back();
  }
}
