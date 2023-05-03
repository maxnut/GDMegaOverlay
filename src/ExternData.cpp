#include "ExternData.h"

float ExternData::screenSize = 0, ExternData::oldScreenSize;

std::vector<std::string> ExternData::musicPaths;
std::filesystem::path ExternData::path;
std::vector<std::function<void()>> ExternData::imguiFuncs, ExternData::openFuncs, ExternData::closeFuncs;
DiscordManager ExternData::ds;
json ExternData::bypass, ExternData::creator, ExternData::global, ExternData::level, ExternData::player, ExternData::variables;
int ExternData::amountOfClicks, ExternData::amountOfReleases, ExternData::amountOfMediumClicks, ExternData::steps;
float ExternData::tps, ExternData::screenFps;
bool ExternData::show, ExternData::fake, ExternData::isCheating, ExternData::holdingAdvance;
char ExternData::searchbar[30];
std::string ExternData::hackName;

bool ExternData::resetWindows = false, ExternData::repositionWindows = false, ExternData::saveWindows = false;

Windows ExternData::windowPositions;

std::map<std::string, nlohmann::json> ExternData::settingFiles;