#include <sstream>

#include "src/FixedPoint.h"
#include "src/FloatingPoint.h"

int main(int argc, char** argv) {
  if (argc < 2) {
    std::cerr << "Invalid Argument";
    return -1;
  }
  if (strlen(argv[1]) == 1) {
    FloatingPointArithmetic opt;
    if (!opt.Parse(argc, argv)) {
      std::cerr << "Invalid Argument";
      return -1;
    }
    opt.DoOperation();
    return 0;
  }
  FixedPointArithmetic opt;
  if (!opt.Parse(argc, argv)) {
    std::cerr << "Invalid Argument";
    return -1;
  }
  opt.DoOperation();
  return 0;
}
