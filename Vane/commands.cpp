#include "commands.h"

static std::string INFO =
  "I am VaneBot, written by Uruwi.\n"
  "I was created for use in the AGC Discord server.\n"
  "I am programmed in C++, using Aidoboy's library \\< https://github.com/Aidoboy/discordpp \\>."
  ;

void respondInfo(discordpp::Bot* bot, nlohmann::json response) {
  (void) bot;
  std::string sid = response["d"]["channel_id"];
  auto id = std::stoull(sid);
  discordpp::DiscordAPI::channels::messages::create(id, INFO);
}

std::string commandString() {
  std::string message = "Here is a list of my commands:\n```";
  for (const auto& p : customCommands) {
    message += p.first;
    message += '\n';
  }
  message += "```";
  return message;
}

static std::string cs;

void respondCommands(discordpp::Bot* bot, nlohmann::json response) {
  (void) bot;
  std::string sid = response["d"]["channel_id"];
  auto id = std::stoull(sid);
  if (cs.length() == 0) cs = commandString();
  discordpp::DiscordAPI::channels::messages::create(id, cs);
}

std::unordered_map<std::string, std::function<void(discordpp::Bot*, nlohmann::json)>> customCommands = {
  {"Who are you?", respondInfo},
  {"Please help me.", respondCommands},
};