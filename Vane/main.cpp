#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <regex>
#include <unordered_map>

#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <discordpp/bot.hh>
#include <discordpp/rest-curlpp.hh>
#include <discordpp/websocket-websocketpp.hh>

#include "baseconv.h"
#include "commands.h"
#include "filter.h"
#include "hacm.h"
#include "ipa.h"
#include "say.h"

using aios_ptr = std::shared_ptr<boost::asio::io_service>;
aios_ptr service;
using Handler = std::function<void(discordpp::Bot*, nlohmann::json)>;

std::string getToken() {
  std::fstream f("token", std::fstream::in);
  std::string s;
  std::getline(f, s);
  return s;
}

bool respondWithFilter(discordpp::Bot* bot, nlohmann::json response) {
  static std::regex qslash("\\?/([^/]+)/");
  std::string message = response["content"];
  auto begin = std::sregex_iterator(message.begin(), message.end(), qslash);
  auto end = std::sregex_iterator();
  if (begin != end) {
    std::string sid = response["channel_id"];
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
      say(bot, id, message);
    }
    return true;
  }
  return false;
}

bool respondWithHacm(discordpp::Bot* bot, nlohmann::json response) {
  static std::regex hslash("h/([^/]+)/");
  std::string message = response["content"];
  auto begin = std::sregex_iterator(message.begin(), message.end(), hslash);
  auto end = std::sregex_iterator();
  if (begin != end) {
    std::string sid = response["channel_id"];
    auto id = std::stoull(sid);
    for (auto i = begin; i != end; ++i) {
      std::smatch match = *i;
      std::string matchString = match[1];
      say(bot, id, latinToHacm(matchString));
    }
    return true;
  }
  return false;
}

bool respondWithIPA(discordpp::Bot* bot, nlohmann::json response) {
  static std::regex xslash("x/([^/]+)/");
  std::string message = response["content"];
  auto begin = std::sregex_iterator(message.begin(), message.end(), xslash);
  auto end = std::sregex_iterator();
  if (begin != end) {
    std::string sid = response["channel_id"];
    auto id = std::stoull(sid);
    for (auto i = begin; i != end; ++i) {
      std::smatch match = *i;
      std::string matchString = match[1];
      say(bot, id, xsampaToIPA(matchString));
    }
    return true;
  }
  return false;
}

bool respondWithBaseConv(discordpp::Bot* bot, nlohmann::json response) {
  static std::regex bcregex(R"(BASECONV\[(\d+)>(\d+)\]\s*(\w+))");
  std::string message = response["content"];
  auto begin = std::sregex_iterator(message.begin(), message.end(), bcregex);
  auto end = std::sregex_iterator();
  if (begin != end) {
    std::string sid = response["channel_id"];
    auto id = std::stoull(sid);
    for (auto i = begin; i != end; ++i) {
      std::smatch match = *i;
      int bfrom = std::stoi(match[1]);
      int bto = std::stoi(match[2]);
      std::string str = match[3];
      // std::cerr << bfrom << ' ' << bto << ' ' << str << '\n';
      auto res = convertBase(str.c_str(), bfrom, bto);
      if (std::holds_alternative<std::string>(res)) {
        say(bot, id, std::get<std::string>(res));
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
        say(bot, id, message);
      }
    }
    return true;
  }
  return false;
}

void respondToMessage(discordpp::Bot* bot, nlohmann::json response) {
  // std::cout << response.dump(4) << '\n';
  if (response["author"]["id"] == bot->me_["id"]) return;
  std::string message = response["content"];
  message = trimWhitespace(message);
  if (message[0] == '&') {
    size_t nextSpace = message.find_first_of(" \t\r\n", 1);
    std::string cmdname = message.substr(1, nextSpace - 1);
    auto iterator = customCommands.find(cmdname);
    if (iterator != customCommands.end()) {
      iterator->second(
        bot, response,
        trimWhitespace(message.substr(nextSpace + 1))
      );
    }
  }
  respondWithIPA(bot, response);
  respondWithHacm(bot, response);
  respondWithBaseConv(bot, response);
  respondWithFilter(bot, response);
}

uint64_t getRedRoleID(discordpp::Bot* bot, uint64_t guildID) {
  uint64_t roleID = 0;
  bot->call("/guilds/" + std::to_string(guildID) + "/roles", {}, "GET",
      [&roleID](discordpp::Bot*, nlohmann::json response) {
        for (const auto& role : response) {
          if (role["name"] == "Can't say that name!") {
            roleID = std::stoull((std::string) role["id"]);
            break;
          }
        }
      });
  return roleID;
}

