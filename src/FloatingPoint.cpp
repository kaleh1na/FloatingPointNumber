#include "FloatingPoint.h"

FloatingNumber::FloatingNumber(uint32_t number, const uint8_t format,
                               const uint8_t rounding_type)
    : format(format), rounding_type(rounding_type) {
  FixFormat();
  mantissa = number % (1 << mantissa_size);
  number >>= mantissa_size;
  exponent = number % (1 << exponent_size) - exponent_shift;
  number >>= exponent_size;
  is_negative = number;
  if ((exponent == min_exponent) && (mantissa == 0)) {
    is_null = true;
  }
  if (exponent == min_exponent && mantissa != 0) {
    exponent = min_exponent + 1;
    while ((mantissa << 1) < (1 << (mantissa_size + 1))) {
      --exponent;
      mantissa <<= 1;
    }
    mantissa %= (1 << mantissa_size);
  }
}

FloatingNumber::FloatingNumber(const uint8_t format,
                               const uint8_t rounding_type)
    : format(format), rounding_type(rounding_type) {
  FixFormat();
}

FloatingNumber::FloatingNumber() = default;

void FloatingNumber::FixFormat() {
  if (format == 'h') {
    mantissa_size = 10;
    exponent_size = 5;
    exponent_shift = 15;
    max_exponent = 16;
    min_exponent = -15;
  }
}

uint32_t FloatingNumber::GetMantissa() const {
  return (1 << mantissa_size) + mantissa;
}

void FloatingNumber::ChangeSign(const bool is_neg) { is_negative = is_neg; }

void FloatingNumber::MakeInfinity() {
  exponent = max_exponent;
  mantissa = 0;
}

void FloatingNumber::MakeNan() {
  exponent = max_exponent;
  mantissa = 1;
}

void FloatingNumber::MakeNull() {
  exponent = min_exponent;
  mantissa = 0;
  is_null = true;
}

void FloatingNumber::MakeMaxFinite() {
  exponent = max_exponent - 1;
  mantissa = (1 << mantissa_size) - 1;
}

void FloatingNumber::MakeMinFinite() {
  exponent = min_exponent - mantissa_size + 1;
  mantissa = 0;
}

bool FloatingNumber::IsNegative() const { return is_negative; }

bool FloatingNumber::IsInfinity() const {
  return (exponent == max_exponent) && mantissa == 0;
}

bool FloatingNumber::IsNan() const {
  return (exponent == max_exponent) && mantissa != 0;
}

bool FloatingNumber::IsNull() const {
  return is_null;
}

void FloatingNumber::FixOverflow() {
  switch (rounding_type) {
    case 0:
      MakeMaxFinite();
      return;
    case 1:
      MakeInfinity();
      return;
    case 2:
      if (is_negative) {
        MakeMaxFinite();
        return;
      }
      MakeInfinity();
      return;
    case 3:
      if (is_negative) {
        MakeInfinity();
        return;
      }
      MakeMaxFinite();
      return;
  }
}

void FloatingNumber::FixUnderflow() {
  switch (rounding_type) {
    case 0:
      MakeNull();
      return;
    case 1:
      if ((exponent == min_exponent - mantissa_size) && mantissa != 0) {
        MakeMinFinite();
        return;
      }
      MakeNull();
      return;
    case 2:
      if (is_negative) {
        MakeNull();
        return;
      }
      MakeMinFinite();
      return;
    case 3:
      if (is_negative) {
        MakeMinFinite();
        return;
      }
      MakeNull();
      return;
  }
}

void FloatingNumber::PrintNumber() const {
  if (IsNan()) {
    std::cout << "nan";
    return;
  }
  if (IsNegative()) {
    std::cout << '-';
  }
  if (IsInfinity()) {
    std::cout << "inf";
    return;
  }
  if (IsNull()) {
    if (format == 'f') {
      std::cout << "0x0.000000p+0";
    } else {
      std::cout << "0x0.000p+0";
    }
    return;
  }
  if (format == 'f') {
    printf("0x1.%06lxp", (unsigned long)mantissa << 1);
  } else {
    printf("0x1.%03lxp", (unsigned long)mantissa << 2);
  }
  if (exponent >= 0) {
    std::cout << '+';
  }
  std::cout << exponent;
}

bool FloatingPointArithmetic::HexToInt(const char *arg, uint32_t &number) {
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
  if (format == 'h') {
    number = (uint16_t)number;
  }
  return true;
}

