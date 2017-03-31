#include <fstream>
#include <iostream>
#include <regex>
#include <unordered_map>

#include <discordpp/bot.hh>
#include <discordpp/discordpp.hh>

#include "commands.h"
#include "ipa.h"

std::string getToken() {
  std::fstream f("token", std::fstream::in);
  std::string s;
  std::getline(f, s);
  return s;
}

void respondWithIPA(nlohmann::json response) {
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
  }
}

void respondToMessage(discordpp::Bot* bot, nlohmann::json response) {
  if (response["d"]["author"]["id"] == bot->me["id"]) return;
  std::cout << response << '\n';
  auto iterator = customCommands.find(response["d"]["content"]);
  if (iterator != customCommands.end()) {
    iterator->second(bot, response);
  } else respondWithIPA(response);
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
