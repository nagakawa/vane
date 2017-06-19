#include "baseconv.h"

#include <algorithm>

int digitCharToVal(char c) {
  if (c >= '0' && c <= '9') return c - '0';
  if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
  if (c >= 'a' && c <= 'z') return c - 'a' + 10;
  return -1;
}

char digitValToChar(int x) {
  return (x < 10) ? (x + '0') : (x - 10 + 'A');
}

std::variant<BaseError, intmax_t> fromString(const char* s, int b) {
  if (b < 2 || b > 36)
    return std::variant<BaseError, intmax_t>(BaseError::outOfRange(b));
  bool isNegative = false;
  if (*s == '-') {
    ++s;
    isNegative = true;
  }
  intmax_t x = 0;
  while (*s != '\0') {
    int d = digitCharToVal(*s);
    if (d == -1 || d >= b)
      return std::variant<BaseError, intmax_t>(BaseError::invalidDigit(*s));
    if (x > INTMAX_MAX / b)
      return std::variant<BaseError, intmax_t>(BaseError::tooBig());
    x *= b;
    if (x > INTMAX_MAX - d)
      return std::variant<BaseError, intmax_t>(BaseError::tooBig());
    x += d;
    ++s;
  }
  if (isNegative) x = -x;
  return std::variant<BaseError, intmax_t>(x);
}

std::string toString(intmax_t x, int b) {
  if (x == 0) return "0";
  std::string s;
  bool isNegative = false;
  if (x < 0) {
    isNegative = true;
    x = -x;
  }
  while (x != 0) {
    intmax_t d = x % b;
    s += digitValToChar((int) d);
    x /= b;
  }
  if (isNegative) s += '-';
  std::reverse(s.begin(), s.end());
  return s;
}

std::variant<BaseError, std::string> convertBase(const char* s, int bfrom, int bto) {
  auto iResult = fromString(s, bfrom);
  if (std::holds_alternative<BaseError>(iResult))
    return std::variant<BaseError, std::string>(std::get<BaseError>(iResult));
  return std::variant<BaseError, std::string>(
    toString(std::get<intmax_t>(iResult), bto)
  );
}