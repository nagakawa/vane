#pragma once

#include <string.h>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "utf8.h"

void readFilterFiles();

bool isNonceChar(char c);
bool canSay(const std::string& s);