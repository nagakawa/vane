#include <fstream>
#include <iostream>
#include <regex>
#include <unordered_map>

#include <discordpp/bot.hh>
#include <discordpp/discordpp.hh>

#include "baseconv.h"
#include "commands.h"
#include "ipa.h"

std::string getToken() {
  std::fstream f("token", std::fstream::in);
  std::string s;
  std::getline(f, s);
  return s;
}

bool respondWithIPA(nlohmann::json response) {
  static std::regex xslash("x/([^/]+)/");
  std::string message = response["d"]["content"];
  auto begin = std::sregex_iterator(message.begin(), message.end(), xslash);
  auto end = std::sregex_iterator();
  if (begin != end) {
    std::string sid = response["d"]["channel_id"];
    auto id = std::stoull(sid);
    for (auto i = begin; i != end; ++i) {
      std::smatch match = *i;
      std::string matchString = match[1];
      discordpp::DiscordAPI::channels::messages::create(id, xsampaToIPA(matchString));
    }
    return true;
  }
  return false;
}

bool respondWithBaseConv(nlohmann::json response) {
  static std::regex bcregex(R"(BASECONV\[(\d+)>(\d+)\]\s*(\w+))");
  std::string message = response["d"]["content"];
  auto begin = std::sregex_iterator(message.begin(), message.end(), bcregex);
  auto end = std::sregex_iterator();
  if (begin != end) {
    std::string sid = response["d"]["channel_id"];
    auto id = std::stoull(sid);
    for (auto i = begin; i != end; ++i) {
      std::smatch match = *i;
      int bfrom = std::stoi(match[1]);
      int bto = std::stoi(match[2]);
      std::string str = match[3];
      // std::cerr << bfrom << ' ' << bto << ' ' << str << '\n';
      auto res = convertBase(str.c_str(), bfrom, bto);
      if (std::holds_alternative<std::string>(res)) {
        discordpp::DiscordAPI::channels::messages::create(id, std::get<std::string>(res));
      } else {
        auto be = std::get<BaseError>(res);
        std::string message;
        switch (be.code) {
        case BE_INVALID_DIGIT:
          message += "Character ";
          message += (char) be.info;
          message += " is invalid in base ";
          message += std::to_string(bfrom);
          message += ".";
          break;
        case BE_TOO_BIG:
          message = "Integer is too big to fit!";
          break;
        case BE_BASE_OUT_OF_RANGE:
          message = "Base must be between 2 and 36, inclusive.";
          break;
        default:
          message += "Unknown error with code ";
          message += std::to_string(be.code);
          message += ".";
        }
        discordpp::DiscordAPI::channels::messages::create(id, message);
      }
    }
    return true;
  }
  return false;
}

void respondToMessage(discordpp::Bot* bot, nlohmann::json response) {
  if (response["d"]["author"]["id"] == bot->me["id"]) return;
  std::cout << response << '\n';
  auto iterator = customCommands.find(response["d"]["content"]);
  if (iterator != customCommands.end()) {
    iterator->second(bot, response);
  } else {
    respondWithIPA(response);
    respondWithBaseConv(response);
  }
}

int main() {
  std::cout << "Starting bot...\n";
  discordpp::Bot bot(getToken());
  bot.addResponse("MESSAGE_CREATE", respondToMessage);
  //bot.addResponse("PRESENCE_UPDATE", respondToJoin);
  //bot.addResponse("TYPING_START", respondToJoin);
  bot.start();
  return 0;
}
