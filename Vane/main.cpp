#include <fstream>
#include <iostream>
#include <unordered_map>

#include <discordpp/bot.hh>
#include <discordpp/discordpp.hh>

#include <commands.h>

std::string getToken() {
  std::fstream f("token", std::fstream::in);
  std::string s;
  std::getline(f, s);
  return s;
}

void respondToMessage(discordpp::Bot* bot, nlohmann::json response) {
  if (response["d"]["author"]["id"] == bot->me["id"]) return;
  std::cout << response << '\n';
  auto iterator = customCommands.find(response["d"]["content"]);
  if (iterator != customCommands.end()) {
    iterator->second(bot, response);
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
