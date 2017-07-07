#include <fstream>
#include <iostream>
#include <regex>
#include <unordered_map>

#include <discordpp/bot.hh>
#include <discordpp/discordpp.hh>

#include "baseconv.h"
#include "commands.h"
#include "filter.h"
#include "ipa.h"

std::string getToken() {
  std::fstream f("token", std::fstream::in);
  std::string s;
  std::getline(f, s);
  return s;
}

bool respondWithFilter(nlohmann::json response) {
  static std::regex qslash("\\?/([^/]+)/");
  std::string message = response["d"]["content"];
  auto begin = std::sregex_iterator(message.begin(), message.end(), qslash);
  auto end = std::sregex_iterator();
  if (begin != end) {
    std::string sid = response["d"]["channel_id"];
    auto id = std::stoull(sid);
    for (auto i = begin; i != end; ++i) {
      std::smatch match = *i;
      std::string matchString = match[1];
      bool allowed = canSay(matchString);
      std::string message = "`";
      message += matchString;
      message += "` can";
      if (!allowed) message += "'t";
      message += " be said";
      discordpp::DiscordAPI::channels::messages::create(id, message);
    }
    return true;
  }
  return false;
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
  (void) bot;
  if (response["d"]["author"]["id"] == bot->me["id"]) return;
  auto iterator = customCommands.find(response["d"]["content"]);
  if (iterator != customCommands.end()) {
    iterator->second(bot, response);
  } else {
    respondWithIPA(response);
    respondWithBaseConv(response);
    respondWithFilter(response);
  }
}

uint64_t getRedRoleID(uint64_t guildID) {
  auto roles = discordpp::DiscordAPI::call("/guilds/" + std::to_string(guildID) + "/roles", discordpp::data::lastToken(), {}, "GET");
  uint64_t roleID = 0;
  for (const auto& role : roles) {
    if (role["name"] == "Can't say that name!") {
      roleID = std::stoull((std::string) role["id"]);
      break;
    }
  }
  return roleID;
}

void setOrResetRedRole(uint64_t guildID, uint64_t userID, uint64_t roleID, bool allowed) {
  if (allowed)
    discordpp::DiscordAPI::call("/guilds/" + std::to_string(guildID) + "/members/" + std::to_string(userID) + "/roles/" + std::to_string(roleID), discordpp::data::lastToken(), "a", "DELETE");
  else
    discordpp::DiscordAPI::call("/guilds/" + std::to_string(guildID) + "/members/" + std::to_string(userID) + "/roles/" + std::to_string(roleID), discordpp::data::lastToken(), "a", "PUT");
}

std::string getNickOrName(const nlohmann::json& info) {
  if (info.find("nick") == info.end() || info["nick"].is_null())
    return (std::string) info["user"]["username"];
  return (std::string) info["nick"];
}

void respondToUserUpdate(discordpp::Bot* bot, nlohmann::json response) {
  (void) bot;
  auto guildID = std::stoull((std::string) response["d"]["guild_id"]);
  std::string nick = getNickOrName(response["d"]);
  bool allowed = canSay(nick);
  //auto roles = discordpp::DiscordAPI::guilds::roles::get(guildID);
  uint64_t roleID = getRedRoleID(guildID);
  if (roleID == 0) return;
  auto userID = std::stoull((std::string) response["d"]["user"]["id"]);
  setOrResetRedRole(guildID, userID, roleID, allowed);
  /*
  std::string sid = response["d"]["channel_id"];
  auto id = std::stoull(sid);
  discordpp::DiscordAPI::channels::messages::create(id, "```json\n" + s + "\n```");
  */
}

constexpr uint64_t MY_GUILD = 99960841276768256LL;

void updateAllUserRedRole(discordpp::Bot* bot, nlohmann::json response) {
  (void) bot; (void) response;
  uint64_t guildID = MY_GUILD;
  auto users = discordpp::DiscordAPI::call("/guilds/" + std::to_string(guildID) + "/members?limit=1000", discordpp::data::lastToken(), {}, "GET");
  uint64_t roleID = getRedRoleID(guildID);
  if (roleID == 0) return;
  for (auto user : users) {
    auto userID = std::stoull((std::string) user["user"]["id"]);
    std::string nick = getNickOrName(user);
    bool allowed = canSay(nick);
    setOrResetRedRole(guildID, userID, roleID, allowed);
    std::cout << nick << ": ";
    std::cout << (allowed ? "allowed" : "not allowed") << '\n';
  }
}

int main() {
  try {
    readFilterFiles();
    std::cout << "Starting bot...\n";
    discordpp::Bot bot(getToken());
    bot.addResponse("MESSAGE_CREATE", respondToMessage);
    bot.addResponse("GUILD_MEMBER_UPDATE", respondToUserUpdate);
    bot.addResponse("READY", updateAllUserRedRole);
    //bot.addResponse("PRESENCE_UPDATE", respondToJoin);
    //bot.addResponse("TYPING_START", respondToJoin);
    bot.start();
    return 0;
  } catch (std::string s) {
    std::cerr << s << '\n';
    return -1;
  } catch (const char* s) {
    std::cerr << s << '\n';
    return -1;
  } catch (char* s) {
    std::cerr << s << '\n';
    return -1;
  }
}
