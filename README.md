# Fixed-point arbitrary precision library

A simple fixed-point arbitrary precision number manipulation library

# How to build/test/delete this library

```bash
# Build static library.
# Creates liblongnum.a in the same directory as this file.
make

# Build library and run test suite. Creates test/test_runner binary.
make test

# Build and run pi computation example.
make pi

# Rebuild library. Same as make fclean && make.
make re

# Delete temporary files (object files and other junk).
make clean

# Deletes build files, liblongnum.a and test_runner.
make fclean
```

