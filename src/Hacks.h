#include "pch.h"
#include "speedhackaudio.h"
#include <vector>
#include <curl/curl.h>
#include <fstream>
#include "bools.h"
#include "fmod.hpp"
#include "ReplayPlayer.h"
#include "filesystem"
#include "subprocess.hpp"
#include <ShlObj_core.h>
#pragma comment(lib, "shell32")
#include <shellapi.h>
#include "json.hpp"
#include "DiscordManager.h"

#define STATUSSIZE 14

static DWORD libcocosbase = (DWORD)GetModuleHandleA("libcocos2d.dll");
extern struct HacksStr hacks;
extern struct Labels labels;
extern struct Debug debug;

using json = nlohmann::json;

namespace Hacks
{

    static std::vector<bool> cheatCheck;
    static const std::vector<std::string> cheatVector = {"0x60554", "0x60753", "0x207328", "0x206921", "0x20612C", "0x2062C2", "0x1F9971", "0x20456D", "0x20456D", "0x204D08", "0x2061BC", "0x20A23C", "0x20A34F", "0x205347", "0x20456D", "0x20456D", "0x20CEA4", "0x254343", "0x205161", "0x203519", "0x1E9141", "0x1EBAE8", "0x203DA2", "0x1E9C50", "0x1E9E30"};

    extern std::vector<std::string> musicPaths;
    extern std::filesystem::path path;

    extern DiscordManager ds;

    extern json bypass, creator, global, level, player;

    extern int amountOfClicks, amountOfReleases, amountOfMediumClicks;

    extern float tps, screenFps;

    extern bool show, fake;

    extern std::string hackName;

    void RenderMain();

    static std::string utf16ToUTF8(const std::wstring &s)
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
        file << "\n"
             << s.str() << " " << out;
        file.close();
    }

    static void writeOutput(int out)
    {
        std::ofstream file("output.log", std::fstream::app);
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream s;
        s << std::put_time(&tm, "%H:%M:%S");
        file << "\n"
             << s.str() << " " << out;
        file.close();
    }

    static void writeOutput(float out)
    {
        std::ofstream file("output.log", std::fstream::app);
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream s;
        s << std::put_time(&tm, "%H:%M:%S");
        file << "\n"
             << s.str() << " " << out;
        file.close();
    }

    static void writeOutput(double out)
    {
        std::ofstream file("output.log", std::fstream::app);
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::ostringstream s;
        s << std::put_time(&tm, "%H:%M:%S");
        file << "\n"
             << s.str() << " " << out;
        file.close();
    }

    static bool writeBytes(std::uintptr_t const address, std::vector<uint8_t> const &bytes)
    {
        return WriteProcessMemory(
            GetCurrentProcess(),
            reinterpret_cast<LPVOID>(address),
            bytes.data(),
            bytes.size(),
            nullptr);
    }

    template <class T>
    T Read(uint32_t vaddress)
    {
        T buf;
        return ReadProcessMemory(GetCurrentProcess(), reinterpret_cast<void *>(vaddress), &buf, sizeof(T), NULL) ? buf : T();
    }

    template <class T>
    bool Write(uint32_t vaddress, const T &value)
    {
        DWORD oldProtect = 0;
        VirtualProtectEx(GetCurrentProcess(), reinterpret_cast<void *>(vaddress), 256, PAGE_EXECUTE_READWRITE, &oldProtect);
        return WriteProcessMemory(GetCurrentProcess(), reinterpret_cast<void *>(vaddress), &value, sizeof(T), NULL);
    }

    static void WriteRef(uint32_t vaddress, float value)
    {
        DWORD old_prot;
        VirtualProtect((void *)(vaddress), sizeof(size_t), PAGE_EXECUTE_READWRITE, &old_prot);
        auto x = new float;
        *x = value;
        *reinterpret_cast<float **>(vaddress) = x;
    }

    static std::string narrow(const wchar_t *str)
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
    static inline auto narrow(const std::wstring &str) { return narrow(str.c_str()); }

    static std::wstring widen(const char *str)
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
    static inline auto widen(const std::string &str) { return widen(str.c_str()); }

    static std::string getDifficultyName(gd::GJGameLevel &level)
    {
        if (level.autoLevel)
        {
            return "auto";
        }

        if(level.ratingsSum != 0) level.difficulty = level.ratingsSum / 10;

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

    //---------------OTHER--------------------
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

    static std::vector<uint8_t> HexToBytes(const std::string &hex)
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

    static void ToggleJSONHack(json &js, size_t index, bool toggle)
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
            Hacks::writeBytes(js["mods"][index]["opcodes"][j]["lib"].get<std::string>() == "libcocos2d.dll" ? libcocosbase + address : gd::base + address, HexToBytes(opc));
        }
    }

    static void ToggleJSONHack(json &js, std::string name, bool toggle)
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
            Hacks::writeBytes(js["mods"][name]["opcodes"][j]["lib"].get<std::string>() == "libcocos2d.dll" ? libcocosbase + address : gd::base + address, HexToBytes(opc));
        }
    }

    static void SaveSettings()
    {

        std::ofstream f;
        f.open("GDMenu/settings.bin", std::fstream::binary);
        if (f)
            f.write((char *)&hacks, sizeof(HacksStr));
        f.close();
        f.open("GDMenu/labels.bin", std::fstream::binary);
        if (f)
            f.write((char *)&labels, sizeof(HacksStr));
        f.close();

        if (bypass.contains("data"))
        {
            f.open("GDMenu/mod/bypass.json");
            if (f)
                f << bypass;
            f.close();
        }

        if (creator.contains("data"))
        {
            f.open("GDMenu/mod/creator.json");
            if (f)
                f << creator;
            f.close();
        }

        if (global.contains("data"))
        {
            f.open("GDMenu/mod/global.json");
            if (f)
                f << global;
            f.close();
        }

        if (level.contains("data"))
        {
            f.open("GDMenu/mod/level.json");
            if (f)
                f << level;
            f.close();
        }

        if (player.contains("data"))
        {
            f.open("GDMenu/mod/player.json");
            if (f)
                f << player;
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
        writeBytes(gd::base + 0x1FD742, {0xC7, 0x87, 0xE0, 0x02, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0xC7, 0x87, 0xE4, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
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
            path.clear();
            gd::GameSoundManager::sharedState()->stopBackgroundMusic();
            if (hacks.randomMusic)
            {
                hacks.randomMusicIndex = rand() % musicPaths.size();
                gd::GameSoundManager::sharedState()->playBackgroundMusic(true, musicPaths[hacks.randomMusicIndex]);
            }
            else
            {
                gd::GameSoundManager::sharedState()->playBackgroundMusic(true, GetSongFolder() + "/" + hacks.menuSongId + ".mp3");
            }
        }
    }

    static void ChangePitch(float pitch)
    {
        std::string path = GetSongFolder() + hacks.pitchId + ".mp3";
        debug.debugString = path;

        std::thread([&, path, pitch]()
                    {
        {
            std::stringstream stream;
            stream << "ffmpeg -y -i " << '"' << path << '"' << " -af " << '"' << "rubberband=pitch=" << pitch << ":pitchq=consistency:smoothing=on" << '"' << " " << '"' << GetSongFolder() << "/out.mp3" << '"';
            auto process = subprocess::Popen(stream.str());
            if (process.close())
            {
                return;
            }
        }
        std::filesystem::remove(Hacks::widen(path));
        std::filesystem::rename(GetSongFolder() + "/out.mp3", Hacks::widen(path)); })
            .detach();
    }
}