#include "say.h"

void say(discordpp::Bot* bot, uint64_t chanID, const std::string& message) {
  bot->call("/channels/" + std::to_string(chanID) + "/messages",
    {{"content", message}},
    "POST"
  );
}