void FloatingPointArithmetic::Round(uint64_t &number, const uint64_t divider,
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

void FloatingPointArithmetic::Normalize(FloatingNumber &result,
                                        uint64_t &mantissa, int32_t exponent,
                                        const bool is_negative,
                                        uint64_t mantissa1 = 0,
                                        uint64_t divider = 1) {
  result.is_negative = is_negative;
  int32_t point_shift = 0;
  uint32_t i = 0;
  uint64_t mantissa_copy = mantissa;
  while (mantissa_copy > 0) {
    if (mantissa_copy % 2 == 1) {
      point_shift = i;
    }
    ++i;
    mantissa_copy /= 2;
  }
  while (point_shift - mantissa_size < 0) {
    point_shift++;
    exponent--;
    mantissa <<= 1;
  }
  exponent += point_shift;
  if (exponent >= max_exponent) {
    result.FixOverflow();
    return;
  }
  if (exponent < (min_exponent - mantissa_size + 1)) {
    if (mantissa % ((uint64_t)1 << point_shift) == 0) {
      result.mantissa = 0;
    } else {
      result.mantissa = 1;
    }
    result.exponent = exponent;
    result.FixUnderflow();
    return;
  }
  uint32_t denormal_digits = 0;
  if (exponent <= min_exponent) {
    denormal_digits = min_exponent - exponent + 1;
  }
  divider *= ((uint64_t)1 << (point_shift - mantissa_size + denormal_digits));
  if (mantissa1 != 0) {
    mantissa = mantissa1;
  }
  Round(mantissa, divider, is_negative);
  if (mantissa >= ((uint64_t)1 << (mantissa_size - denormal_digits + 1))) {
    exponent += 1;
    if (exponent >= max_exponent) {
      result.FixOverflow();
      return;
    }
    mantissa >>= 1;
  }
  mantissa <<= denormal_digits;
  mantissa %= (1 << mantissa_size);
  result.mantissa = mantissa;
  result.exponent = exponent;
}

void FloatingPointArithmetic::Addition(FloatingNumber &result) {
  if (number1.IsInfinity() && number2.IsInfinity() &&
      (number1.IsNegative() ^ number2.IsNegative()) == 1) {
    result.MakeNan();
    return;
  }
  if (number1.IsInfinity()) {
    result.MakeInfinity();
    result.ChangeSign(number1.is_negative);
    return;
  }
  if (number2.IsInfinity()) {
    result.MakeInfinity();
    result.ChangeSign(number2.is_negative);
    return;
  }
  if (number1.exponent == number2.exponent &&
      number1.mantissa == number2.mantissa &&
      (number1.IsNegative() ^ number2.IsNegative()) == 1) {
    result.MakeNull();
    if (rounding_type == 3) {
      result.is_negative = true;
    }
    return;
  }
  if (number1.IsNull()) {
    result = number2;
    return;
  }
  if (number2.IsNull()) {
    result = number1;
    return;
  }
  uint64_t mantissa1 = number1.GetMantissa();
  uint64_t mantissa2 = number2.GetMantissa();
  int32_t exponent1 = number1.exponent;
  int32_t exponent2 = number2.exponent;
  bool is_negative1 = number1.is_negative;
  bool is_negative2 = number2.is_negative;
  if (exponent2 > exponent1) {
    std::swap(exponent1, exponent2);
    std::swap(mantissa1, mantissa2);
    std::swap(is_negative1, is_negative2);
  }
  uint64_t mantissa;
  int32_t exponent;
  bool is_negative = false;
  if ((exponent1 - exponent2) > (mantissa_size + 2)) {
    mantissa = mantissa1;
    exponent = exponent1 - mantissa_size;
    is_negative = is_negative1;
    if ((is_negative1 ^ is_negative2) == 0) {
      if ((!is_negative && rounding_type == 2) ||
          (is_negative && rounding_type == 3)) {
        mantissa += 1;
        Normalize(result, mantissa, exponent, is_negative);
      }
    } else {
      if ((rounding_type == 0) || (!is_negative && rounding_type == 3) ||
          (is_negative && rounding_type == 2)) {
        mantissa -= 1;
        Normalize(result, mantissa, exponent, is_negative);
        if (exponent1 - result.exponent == 1) {
          result.mantissa += 1;
        }
      }
    }
    return;
  }
  while (exponent1 != exponent2) {
    mantissa1 <<= 1;
    --exponent1;
  }
  exponent = exponent1 - mantissa_size;
  if ((is_negative1 ^ is_negative2) == 0) {
    is_negative = is_negative1;
    mantissa = mantissa1 + mantissa2;
  } else if (mantissa1 > mantissa2) {
    is_negative = is_negative1;
    mantissa = mantissa1 - mantissa2;
  } else {
    is_negative = is_negative2;
    mantissa = mantissa2 - mantissa1;
  }
  Normalize(result, mantissa, exponent, is_negative);
}

void FloatingPointArithmetic::Subtraction(FloatingNumber &result) {
  number2.is_negative = !number2.is_negative;
  Addition(result);
}

void FloatingPointArithmetic::Multiplication(FloatingNumber &result) {
  if ((number1.IsNull() && number2.IsInfinity()) ||
      (number2.IsNull() && number1.IsInfinity())) {
    result.MakeNan();
    return;
  }
  if (number1.IsInfinity() || number2.IsInfinity()) {
    result.MakeInfinity();
    result.ChangeSign(number1.IsNegative() ^ number2.IsNegative());
    return;
  }
  if (number1.IsNull() || number2.IsNull()) {
    result.MakeNull();
    result.ChangeSign(number1.IsNegative() ^ number2.IsNegative());
    return;
  }
  int32_t exponent = number1.exponent + number2.exponent - 2 * mantissa_size;
  uint64_t mantissa1 = number1.GetMantissa();
  uint64_t mantissa2 = number2.GetMantissa();
  uint64_t mantissa = mantissa1 * mantissa2;
  bool is_negative = number1.IsNegative() ^ number2.IsNegative();
  Normalize(result, mantissa, exponent, is_negative);
}

void FloatingPointArithmetic::Division(FloatingNumber &result) {
  if (number1.IsNull() && number2.IsNull()) {
    result.MakeNan();
    return;
  }
  if (number1.IsInfinity() && number2.IsInfinity()) {
    result.MakeNan();
    return;
  }
  if (number1.IsInfinity() || number2.IsNull()) {
    result.MakeInfinity();
    result.ChangeSign(number1.IsNegative() ^ number2.IsNegative());
    return;
  }
  if (number1.IsNull() || number2.IsInfinity()) {
    result.MakeNull();
    result.ChangeSign(number1.IsNegative() ^ number2.IsNegative());
    return;
  }
  int32_t exponent = number1.exponent - number2.exponent - mantissa_size - 1;
  uint64_t mantissa1 = number1.GetMantissa();
  uint64_t mantissa2 = number2.GetMantissa();
  uint64_t mantissa = (mantissa1 << (mantissa_size + 1)) / mantissa2;
  bool is_negative = number1.IsNegative() ^ number2.IsNegative();
  Normalize(result, mantissa, exponent, is_negative,
            mantissa1 << (mantissa_size + 1), mantissa2);
}

bool FloatingPointArithmetic::Parse(const int argc, char **argv) {
  if (!(argc == 4 || argc == 6)) {
    return false;
  }
  if (!(argv[1][0] == 'h' || argv[1][0] == 'f')) {
    return false;
  }
  if (argv[1][0] == 'h') {
    format = 'h';
    mantissa_size = 10;
    exponent_size = 5;
    exponent_shift = 15;
    max_exponent = 16;
    min_exponent = -15;
  }
  if (!(strlen(argv[2]) == 1 && argv[2][0] > 47 && argv[2][0] < 52)) {
    return false;
  }
  rounding_type = (argv[2][0] - '0');
  if (rounding_type < 0 || rounding_type > 3) {
    return false;
  }
  uint32_t num1 = 0;
  if (!HexToInt(argv[3], num1)) {
    return false;
  }
  number1 = FloatingNumber(num1, format, rounding_type);
  if (argc == 4) {
    return true;
  }
  if (!(strlen(argv[4]) == 1 && (argv[4][0] == '+' || argv[4][0] == '-' ||
                                 argv[4][0] == '*' || argv[4][0] == '/'))) {
    return false;
  }
  operation = argv[4][0];
  uint32_t num2 = 0;
  if (!HexToInt(argv[5], num2)) {
    return false;
  }
  number2 = FloatingNumber(num2, format, rounding_type);
  return true;
}

void FloatingPointArithmetic::DoOperation() {
  FloatingNumber result(format, rounding_type);
  if (number1.IsNan() || number2.IsNan()) {
    result.MakeNan();
  } else {
    switch (operation) {
      case '+':
        Addition(result);
        break;
      case '-':
        Subtraction(result);
        break;
      case '*':
        Multiplication(result);
        break;
      case '/':
        Division(result);
        break;
      case '=':
        result = number1;
        break;
    }
  }
  result.PrintNumber();
}