#include "ReplayPlayer.h"
#include "bools.h"
#include "filesystem"
#include "fmod.hpp"
#include "pch.h"
#include "speedhackaudio.h"
#include "subprocess.hpp"
#include <ShlObj_core.h>
#include <curl/curl.h>
#include <fstream>
#include <vector>
#pragma comment(lib, "shell32")
#include "DiscordManager.h"
#include "ExternData.h"
#include "PlayLayer.h"
#include "json.hpp"
#include <functional>
#include <random>
#include <shellapi.h>
#include "ConstData.h"

#define STATUSSIZE 14

static DWORD libcocosbase = (DWORD)GetModuleHandleA("libcocos2d.dll");
extern struct HacksStr hacks;
extern struct Labels labels;
extern struct Debug debug;

using json = nlohmann::json;

namespace Hacks
{

static std::vector<bool> cheatCheck;
static const std::vector<std::string> cheatVector = {
	"0x60554",	"0x60753",	"0x207328", "0x206921", "0x20612C", "0x2062C2", "0x20456D", "0x20456D",
	"0x204D08", "0x2061BC", "0x20A23C", "0x20A34F", "0x205347", "0x20456D", "0x20456D", "0x20CEA4",
	"0x254343", "0x205161", "0x203519", "0x1E9141", "0x1EBAE8", "0x203DA2", "0x1E9C50", "0x1E9E30", "0x1E9F6B"};

static const std::vector<std::string> cheatNames = {"No Rotation",
													"Force Visibility",
													"Camera V2",
													"Force Don't Enter",
													"Force Don't Fade",
													"No Shade Blocks Effect",
													"Force Block Type",
													"Everything Hurts",
													"No Mirror",
													"Force Objects Invisible",
													"No-clip",
													"No-clip Legacy",
													"No Spikes",
													"No Hitbox",
													"No Solids",
													"No Ghost Trail Trigger",
													"No Hide Trigger",
													"No Blocks",
													"Freeze Player",
													"Jump Hack",
													"High FPS Rotation Fix",
													"No Rotation Cube",
													"No Rotation Ball",
													"Suicide",
													"FPS Bypass",
													"TPS Bypass",
													"Draw Divide",
													"Autoclicker",
													"Speedhack",
													"Toggle Playing",
													"Layout Mode",
													"Hitbox Multiplier",
													"Show Hitboxes",
													"Hide Pause Menu",
													"2P One Key",
													"Show Trajectory",
													"Wave Trail Size"};

void RenderMain();

static std::string utf16ToUTF8(const std::wstring& s)
{
	const int size = ::WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, NULL, 0, 0, NULL);

	std::vector<char> buf(size);
	::WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, &buf[0], size, 0, NULL);

	return std::string(&buf[0]);
}

static void writeOutput(std::string out)
{
	std::ofstream file("output.log", std::fstream::app);
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	std::ostringstream s;
	s << std::put_time(&tm, "%H:%M:%S");
	file << "\n" << s.str() << " " << out;
	file.close();
}

static void writeOutput(int out)
{
	std::ofstream file("output.log", std::fstream::app);
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	std::ostringstream s;
	s << std::put_time(&tm, "%H:%M:%S");
	file << "\n" << s.str() << " " << out;
	file.close();
}

static void writeOutput(size_t out)
{
	std::ofstream file("output.log", std::fstream::app);
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	std::ostringstream s;
	s << std::put_time(&tm, "%H:%M:%S");
	file << "\n" << s.str() << " " << out;
	file.close();
}

static void writeOutput(float out)
{
	std::ofstream file("output.log", std::fstream::app);
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	std::ostringstream s;
	s << std::put_time(&tm, "%H:%M:%S");
	file << "\n" << s.str() << " " << out;
	file.close();
}

static void writeOutput(double out)
{
	std::ofstream file("output.log", std::fstream::app);
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	std::ostringstream s;
	s << std::put_time(&tm, "%H:%M:%S");
	file << "\n" << s.str() << " " << out;
	file.close();
}

template <class T>
static void writeOutput(T out)
{
	std::ofstream file("output.log", std::fstream::app);
	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	std::ostringstream s;
	s << std::put_time(&tm, "%H:%M:%S");
	file << "\n" << s.str() << " " << out;
	file.close();
}

