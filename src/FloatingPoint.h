#pragma once
#include <stdio.h>

#include <cstdint>
#include <cstring>
#include <iostream>

struct FloatingNumber {
  bool is_negative = false;
  bool is_null = false;
  uint32_t mantissa;
  int32_t exponent;

  int32_t mantissa_size = 23;
  int32_t exponent_size = 8;
  int32_t exponent_shift = 127;
  int32_t max_exponent = 128;
  int32_t min_exponent = -127;

  uint8_t rounding_type = 0;
  uint8_t format = 'f';

  FloatingNumber(uint32_t number, const uint8_t format,
                 const uint8_t rounding_type);

  FloatingNumber(const uint8_t format, const uint8_t rounding_type);

  FloatingNumber();

  void FixFormat();

  uint32_t GetMantissa() const;

  void ChangeSign(const bool is_neg);

  void MakeInfinity();

  void MakeNan();

  void MakeNull();

  void MakeMaxFinite();

  void MakeMinFinite();

  bool IsNegative() const;

  bool IsInfinity() const;

  bool IsNan() const;

  bool IsNull() const;

  void FixOverflow();

  void FixUnderflow();

  void PrintNumber() const;
};

class FloatingPointArithmetic {
  uint8_t rounding_type = 0;
  uint8_t operation = '=';
  FloatingNumber number1;
  FloatingNumber number2;
  uint8_t format = 'f';

  int32_t mantissa_size = 23;
  int32_t exponent_size = 8;
  int32_t exponent_shift = 127;
  int32_t max_exponent = 128;
  int32_t min_exponent = -127;

  bool HexToInt(const char *arg, uint32_t &number);

  void Round(uint64_t &number, const uint64_t divider, const bool is_negative);

  void Normalize(FloatingNumber &result, uint64_t &mantissa, int32_t exponent,
                 const bool is_negative, uint64_t mantissa1,
                 uint64_t divider);

  void Addition(FloatingNumber &result);

  void Subtraction(FloatingNumber &result);

  void Multiplication(FloatingNumber &result);

  void Division(FloatingNumber &result);

 public:
  bool Parse(const int argc, char **argv);

  void DoOperation();
};