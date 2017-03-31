#include <string>
#include <unordered_map>

#include <discordpp/bot.hh>
#include <discordpp/discordpp.hh>

extern std::unordered_map<std::string, std::function<void(discordpp::Bot*, nlohmann::json)>> customCommands;