#include <chrono>
#include <cstdlib>
#include <iostream>

#include "longnum.hpp"

int main(int argc, char *argv[]) {
  if (argc > 2) {
    std::cout << "Computes pi with n decimal digits of precision\n"
                 "\n"
                 "Usage:\n"
              << argv[0] << " [precision]\n";
    return EXIT_FAILURE;
  }

  int dec_precision{100};
  if (argc == 2) {
    try {
      dec_precision = std::stoi(argv[1]);
    } catch (...) {
      std::cerr << "Exception raised when converting precision to an integer\n";
      return EXIT_FAILURE;
    }
  }

  if (dec_precision < 0) {
    std::cerr << "Precision must be non-negative\n";
    return EXIT_FAILURE;
  }

  // 2^10 = 1024
  // 10^3 = 1000
  auto bin_precision{(10 * dec_precision + 2) / 3};

  // Align to make shifts faster
  bin_precision = static_cast<long long>((bin_precision * 32 + 31)) / 32 + 32;

  ln::Longnum pi(0, bin_precision);
  ln::Longnum n1(1);
  ln::Longnum n2(2);
  ln::Longnum n4(4);
  ln::Longnum n8(8);

  ln::Longnum a(1, bin_precision);
  ln::Longnum b(4, bin_precision);
  ln::Longnum c(5, bin_precision);
  ln::Longnum d(6, bin_precision);
  ln::Longnum pow16(1);

  auto start{std::chrono::high_resolution_clock::now()};

  for (int i{0}; i <= dec_precision; i++) {
    pi += (n4 / a - n2 / b - n1 / c - n1 / d) / pow16;

    pow16 *= 16;
    a += n8;
    b += n8;
    c += n8;
    d += n8;
  }

  auto end{std::chrono::high_resolution_clock::now()};
  auto duration{
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start)};

  std::cout << "First " << dec_precision
            << " decimal floating point places of pi are:\n\n";
  std::cout << pi.to_string(dec_precision) << '\n';
  std::cout << "\nComputed in " << duration << '\n';

  return EXIT_SUCCESS;
}
