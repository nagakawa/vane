#include "ipa.h"

#include <string.h>

#include <iostream>

static const char* lowercaseHacm[] = {
  "ı", "b", "λ", "ꭥ", "e",
  "ɑ", "φ", "h", "c", "s",
  "⎫", "l", "ᴅ", "n", "ɔ",
  "d", "ɜ", "µ", "ȷ", "ſ",
  "ə", "u", "o", "╰", "ɥ",
  "z"
};

std::string latinToHacm(const std::string &l) {
  std::string s;
  size_t i = 0;
  size_t len = l.length();
  while (i < len) {
    char c = l[i];
    if (islower(c)) {
      s += lowercaseHacm[c - 'a'];
    } else {
      s += c;
    }
    ++i;
  }
  std::cerr << l << " -> " << s << '\n';
  return s;
}