void setOrResetRedRole(discordpp::Bot* bot, uint64_t guildID, uint64_t userID, uint64_t roleID, bool allowed) {
  try {
    if (allowed)
      bot->call("/guilds/" + std::to_string(guildID) + "/members/" + std::to_string(userID) + "/roles/" + std::to_string(roleID), {}, "DELETE");
    else
      bot->call("/guilds/" + std::to_string(guildID) + "/members/" + std::to_string(userID) + "/roles/" + std::to_string(roleID), "", "PUT");
  } catch (discordpp::ratelimit& e) {
    // Wait a bit and try again.
    std::cout << "Waiting out rate limit for " << e.millis << " milliseconds.\n";
    boost::asio::deadline_timer t(*service, boost::posix_time::milliseconds(e.millis));
    t.async_wait([bot, guildID, userID, roleID, allowed](const boost::system::error_code&) {
      setOrResetRedRole(bot, guildID, userID, roleID, allowed);
    });
  }
}

void setOrResetRedRoleIfNecessary(discordpp::Bot* bot, uint64_t guildID, uint64_t userID, uint64_t roleID, bool allowed, const nlohmann::json& rinfo) {
  bool wasAllowed = true;
  for (const auto& role : rinfo) {
    auto id = std::stoull((std::string) role);
    if (id == roleID) {
      wasAllowed = false;
      break;
    }
  }
  if (wasAllowed != allowed) setOrResetRedRole(bot, guildID, userID, roleID, allowed);
}

std::string getNickOrName(const nlohmann::json& info) {
  if (info.find("nick") == info.end() || info["nick"].is_null())
    return (std::string) info["user"]["username"];
  return (std::string) info["nick"];
}

void respondToUserUpdate(discordpp::Bot* bot, nlohmann::json response) {
  auto guildID = std::stoull((std::string) response["guild_id"]);
  std::string nick = getNickOrName(response);
  bool allowed = canSay(nick);
  //auto roles = discordpp::DiscordAPI::guilds::roles::get(guildID);
  uint64_t roleID = getRedRoleID(bot, guildID);
  if (roleID == 0) return;
  auto userID = std::stoull((std::string) response["user"]["id"]);
  setOrResetRedRoleIfNecessary(bot, guildID, userID, roleID, allowed, response["roles"]);
  /*
  std::string sid = response["d"]["channel_id"];
  auto id = std::stoull(sid);
  discordpp::DiscordAPI::channels::messages::create(id, "```json\n" + s + "\n```");
  */
}

constexpr uint64_t MY_GUILD = 99960841276768256LL;

void updateAllUserRedRole(discordpp::Bot* bot, nlohmann::json response) {
  (void) response;
  uint64_t guildID = MY_GUILD;
  uint64_t roleID = getRedRoleID(bot, guildID);
  if (roleID == 0) return;
  bot->call("/guilds/" + std::to_string(guildID) + "/members?limit=1000", {}, "GET",
    [guildID, roleID](discordpp::Bot* bot, nlohmann::json users) {
      for (const auto& user : users) {
        auto userID = std::stoull((std::string) user["user"]["id"]);
        std::string nick = getNickOrName(user);
        bool allowed = canSay(nick);
        setOrResetRedRoleIfNecessary(bot, guildID, userID, roleID, allowed, user["roles"]);
        std::cout << nick << ": ";
        std::cout << (allowed ? "allowed" : "not allowed") << '\n';
      }
    }
  );
}

int main() {
  try {
    readFilterFiles();
    std::cout << "Starting bot...\n";
    //bot.addResponse("PRESENCE_UPDATE", respondToJoin);
    //bot.addResponse("TYPING_START", respondToJoin);
    service = std::make_shared<boost::asio::io_service>();
    std::string token = getToken();
    discordpp::Bot bot(
      service,
      token,
      std::make_shared<discordpp::RestCurlPPModule>(service, token),
      std::make_shared<discordpp::WebsocketWebsocketPPModule>(service, token));
    bot.addHandler("READY", updateAllUserRedRole);
    bot.addHandler("GUILD_MEMBER_UPDATE", respondToUserUpdate);
    bot.addHandler("MESSAGE_CREATE", respondToMessage);
    service->run();
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
