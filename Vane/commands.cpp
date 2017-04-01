#include "commands.h"

#include <time.h>

bool isAprilFools() {
  time_t now = std::time(nullptr);
  std::tm* nowTM = localtime(&now);
  return nowTM->tm_mon == 3 && nowTM->tm_mday == 1;
}

static std::string INFO =
  "I am VaneBot, written by Uruwi.\n"
  "I was created for use in the AGC Discord server.\n"
  "I am programmed in C++, using Aidoboy's library \\< https://github.com/Aidoboy/discordpp \\>.\n"
  "Check out my code at \\< https://github.com/nagakawa/vane \\>."
  ;
static std::string INFO_APRIL =
  "hAI me Iz vANeBot! me iZ oohrUDEwhee's dAUgHter!!!\n"
  "oOhRudeWHEE :spoon:ed oNE DaY iN le BEEGEEVEE SERVER!!! OMG!!!\n"
  "lolololol HAHA Me iz progRAMMed in SEE PLUS PLUS!!! OMG kan u believes it???\n"
  "kthxbye"
  ;

void respondInfo(discordpp::Bot* bot, nlohmann::json response) {
  (void) bot;
  std::string sid = response["d"]["channel_id"];
  auto id = std::stoull(sid);
  discordpp::DiscordAPI::channels::messages::create(
    id,
    isAprilFools() ? INFO_APRIL : INFO
  );
}

std::string commandString() {
  std::string message =
    isAprilFools() ?
    "sAY These things too meh!!!1111\n```" :
    "Here is a list of my commands:\n```";
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