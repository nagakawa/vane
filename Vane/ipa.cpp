#include "ipa.h"

#include <string.h>

#include <iostream>

const char* backslashLowercase[] = {
  "", "", "", "", "", "", "", u8"ɦ",
  "", u8"ʝ", "", "ɺ", "", "", "", u8"ɸ",
  "", u8"ɹ", u8"ɕ", "", u8"ʋ", "", u8"ɧ",
  "", u8"ʑ"
};

const char* retroLowercase[] = {
  "", "", "", u8"ɖ", "", "", "", "",
  "", "", "", u8"ɭ", "", u8"ɳ", "", "",
  "", u8"ɽ", u8"ʂ", u8"ʈ", "", "", "", "",
  "", u8"ʐ"
};

const char* implosiveLowercase[] = {
  "", u8"ɓ", "", u8"ɗ", "", "", u8"ɠ"
};

const char* capital[] = {
  u8"ɑ", u8"β", u8"ç", u8"ð", u8"ɛ",
  u8"ɱ", u8"ɣ", u8"ɥ", u8"ɪ", u8"ɲ",
  u8"ɬ", u8"ʎ", u8"ɯ", u8"ŋ", u8"ɔ",
  u8"ʋ", u8"ɒ", u8"ʁ", u8"ʃ", u8"θ",
  u8"ʊ", u8"ʌ", u8"ʍ", u8"χ", u8"ʏ",
  u8"ʒ"
};

const char* backslashCapital[] = {
  "", u8"ʙ", "", "", "", "", u8"ɢ", u8"ʜ",
  u8"ᵻ", u8"ɟ", u8"ɮ", u8"ʟ", u8"ɰ", u8"ɴ",
  u8"ʘ", "", "", u8"ʀ", "", "", u8"ᵿ",
  "", "", u8"ħ", "", ""
};

const char* symbols[] = {
  // start with space
  u8" ",
  u8"ꜜ", u8"ˈ", u8"#", u8"$", u8"ˌ", u8"ɶ",
  u8"ʲ", u8"(", u8")", u8"*", u8"+", u8",",
  u8"", u8".", u8"/", u8"0", u8"ɨ", u8"ø",
  u8"ɜ", u8"ɾ", u8"ɫ", u8"ɐ", u8"ɤ", u8"ɵ",
  u8"œ", u8"ː", u8";", u8"<", u8"̩", u8">",
  u8"ʔ", u8"ə"
};

const char* udia[] = {
  // start with "
  u8"̈", "", "", "", "", "", "", "", "",
  u8"̟", "", u8"", "", u8"̌", u8"̥", "",
  "", "", "", "", "", "", "", "",
  "", "", "", u8"̩", u8"ʼ", "", "",
  // A
  u8" ̘", u8"̏", "", "", "", u8"̂", u8"ˠ",
  u8"́", "", "", "", u8"̀", u8" ̄", u8"̼",
  u8"̹", "", "", u8"̌", "", u8"̋", "",
  "", "", u8"̆", "", "", // Z
  "", "", "", "", "", "",
  // a
  u8"̺", "", u8"̜", u8"̪", u8"̴", "", "",
  u8"ʰ", "", u8"ʲ", u8"̰", u8"ˡ", u8"̻",
  u8"ⁿ", u8"̞", "", u8"̙", u8"̝", "",
  u8"̤", "", u8"̬", u8"ʷ", u8"̽", "", "", // z
  "", "", u8"̚", u8"̃"
};

inline char index(const std::string& xs, size_t i) {
  return (i < xs.length()) ? xs[i] : '\0';
}

