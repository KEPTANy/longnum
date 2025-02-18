#include <iostream>
#include <cstddef>

#include "longnum.hpp"

int main(int argc, char *argv[]) {
  if (argc > 2) {
    std::cout << 
      "Computes pi with n decimal digits of precision\n"
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

// TODO: compute pi

  return EXIT_SUCCESS;
}
