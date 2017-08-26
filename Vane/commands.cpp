#include "commands.h"

#include <time.h>

#include "say.h"

bool isAprilFools() {
  time_t now = time(nullptr);
  tm* nowTM = localtime(&now);
  return nowTM->tm_mon == 3 && nowTM->tm_mday == 1;
}

static std::string INFO =
  "I am VaneBot, written by Uruwi.\n"
  "I was created for use in the AGC Discord server.\n"
  "I am programmed in C++, using Aidoboy's library \\<<https://github.com/discordpp/discordpp>>.\n"
  "Check out my code at \\<<https://github.com/nagakawa/vane>>.\n"
  "(Hint: use `x/.../` to convert X-SAMPA to IPA, or `BASECONV[10>16] 20000` for base conversion!)"
  ;
static std::string INFO_APRIL =
  "hAI me Iz vANeBot! me iZ oohrUDEwhee's dAUgHter!!!\n"
  "oOhRudeWHEE :spoon:ed oNE DaY iN le BEEGEEVEE SERVER!!! OMG!!!\n"
  "lolololol HAHA Me iz progRAMMed in SEE PLUS PLUS!!! OMG kan u believes it???\n"
  "kthxbye"
  ;

void respondInfo(discordpp::Bot* bot, nlohmann::json response, const std::string& /*ignored*/) {
  (void) bot;
  std::string sid = response["channel_id"];
  auto id = std::stoull(sid);
  say(
    bot, id,
    isAprilFools() ? INFO_APRIL : INFO
  );
}

std::string commandString() {
  std::string message =
    isAprilFools() ?
    "sAY These things too meh!!!1111\n```" :
    "Here is a list of my commands:\n```";
  for (const auto& p : customCommands) {
    message += '&';
    message += p.first;
    message += '\n';
  }
  message += "```";
  return message;
}

static std::string cs;

void respondCommands(discordpp::Bot* bot, nlohmann::json response, const std::string& /*ignored*/) {
  std::string sid = response["channel_id"];
  auto id = std::stoull(sid);
  if (cs.length() == 0) cs = commandString();
  say(bot, id, cs);
}

std::unordered_map<std::string, std::function<void(discordpp::Bot*, nlohmann::json, const std::string&)>> customCommands = {
  {"about", respondInfo},
  {"help", respondCommands},
};

std::string trimWhitespace(const std::string& s) {
  size_t first = s.find_first_not_of(" \t\n\r");
  if (first == std::string::npos) return "";
  size_t last = s.find_last_not_of(" \t\n\r");
  return s.substr(first, last - first + 1);
}