static bool writeBytes(std::uintptr_t const address, std::vector<uint8_t> const& bytes)
{
	return WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<LPVOID>(address), bytes.data(), bytes.size(),
							  nullptr);
}

template <class T> T Read(uint32_t vaddress)
{
	T buf;
	return ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(vaddress), &buf, sizeof(T), NULL) ? buf : T();
}

static std::vector<uint8_t> ReadBytes(uint32_t vaddress, size_t size)
{
	std::vector<std::uint8_t> buffer;
	for (size_t i = 0; i < size; ++i)
	{
		buffer.push_back(Read<std::uint8_t>(vaddress + i));
	}
	return buffer;
}

template <class T> bool Write(uint32_t vaddress, const T& value)
{
	DWORD oldProtect = 0;
	VirtualProtectEx(GetCurrentProcess(), reinterpret_cast<void*>(vaddress), 256, PAGE_EXECUTE_READWRITE, &oldProtect);
	return WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void*>(vaddress), &value, sizeof(T), NULL);
}

template <class T> void WriteRef(uint32_t vaddress, const T& value)
{
	DWORD old_prot;
	VirtualProtect((void*)(vaddress), sizeof(size_t), PAGE_EXECUTE_READWRITE, &old_prot);
	auto x = new T;
	*x = value;
	*reinterpret_cast<T**>(vaddress) = x;
	VirtualProtect((void*)(vaddress), sizeof(size_t), old_prot, &old_prot);
}

static std::string narrow(const wchar_t* str)
{
	int size = WideCharToMultiByte(CP_UTF8, 0, str, -1, nullptr, 0, nullptr, nullptr);
	if (size <= 0)
	{ /* fuck */
	}
	auto buffer = new char[size];
	WideCharToMultiByte(CP_UTF8, 0, str, -1, buffer, size, nullptr, nullptr);
	std::string result(buffer, size_t(size) - 1);
	delete[] buffer;
	return result;
}
static inline auto narrow(const std::wstring& str)
{
	return narrow(str.c_str());
}

static std::wstring widen(const char* str)
{
	int size = MultiByteToWideChar(CP_UTF8, 0, str, -1, nullptr, 0);
	if (size <= 0)
	{ /* fuck */
	}
	auto buffer = new wchar_t[size];
	MultiByteToWideChar(CP_UTF8, 0, str, -1, buffer, size);
	std::wstring result(buffer, size_t(size) - 1);
	delete[] buffer;
	return result;
}
static inline auto widen(const std::string& str)
{
	return widen(str.c_str());
}

static std::string getDifficultyName(gd::GJGameLevel& level)
{
	if (level.autoLevel)
	{
		return "auto";
	}

	if (level.ratingsSum != 0)
		level.difficulty = level.ratingsSum / 10;

	if (level.demon)
	{
		switch (level.demonDifficulty)
		{
		case 3:
			return "easy_demon";
		case 4:
			return "medium_demon";
		case 5:
			return "insane_demon";
		case 6:
			return "extreme_demon";
		default:
		case 0:
			return "hard_demon";
		}
	}

	switch (level.difficulty)
	{
	case gd::kGJDifficultyEasy:
		return "easy";
	case gd::kGJDifficultyNormal:
		return "normal";
	case gd::kGJDifficultyHard:
		return "hard";
	case gd::kGJDifficultyHarder:
		return "harder";
	case gd::kGJDifficultyInsane:
		return "insane";
	case gd::kGJDifficultyDemon:
		return "hard_demon";
	default:
		return "na";
	}
}

static int randomInt(int min, int max)
{
	std::random_device device;
	std::mt19937 generator(device());
	std::uniform_int_distribution<int> distribution(min, max);

	return distribution(generator);
}

static std::vector<std::string> splitByDelim(const std::string& str, char delim)
{
	std::vector<std::string> tokens;
	size_t pos = 0;
	size_t len = str.length();
	tokens.reserve(len / 2); // allocate memory for expected number of tokens

	while (pos < len)
	{
		size_t end = str.find_first_of(delim, pos);
		if (end == std::string::npos)
		{
			tokens.emplace_back(str.substr(pos));
			break;
		}
		tokens.emplace_back(str.substr(pos, end - pos));
		pos = end + 1;
	}

	return tokens;
};

