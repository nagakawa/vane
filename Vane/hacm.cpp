#include "ipa.h"

#include <string.h>

#include <iostream>

static const char* lowercaseHacm[] = {
  "ı", "b", "λ", "ꭥ", "e",
  "ɑ", "φ", "h", "c", "s",
  "⎫", "l", "ᴅ", "n", "ɔ",
  "d", "ɜ", "µ", "ȷ", "ſ",
  "ə", "u", "o", "Ɩ", "ɥ",
  "z"
};

static const char* lowercaseHacmSuper[] = {
  "ˈ", "ᵇ", "?", "?", "ᵉ",
  "ᵅ", "ᵠ", "ʰ", "ᶜ", "ˢ",
  "?", "ˡ", "ᴰ", "ⁿ", "ᵓ",
  "ᵈ", "ᶟ", "?", "ʲ", "ᶴ",
  "ᵊ", "ᵘ", "ᵒ", "ᶥ", "ᶣ",
  "ᶻ"
};

static const char* hacmDigits[] = {
  "0", "1", "⅃", "ʔ", "Ƣ",
  "4", "ʕ", "9", "Δ", "L"
};

std::string latinToHacm(const std::string &l) {
  std::string s;
  size_t i = 0;
  size_t len = l.length();
  while (i < len) {
    char c = l[i];
    if (c == '\\') {
      s += l[++i];
    } else if (c == '^') {
      char d = l[++i];
      if (islower(d)) {
        s += lowercaseHacmSuper[d - 'a'];
      } else {
        s += '^';
        s += d;
      }
    } else if (islower(c)) {
      s += lowercaseHacm[c - 'a'];
    } else if (isdigit(c)) {
      s += hacmDigits[c - '0'];
    } else {
      s += c;
    }
    ++i;
  }
  std::cerr << l << " -> " << s << '\n';
  return s;
}