#include "longnum.hpp"

longnum::Longnum::Longnum() : digits{}, exponent{0}, is_negative{false} {}

void longnum::Longnum::remove_leading_zeros() {
  while (!digits.empty() && digits.back() == 0) {
    digits.pop_back();
  }
}