static void FPSBypass(float value)
{
	if (value <= 1)
		value = 60;
	CCDirector::sharedDirector()->setAnimationInterval(1.0f / value);
}

static void Speedhack(float value)
{
	if (value <= 0)
		value = 1;
	CCDirector::sharedDirector()->getScheduler()->setTimeScale(value);
}

static std::vector<uint8_t> HexToBytes(const std::string& hex)
{
	std::vector<uint8_t> bytes;
	for (unsigned int i = 0; i < hex.length(); i += 3)
	{
		std::string byteString = hex.substr(i, 2);
		uint8_t byte = (uint8_t)strtol(byteString.c_str(), nullptr, 16);
		bytes.push_back(byte);
	}
	return bytes;
}

static void ToggleJSONHack(json& js, size_t index, bool toggle)
{
	if (toggle)
		js["mods"][index]["toggle"] = !js["mods"][index]["toggle"];
	for (size_t j = 0; j < js["mods"][index]["opcodes"].size(); j++)
	{
		std::string add = js["mods"][index]["opcodes"][j]["address"].get<std::string>();
		unsigned int address = std::stoul(add, nullptr, 16);
		std::string opc;
		if (js["mods"][index]["toggle"])
			opc = js["mods"][index]["opcodes"][j]["on"].get<std::string>();
		else
			opc = js["mods"][index]["opcodes"][j]["off"].get<std::string>();
		Hacks::writeBytes(js["mods"][index]["opcodes"][j]["lib"].get<std::string>() == "libcocos2d.dll"
							  ? libcocosbase + address
							  : gd::base + address,
						  HexToBytes(opc));
	}
}

static void ToggleJSONHack(json& js, std::string name, bool toggle)
{
	if (toggle)
		js["mods"][name]["toggle"] = !js["mods"][name]["toggle"];
	for (size_t j = 0; j < js["mods"][name]["opcodes"].size(); j++)
	{
		std::string add = js["mods"][name]["opcodes"][j]["address"].get<std::string>();
		unsigned int address = std::stoul(add, nullptr, 16);
		std::string opc;
		if (js["mods"][name]["toggle"])
			opc = js["mods"][name]["opcodes"][j]["on"].get<std::string>();
		else
			opc = js["mods"][name]["opcodes"][j]["off"].get<std::string>();
		Hacks::writeBytes(js["mods"][name]["opcodes"][j]["lib"].get<std::string>() == "libcocos2d.dll"
							  ? libcocosbase + address
							  : gd::base + address,
						  HexToBytes(opc));
	}
}

static void SaveSettings()
{

	std::ofstream f;
	f.open("GDMenu/settings.bin", std::fstream::binary);
	if (f)
		f.write((char*)&hacks, sizeof(HacksStr));
	f.close();
	f.open("GDMenu/labels.bin", std::fstream::binary);
	if (f)
		f.write((char*)&labels, sizeof(HacksStr));
	f.close();

	if (ExternData::bypass.contains("data"))
	{
		f.open("GDMenu/mod/bypass.json");
		if (f)
			f << ExternData::bypass.dump(4);
		f.close();
	}

	if (ExternData::creator.contains("data"))
	{
		f.open("GDMenu/mod/creator.json");
		if (f)
			f << ExternData::creator.dump(4);
		f.close();
	}

	if (ExternData::global.contains("data"))
	{
		f.open("GDMenu/mod/global.json");
		if (f)
			f << ExternData::global.dump(4);
		f.close();
	}

	if (ExternData::level.contains("data"))
	{
		f.open("GDMenu/mod/level.json");
		if (f)
			f << ExternData::level.dump(4);
		f.close();
	}

	if (ExternData::player.contains("data"))
	{
		f.open("GDMenu/mod/player.json");
		if (f)
			f << ExternData::player.dump(4);
		f.close();
	}

	f.open("GDMenu/windows.json");
	if (f)
		f << ExternData::windowPositions.dump(4);
	f.close();

	f.open("GDMenu/dll/extensions.json");
	if (f)
		f << ExternData::dlls.dump(4);
	f.close();

	for (auto s : ExternData::settingFiles)
	{
		auto path = "GDMenu/extsettings/" + s.first + ".json";
		f.open(path);
		if (f)
			f << s.second.dump(4);
		f.close();
	}
}

