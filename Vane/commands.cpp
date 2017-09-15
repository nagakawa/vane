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
static std::string KH =
  "ridia! ridia! ridia! ridiaaaaaaaaaaaaaaaaaaaaaaaaaannnnnnnnn!!!!!!!!!!\n"
  "aaaaaaaaa.......aaaa. aa...aaaggg!! ridia, ridia, ridiaaaaaaaaaaaaaaaaannnnnnnnnn!!!!!!!\n"
  "aaaaa, kuh kuh! kuuh kuuh!! suh suh! suuh suuh!! aca ti et liito aa... son son.\n"
  "ahaaa! an toan lax nia leje e ridia lutia liij fo kuuh kuuh aaa!!! kuuh kuuh! aaa!!!!\n"
  "teo! an oj lax nia laat fo muf muf! mof mof, muuf muuf! nia, nia! muf muf! kyan kyan kyun!\n"
  "ridia liij kaen ardia keno xelt e milf at ank tinka dac! aaaaaaa...aaaa...ap, aaaaaa! oooouuuuuuuugggg!\n"
  "an na nau on ti sil nok a slax 5 t'avelantis, ridia liij! aaaaaa! ank, ank e! ridia liij! ank!!! ap, aaaaaa!!!\n"
  "vei kalfia as at atm ento an na nau ati va--- teeeeeeeeeeeeeeeeeeee! teoooooooooooo!!! aaaaaaaaaaaaaarteeeeeeeeee!!!\n"
  "haaaaaaaaaaaaaaaaaaaaaaizeeeeeeeeeeeeennnn! kalfia tis de fiana daaaaaaaaaa!!!!!! ap, an xaklik yul avelantis tan et........\n"
  "RIDIA LIIJ et en fiana??????????????? teeeoooooooooooooooooo!!!!!! aaaaaaaaaaaaaaaaa!!!!!!\n"
  "aaarteeeeeeeeeeeee! kleeeeeeeeeeeeeeveeeeeeeeeeeeeeeelllllll! aaaaaaaaaaaaaaaaaaannnn! aaaaaaaaarbazaaaaaaaaaaard!!!!\n"
  "ocaaaaaaa! beo daaaaaa!!!!!! an leev! an leev van tuube fia tis-- to?? lu inor...? ridia liiz kaen olmaleis inor an xa!!!\n"
  "ridia liiz kaen avelantis xookor a men da!!!! atta... tu fia des ges xa!! (xante\n"
  "hwaaaaaaaaaaaaaaaa!!!!! an til ridia liiz!!!!! atta al an, mel! men so sen xed netal eu!\n"
  "ou, ridia liiz kaen kalfiaaaaaaaaaaaaaaa! teooooooooooooooooooooooooo!\n"
  "aaaaaaaa, aaaaaaag, ap, ap, liiza xanxaaaaaaaa! k...kmiiiiiiiirrrr! luxiaaaaaaaaaaaa!!!!!! miliaaaaaaaaa!!!!!!\n"
  "u....uuuuuuuuuuuuooooooooooooo!!!!!!! der xiilas leyute fiina ti, ridia. alem ant, ti re luko sil ridia kaen arbazard!\n"
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

void respondKH(discordpp::Bot* bot, nlohmann::json response, const std::string& /*ignored*/) {
  std::string sid = response["channel_id"];
  auto id = std::stoull(sid);
  say(bot, id, KH);
}

void respondADict(discordpp::Bot* bot, nlohmann::json response, const std::string& word) {
  std::string sid = response["channel_id"];
  auto id = std::stoull(sid);
  std::string url = "http://mindsc.ape.jp/klel/yui.cgi?rein=";
  url += word;
  url += "&ziko=ilm&axn=axnyui&ism=ismyui";
  say(bot, id, url);
}

std::unordered_map<std::string, std::function<void(discordpp::Bot*, nlohmann::json, const std::string&)>> customCommands = {
  {"about", respondInfo},
  {"help", respondCommands},
  {"kh", respondKH},
  {"adict", respondADict},
};

std::string trimWhitespace(const std::string& s) {
  size_t first = s.find_first_not_of(" \t\n\r");
  if (first == std::string::npos) return "";
  size_t last = s.find_last_not_of(" \t\n\r");
  return s.substr(first, last - first + 1);
}