#pragma once
#include <cstdint>
#include <cstring>
#include <iostream>

class FixedPointArithmetic {
  uint8_t integer_size = 0;
  uint8_t fractional_size = 0;
  uint8_t rounding_type = 0;
  uint8_t operation = '=';
  uint32_t number1 = 0;
  uint32_t number2 = 0;

  void Module(uint32_t& number);

  bool HexToInt(const char* arg, uint32_t& number);

  bool ReadFormat(const char* arg);

  void Round(uint64_t& number, const uint32_t divider, const bool is_negative);

  uint32_t Negation(const uint32_t number);

  void Multiplication(uint32_t& result);

  bool Division(uint32_t& result);

  void PrintNumber(uint32_t number);

 public:

  bool Parse(const int argc, char** argv);

  void DoOperation();
};
