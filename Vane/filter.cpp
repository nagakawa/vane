#include <filter.h>

#include <aho_corasick.hpp>

static std::unordered_set<std::string> redlist;
static aho_corasick::trie redlistTrie;
static std::unordered_set<std::string> redlistInner;
static aho_corasick::trie redlistInnerTrie;
static std::unordered_set<std::string> redlistMultiword;
static aho_corasick::trie redlistMultiwordTrie;
static std::unordered_set<std::string> exceptions;
static std::unordered_map<wchar_t, std::string> replacements;

// There's also a whitelist and a blacklist, but we care only
// about what can or can't be said in OPEN chat, so we ignore them.
void readFilterFiles() {
  std::fstream f1("filters/redlist.txt", std::fstream::in);
  std::string s;
  redlistTrie.case_insensitive().only_whole_words();
  redlistMultiwordTrie.case_insensitive().only_whole_words();
  redlistInnerTrie.case_insensitive();
  while (std::getline(f1, s)) {
    while (s.length() != 0 && s[s.length() - 1] == '\r')
      s.pop_back();
    // patterns like "-aa-" mean that anything with "aa" in it
    // but not in exceptions is red
    if (s[0] == '-' && s[s.length() - 1] == '-') {
      std::string inner = s.substr(1, s.length() - 2);
      redlistInner.insert(inner);
      redlistInnerTrie.insert(inner);
    } else if (s.find(' ') != std::string::npos) {
      // multi-word; these get treated differently in Wizard101,
      // so we emulate this behaviour
      redlistMultiword.insert(s);
      redlistMultiwordTrie.insert(s);
    } else {
      redlist.insert(s);
      redlistTrie.insert(s);
    }
  }
  std::fstream f2("filters/exceptions.txt", std::fstream::in);
  while (std::getline(f2, s)) {
    while (s.length() != 0 && s[s.length() - 1] == '\r')
      s.pop_back();
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    exceptions.insert(s);
  }
  std::fstream f3("filters/replacements.txt", std::fstream::in);
  while (std::getline(f3, s)) {
    while (s.length() != 0 && s[s.length() - 1] == '\r')
      s.pop_back();
    if (s[0] == '#' || s.length() == 0) continue;
    // The format is <dest> = <one or more source chars>.
    static std::regex pattern("(\\S+)\\s*=\\s*(\\S+)");
    auto begin = std::sregex_iterator(s.begin(), s.end(), pattern);
    auto end = std::sregex_iterator();
    if (begin == end) {
      throw "Line `" + s + " is malformed.";
    }
    std::smatch match = *begin;
    std::string dest = match[1];
    std::string src = match[2];
    const char* cs = src.c_str();
    while (cs[0] != '\0') {
      int code;
      int stat = utf8Next(&cs, &code);
      if (stat != 0) throw "filters/exceptions.txt is malformed";
      replacements[(wchar_t) code] = dest;
    }
  }
  std::cout << "Redlist-ordinary:\n";
  for (const std::string& s : redlist) std::cout << s << ' ';
  std::cout << "\nRedlist-inner:\n";
  for (const std::string& s : redlistInner) std::cout << s << ' ';
  std::cout << "\nRedlist-multiword:\n";
  for (const std::string& s : redlistMultiword) std::cout << s << " / ";
  std::cout << "\nExceptions:\n";
  for (const std::string& s : exceptions) std::cout << s << ' ';
  std::cout << "Replacements:\n";
  for (const auto& p : replacements) {
    std::cout << p.first << " → " << p.second << " / ";
  }
  std::cout << '\n';
}

bool isNonceChar(char c) {
  return strchr("~`!#%^&*()-_=+\\|;:'\",<.>/?", c) != nullptr;
}

/*
  TODO: strings such as "l*u*ck" should be blocked under the "-*uck-" rule,
  but currently nonce characters are all removed.
  Maybe a version that keeps only asterisks?
*/
bool canSay(const std::string& s) {
  // Returns true if this can be said in open chat,
  // false otherwise.
  // note: `gêt in` is red
  // so replace the characters
  std::string substituted;
  const char* cs = s.c_str();
  int code;
  while (cs[0] != '\0') {
    const char* i = cs;
    int stat = utf8Next(&cs, &code);
    if (stat != 0) return false;
    if (replacements.count(code) == 0) {
      for (; i < cs; ++i) substituted += *i;
    } else {
      substituted += replacements[code];
    }
  }
  std::string substitutedWithoutNonce;
  for (char c : substituted) {
    if (!isNonceChar(c)) substitutedWithoutNonce += c;
  }
  if (!redlistTrie.parse_text(substitutedWithoutNonce).empty()) return false;
  if (!redlistMultiwordTrie.parse_text(substituted).empty()) return false;
  auto innerResults = redlistInnerTrie.parse_text(substitutedWithoutNonce);
  for (const auto& match : innerResults) {
    size_t start = match.get_start();
    size_t end = match.get_end();
    while (start > 0 && !isspace(substitutedWithoutNonce[start - 1])) --start;
    while (end < substitutedWithoutNonce.length() && 
        !isspace(substitutedWithoutNonce[end])) ++end;
    std::string word = substitutedWithoutNonce.substr(start, end - start);
    std::transform(word.begin(), word.end(), word.begin(), ::tolower);
    if (exceptions.count(word) == 0) return false;
  }
  return true;
}