static void Priority(int priority)
{
	switch (priority)
	{
	case 0:
		SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
		break;
	case 1:
		SetPriorityClass(GetCurrentProcess(), BELOW_NORMAL_PRIORITY_CLASS);
		break;
	case 2:
		SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
		break;
	case 3:
		SetPriorityClass(GetCurrentProcess(), ABOVE_NORMAL_PRIORITY_CLASS);
		break;
	case 4:
		SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
		break;
	}
}

static void AnticheatBypass()
{
	writeBytes(gd::base + 0x202AAA, {0xEB, 0x2E});
	writeBytes(gd::base + 0x15FC2E, {0xEB});
	writeBytes(gd::base + 0x1FD557, {0xEB, 0x0C});
	writeBytes(gd::base + 0x1FD742, {0xC7, 0x87, 0xE0, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xC7, 0x87, 0xE4,
									 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
	writeBytes(gd::base + 0x1FD756, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
	writeBytes(gd::base + 0x1FD79A, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
	writeBytes(gd::base + 0x1FD7AF, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
	writeBytes(gd::base + 0x20D3B3, {0x90, 0x90, 0x90, 0x90, 0x90});
	writeBytes(gd::base + 0x1FF7A2, {0x90, 0x90});
	writeBytes(gd::base + 0x18B2B4, {0xB0, 0x01});
	writeBytes(gd::base + 0x20C4E6, {0xE9, 0xD7, 0x00, 0x00, 0x00, 0x90});
}

static std::string GetSongFolder()
{
	bool standardPath = !gd::GameManager::sharedState()->getGameVariable("0033");
	if (standardPath)
		return CCFileUtils::sharedFileUtils()->getWritablePath();
	else
		return CCFileUtils::sharedFileUtils()->getWritablePath2() + "Resources/";
}

static void MenuMusic()
{
	if (hacks.replaceMenuMusic)
	{
		ExternData::path.clear();
		gd::GameSoundManager::sharedState()->stopBackgroundMusic();
		if (hacks.randomMusic)
		{
			hacks.randomMusicIndex = rand() % ExternData::musicPaths.size();
			gd::GameSoundManager::sharedState()->playBackgroundMusic(true,
																	 ExternData::musicPaths[hacks.randomMusicIndex]);
		}
		else
		{
			gd::GameSoundManager::sharedState()->playBackgroundMusic(true,
																	 GetSongFolder() + "/" + hacks.menuSongId + ".mp3");
		}
	}
}

static void ChangePitch(float pitch)
{
	std::string path = GetSongFolder() + hacks.pitchId + ".mp3";
	debug.debugString = path;

	std::thread([&, path, pitch]() {
		{
			std::stringstream stream;
			stream << "ffmpeg -y -i " << '"' << path << '"' << " -af " << '"' << "rubberband=pitch=" << pitch
				   << ":pitchq=consistency:smoothing=on" << '"' << " " << '"' << GetSongFolder() << "/out.mp3" << '"';
			auto process = subprocess::Popen(stream.str());
			if (process.close())
			{
				return;
			}
		}
		std::filesystem::remove(Hacks::widen(path));
		std::filesystem::rename(GetSongFolder() + "/out.mp3", Hacks::widen(path));
	}).detach();
}

static void NongDownload(char* url, char* id)
{
	std::thread([&, url, id]() {
		{
			std::stringstream stream;

			stream << "GDMenu/yt-dlp -f bestaudio[ext=m4a] --output audiofile.m4a " << url;

			auto process = subprocess::Popen(stream.str());
			if (process.close())
			{
				return;
			}
		}

		{
			std::stringstream stream;
			stream << "ffmpeg -y -i audiofile.m4a -c:v copy -c:a libmp3lame -q:a 4 " << '"' << GetSongFolder() << "/"
				   << id << ".mp3" << '"';
			auto process = subprocess::Popen(stream.str());
			if (process.close())
			{
				return;
			}
		}

		std::filesystem::remove(Hacks::widen("audiofile.m4a"));
	}).detach();
}

static void UpdateRichPresence(int state, gd::GJGameLevel* lvl = 0, std::string bestrun = "")
{
	if (ExternData::ds.core && hacks.discordRPC)
	{
		std::string supportString;
		auto pl = gd::GameManager::sharedState()->getPlayLayer();
		auto el = gd::GameManager::sharedState()->getEditorLayer();
		discord::Activity activity{};
		switch (state)
		{
		case 0:
			supportString = hacks.levelPlayDetail;

			while (supportString.find("{name}") != std::string::npos)
				supportString.replace(supportString.find("{name}"), 6, pl->m_level->levelName);

			while (supportString.find("{author}") != std::string::npos)
				supportString.replace(supportString.find("{author}"), 8,
									  pl->m_level->levelID < 100 && pl->m_level->levelID > 0 ? "RobTop"
									  : pl->m_level->userName == ""							 ? "Unknown"
																							 : pl->m_level->userName);

			while (supportString.find("{best}") != std::string::npos)
				supportString.replace(supportString.find("{best}"), 6, std::to_string(pl->m_level->normalPercent));

			while (supportString.find("{stars}") != std::string::npos)
				supportString.replace(supportString.find("{stars}"), 7, std::to_string(pl->m_level->stars));

			while (supportString.find("{id}") != std::string::npos)
				supportString.replace(supportString.find("{id}"), 4, std::to_string(pl->m_level->levelID));

			while (supportString.find("{run}") != std::string::npos)
				supportString.replace(supportString.find("{run}"), 5, bestrun);

			activity.SetDetails(supportString.c_str());

			supportString = hacks.levelPlayState;

			while (supportString.find("{name}") != std::string::npos)
				supportString.replace(supportString.find("{name}"), 6, pl->m_level->levelName);

			while (supportString.find("{author}") != std::string::npos)
				supportString.replace(supportString.find("{author}"), 8,
									  pl->m_level->levelID < 100 && pl->m_level->levelID > 0 ? "RobTop"
									  : pl->m_level->userName == ""							 ? "Unknown"
																							 : pl->m_level->userName);

			while (supportString.find("{best}") != std::string::npos)
				supportString.replace(supportString.find("{best}"), 6, std::to_string(pl->m_level->normalPercent));

			while (supportString.find("{stars}") != std::string::npos)
				supportString.replace(supportString.find("{stars}"), 7, std::to_string(pl->m_level->stars));

			while (supportString.find("{id}") != std::string::npos)
				supportString.replace(supportString.find("{id}"), 4, std::to_string(pl->m_level->levelID));

			while (supportString.find("{run}") != std::string::npos)
				supportString.replace(supportString.find("{run}"), 5, bestrun);

			activity.SetState(supportString.c_str());

			activity.GetAssets().SetSmallImage(Hacks::getDifficultyName(*pl->m_level).c_str());

			break;
		case 1:
			supportString = hacks.editorDetail;
			while (supportString.find("{name}") != std::string::npos)
				supportString.replace(supportString.find("{name}"), 6, lvl->levelName);

			while (supportString.find("{objects}") != std::string::npos)
				supportString.replace(supportString.find("{objects}"), 9, std::to_string(lvl->objectCount));

			activity.SetDetails(supportString.c_str());

			supportString = hacks.editorState;

			while (supportString.find("{name}") != std::string::npos)
				supportString.replace(supportString.find("{name}"), 6, lvl->levelName);

			while (supportString.find("{objects}") != std::string::npos)
				supportString.replace(supportString.find("{objects}"), 9, std::to_string(lvl->objectCount));

			activity.SetState(supportString.c_str());

			activity.GetAssets().SetSmallImage("editor");
			break;

		case 2:
			activity.SetState(hacks.menuState);
			activity.SetDetails(hacks.menuDetail);
			break;
		}

		activity.GetTimestamps().SetStart(ExternData::ds.timeStart);
		activity.GetAssets().SetLargeText(gd::GameManager::sharedState()->m_sPlayerName.c_str());
		activity.GetAssets().SetLargeImage("cool");
		activity.SetType(discord::ActivityType::Playing);
		ExternData::ds.core->ActivityManager().UpdateActivity(activity, [](discord::Result result) {});
	}
}
}; // namespace Hacks