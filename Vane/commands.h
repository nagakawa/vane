#pragma once

#include <string>
#include <unordered_map>

#include <discordpp/bot.hh>
#include <discordpp/restmodule.hh>
#include <discordpp/websocketmodule.hh>

extern std::unordered_map<std::string, std::function<void(discordpp::Bot*, nlohmann::json, const std::string&)>> customCommands;

std::string trimWhitespace(const std::string& s);