// TODO: add underscore-diacritics
std::string xsampaToIPA(const std::string& xs) {
  size_t i = 0;
  std::string result = "";
  while (i < xs.size()) {
    char c = xs[i];
    bool backslash = false;
    bool retro = false;
    size_t j = i + 1;
    if (index(xs, j) == '\\') {
      backslash = true;
      ++j;
    }
    if (index(xs, j) == '`') {
      retro = true;
      ++j;
    }
    if (c >= 'A' && c <= 'Z') {
      if (retro) {
        --j;
      }
      if (!backslash) {
        result += capital[c - 'A'];
      } else {
        const char* sym = backslashCapital[c - 'A'];
        if (sym[0] == '\0') {
          return std::string("Error: backslash can't modify ") + c + ". So far I have /" + result + "/.";
        }
        result += sym;
      }
    } else if (c >= 'a' && c <= 'z') {
      if (retro) {
        if (backslash) {
          if (c == 'r') result += u8"ɻ";
          else {
            --j;
            retro = false;
          }
        } else {
          const char* sym = retroLowercase[c - 'a'];
          if (sym[0] == '\0') {
            --j;
            retro = false;
          }
          else result += sym;
        }
      }
      if (retro) {
        i = j;
        continue;
      }
      if (backslash) {
        const char* sym = backslashLowercase[c - 'a'];
        if (sym[0] == '\0') {
          return std::string("Error: backslash can't modify ") + c + ". So far I have /" + result + "/.";
        }
        result += sym;
      } else if (index(xs, j) == '_' && index(xs, j + 1) == '<') {
        // implosive
        if (c >= 'h' || implosiveLowercase[c - 'a'][0] == '\0') {
          return std::string("Error: _< can't modify ") + c + ". So far I have /" + result + "/.";
        }
        result += implosiveLowercase[c - 'a'];
        j += 2;
      } else result += c;
    } else if (c == '_') {
      if (j < xs.length()) {
        bool shouldContinue = true;
        const char* t = xs.c_str() + j;
        if (strncmp(t, "B_L", 3) == 0)
          result += u8"᷅";
        else if (strncmp(t, "H_T", 3) == 0)
          result += u8"᷄";
        else if (strncmp(t, "R_F", 3) == 0)
          result += u8"᷈";
        else shouldContinue = false;
        if (shouldContinue) {
          i = j + 3;
          continue;
        }
      }
      char d = index(xs, j);
      if (d == '\0') {
        return "Error: Lone underscore at end of string. So far I have /" + result + "/.";
      }
      const char* sym = udia[d - '"'];
      if (sym[0] != '\0') {
        result += sym;
        ++j;
      } else {
        return std::string("Error: unrecognised diacritic _") + d + ". So far I have /" + result + "/.";
      }
    } else if (c < 'A') {
      if (j < xs.length()) {
        bool shouldContinue = true;
        const char* t = xs.c_str() + j - 1;
        if (strncmp(t, "<F>", 3) == 0)
          result += u8"↘";
        else if (strncmp(t, "<R>", 3) == 0)
          result += u8"↗";
        else shouldContinue = false;
        if (shouldContinue) {
          i = j + 2;
          continue;
        }
      }
      if (retro) {
        --j;
      }
      if (!backslash) result += symbols[c - ' '];
      else {
        switch (c) {
          case ':': result += u8"ˑ"; break;
          case '@': result += u8"ɘ"; break;
          case '3': result += u8"ɞ"; break;
          case '?': result += u8"ʕ"; break;
          case '<': result += u8"ʢ"; break;
          case '>': result += u8"ʡ"; break;
          case '!': result += u8"ǃ"; break;
          case '=': result += u8"ǂ"; break;
          case '-': result += u8"‿"; break;
          default:
          return std::string("Error: can't have backslash on ") + c + ". So far I have /" + result + "/.";
        }
      }
    } else switch (c) {
      case '^': result += "ꜛ"; break;
      case '|': {
        if (backslash) {
          if (index(xs, j) == '|' && index(xs, j + 1) == '\\') {
            j += 2;
            result += u8"ǁ";
          } else result += u8"ǀ";
        } else if (index(xs, j) != '|') {
          result += u8"|";
        } else {
          result += u8"‖";
          ++j;
        }
      }
      break;
      case '{': result += u8"æ"; break;
      case '}': result += u8"ʉ"; break;
      case '~': result += u8"̃"; break;
      case '`': result += u8"˞"; break;
      default:
      return std::string("Error: unrecognised symbol ") + c + ". So far I have /" + result + "/.";
    }
    i = j;
    std::cout << result << '\n';
  }
  return result;
}