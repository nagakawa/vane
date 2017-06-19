#pragma once

#include <stdint.h>
#include <string>
#include <variant>

int digitCharToVal(char c);
char digitValToChar(int x);

enum BaseErrorCode {
  BE_INVALID_DIGIT,
  BE_TOO_BIG,
  BE_BASE_OUT_OF_RANGE,
};

struct BaseError {
  BaseErrorCode code;
  int info;
  BaseError(BaseErrorCode code, int info) :
    code(code), info(info) {}
  static BaseError invalidDigit(int digit) {
    return BaseError(BE_INVALID_DIGIT, digit);
  }
  static BaseError tooBig() {
    return BaseError(BE_TOO_BIG, 0);
  }
  static BaseError outOfRange(int base) {
    return BaseError(BE_BASE_OUT_OF_RANGE, base);
  }
};

std::variant<BaseError, intmax_t> fromString(const char* s, int b);

std::string toString(intmax_t x, int b);