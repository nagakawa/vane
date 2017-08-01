#pragma once

#include <stdint.h>
#include <string>

#include <discordpp/bot.hh>
#include <discordpp/rest-curlpp.hh>
#include <discordpp/websocket-websocketpp.hh>

void say(discordpp::Bot* bot, uint64_t chanID, const std::string& message);