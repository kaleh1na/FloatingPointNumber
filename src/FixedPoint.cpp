#include "FixedPoint.h"

void FixedPointArithmetic::Module(uint32_t& number) {
  if ((integer_size + fractional_size) < 32) {
    uint32_t range = 1 << (integer_size + fractional_size);
    number %= range;
  }
}

bool FixedPointArithmetic::HexToInt(const char* arg, uint32_t& number) {
  if (arg[0] != '0' || arg[1] != 'x') {
    return false;
  }
  size_t arg_size = strlen(arg);
  int digit;
  int j = 1;
  for (int i = arg_size - 1; i > 1; --i) {
    if ((arg_size - 1 - i) == 8) {
      break;
    }
    if (isdigit(arg[i])) {
      digit = arg[i] - 48;
    } else if (arg[i] > 64 && arg[i] < 71) {
      digit = arg[i] - 55;
    } else if (arg[i] > 96 && arg[i] < 103) {
      digit = arg[i] - 87;
    } else {
      return false;
    }
    number += digit * j;
    j *= 16;
  }
  Module(number);
  return true;
}

bool FixedPointArithmetic::ReadFormat(const char* arg) {
  size_t i = 0;
  while (arg[i] != '.' && arg[i] != '\0') {
    if (!isdigit(arg[i])) {
      return false;
    }
    integer_size = integer_size * 10 + (arg[i] - '0');
    ++i;
  }
  ++i;
  while (arg[i] != '\0') {
    if (!isdigit(arg[i])) {
      return false;
    }
    fractional_size = fractional_size * 10 + (arg[i] - '0');
    ++i;
  }
  return true;
}

void FixedPointArithmetic::Round(uint64_t& number, const uint32_t divider,
                                       const bool is_negative) {
  uint64_t remainder = number % divider;
  number /= divider;
  if (remainder != 0) {
    switch (rounding_type) {
      case 1:
        if ((remainder * 2 > divider) ||
            (remainder * 2 == divider && number % 2 == 1)) {
          number += 1;
        }
        break;
      case 2:
        if (!is_negative) {
          number += 1;
        }
        break;
      case 3:
        if (is_negative) {
          number += 1;
        }
        break;
    }
  }
}

uint32_t FixedPointArithmetic::Negation(const uint32_t number) {
  if (number == 0) {
    return number;
  }
  uint64_t range = uint64_t(1) << (integer_size + fractional_size);
  return range - number;
}

void FixedPointArithmetic::Multiplication(uint32_t& result) {
  bool is_negative = false;
  if ((number1 >> (integer_size + fractional_size - 1)) % 2 != 0) {
    number1 = Negation(number1);
    is_negative = !is_negative;
  }
  if ((number2 >> (integer_size + fractional_size - 1)) % 2 != 0) {
    number2 = Negation(number2);
    is_negative = !is_negative;
  }
  uint64_t pre_result = ((uint64_t)number1 * number2);
  uint32_t divider = (1 << fractional_size);
  Round(pre_result, divider, is_negative);
  if (is_negative) {
    pre_result = Negation(pre_result);
  }
  result = pre_result;
}

bool FixedPointArithmetic::Division(uint32_t& result) {
  if (number2 == 0) {
    std::cout << "division by zero";
    return false;
  }
  bool is_negative = false;
  if ((number1 >> (integer_size + fractional_size - 1)) % 2 != 0) {
    number1 = Negation(number1);
    is_negative = !is_negative;
  }
  if ((number2 >> (integer_size + fractional_size - 1)) % 2 != 0) {
    number2 = Negation(number2);
    is_negative = !is_negative;
  }
  uint64_t pre_result = ((uint64_t)number1 << fractional_size);
  Round(pre_result, number2, is_negative);
  if (is_negative) {
    pre_result = Negation(pre_result);
  }
  result = pre_result;
  return true;
}

void FixedPointArithmetic::PrintNumber(uint32_t number) {
  bool is_negative = false;
  if (number >> (integer_size + fractional_size - 1) == 1) {
    number = Negation(number);
    is_negative = true;
  }
  uint32_t integer_part = number >> fractional_size;
  uint64_t fractional_part =
      (uint64_t)(number - (number >> fractional_size << fractional_size)) * 1000;
   Round(fractional_part, (uint64_t)1 << fractional_size, is_negative);
  if (fractional_part == 1000) {
    integer_part += 1;
    fractional_part = 0;
  }
  if (fractional_part == 0 && integer_part == 0) {
    is_negative = false;
  }
  if (is_negative) {
    std::cout << '-';
  }
  std::cout << integer_part << "." << fractional_part / 100
            << (fractional_part / 10) % 10 << fractional_part % 10;
}

bool FixedPointArithmetic::Parse(const int argc, char** argv) {
  if (!(argc == 4 || argc == 6)) {
    return false;
  }
  if (!ReadFormat(argv[1])) {
    return false;
  }
  if ((fractional_size + integer_size) > 32) {
    return false;
  }
  if (!(strlen(argv[2]) == 1 && argv[2][0] > 47 && argv[2][0] < 52)) {
    return false;
  }
  rounding_type = (argv[2][0] - '0');
  if (rounding_type < 0 || rounding_type > 3) {
    return false;
  }
  if (!HexToInt(argv[3], number1)) {
    return false;
  }
  if (argc == 4) {
    return true;
  }
  if (!(strlen(argv[4]) == 1 && (argv[4][0] == '+' || argv[4][0] == '-' ||
                                 argv[4][0] == '*' || argv[4][0] == '/'))) {
    return false;
  }
  operation = argv[4][0];
  if (!HexToInt(argv[5], number2)) {
    return false;
  }
  return true;
}

void FixedPointArithmetic::DoOperation() {
  uint32_t result;
  switch (operation) {
    case '+':
      result = number1 + number2;
      break;
    case '-':
      result = number1 + Negation(number2);
      break;
    case '*':
      Multiplication(result);
      break;
    case '/':
      if (!Division(result)) {
        return;
      }
      break;
    case '=':
      result = number1;
      break;
  }
  Module(result);
  PrintNumber(result);
}