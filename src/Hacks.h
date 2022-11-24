#include "pch.h"
#include "speedhackaudio.h"
#include <vector>
#include <curl/curl.h>
#include <fstream>
#include "speedhack.h"
#include "bools.h"
#include "fmod.hpp"
#include "ReplayPlayer.h"
#include "filesystem"
#include <ShlObj_core.h>
#pragma comment(lib, "shell32")
#include <shellapi.h>

static DWORD libcocosbase = (DWORD)GetModuleHandleA("libcocos2d.dll");
extern struct HacksStr hacks;
extern struct Labels labels;
extern struct Debug debug;

namespace Hacks
{

    extern std::vector<std::string> musicPaths;
    extern std::filesystem::path path;

    extern int amountOfClicks, amountOfReleases, amountOfMediumClicks;

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

    //---------------PLAYER--------------------

    static void noclip(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x20A23C, {0xe9, 0x79, 0x06, 0x00, 0x00});
        else
            writeBytes(gd::base + 0x20A23C, {0x6a, 0x14, 0x8b, 0xcb, 0xff});
    }

    static void nospikes(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x205347, {0x75});
        else
            writeBytes(gd::base + 0x205347, {0x74});
    }

    static void nohitbox(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x20456D, {0xB8, 0x07, 0x00, 0x00, 0x00, 0x90});
        else
            writeBytes(gd::base + 0x20456D, {0x8B, 0x83, 0x00, 0x03, 0x00, 0x00});
    }

    static void nosolid(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x20456D, {0xB8, 0x14, 0x00, 0x00, 0x00, 0x90});
        else
            writeBytes(gd::base + 0x20456D, {0x8B, 0x83, 0x00, 0x03, 0x00, 0x00});
    }

    static void forceblocktype(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x20456D, {0x31, 0xC0, 0x83, 0x7B, 0x34, 0x00, 0xBA, 0x07, 0x00, 0x00, 0x00, 0x0F, 0x44, 0xC2, 0x90});
        else
            writeBytes(gd::base + 0x20456D, {0x8B, 0x83, 0x00, 0x03, 0x00, 0x00, 0x83, 0xF8, 0x07, 0x0F, 0x84, 0x7F, 0x0A, 0x00, 0x00});
    }

    static void everythinghurts(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x20456D, {0xB8, 0x02, 0x00, 0x00, 0x00, 0x90});
        else
            writeBytes(gd::base + 0x20456D, {0x8B, 0x83, 0x00, 0x03, 0x00, 0x00});
    }

    static void freezeplayer(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x203519, {0x90, 0x90, 0x90});
        else
            writeBytes(gd::base + 0x203519, {0x50, 0xFF, 0xD6});
    }

    static void jumphack(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0x1E9141, {0x01});
            writeBytes(gd::base + 0x1E9498, {0x01});
        }
        else
        {
            writeBytes(gd::base + 0x1E9141, {0x00});
            writeBytes(gd::base + 0x1E9498, {0x00});
        }
    }

    static void nopulse(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x2060D9, {0xEB, 0x4A});
        else
            writeBytes(gd::base + 0x2060D9, {0x74, 0x4A});
    }

    static void disablerespawnblink(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x1EF358, {0xE9, 0xBD, 0x00, 0x00, 0x00, 0x90});
        else
            writeBytes(gd::base + 0x1EF358, {0x8B, 0x80, 0x64, 0x01, 0x00, 0x00});
    }

    static void disabledeatheffect(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x1EFBE0, {0xC3});
        else
            writeBytes(gd::base + 0x1EFBE0, {0x55});
    }

    static void IgnoreESC(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x1E644C, {0x90, 0x90, 0x90, 0x90, 0x90});
        else
            writeBytes(gd::base + 0x1E644C, {0xE8, 0xBF, 0x73, 0x02, 0x00});
    }

    static void InstantComplete(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x20350D, {0xC7, 0x81, 0x7C, 0x06, 0x00, 0x00, 0x20, 0xBC, 0xBE, 0x4C, 0x90, 0x90, 0x90, 0x90, 0x90});
        else
            writeBytes(gd::base + 0x20350D, {0x8B, 0x35, 0xE0, 0x23, 0x68, 0x00, 0x81, 0xC1, 0x7C, 0x06, 0x00, 0x00, 0x50, 0xFF, 0xD6});
    }

    static void accuratepercentage(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0x2080FB, {0xFF, 0x50, 0x64, 0xF3, 0x0F, 0x10, 0x00, 0x8B, 0x87, 0xC0, 0x03, 0x00, 0x00, 0x83, 0xEC, 0x08, 0x42});
            writeBytes(gd::base + 0x208114, {0xF3, 0x0F, 0x5E, 0x87, 0xB4, 0x03, 0x00, 0x00, 0xC7, 0x02, 0x25, 0x2E, 0x32, 0x66, 0xC7, 0x42, 0x04, 0x25, 0x25, 0x00, 0x00, 0x8B, 0xB0, 0x04, 0x01, 0x00, 0x00, 0xF3, 0x0F, 0x5A, 0xC0, 0xF2, 0x0F, 0x11, 0x04, 0x24, 0x52});
            writeBytes(gd::base + 0x20813F, {0x83, 0xC4, 0x0C});
        }
        else
        {
            writeBytes(gd::base + 0x2080FB, {0xFF, 0x50, 0x64, 0xF3, 0x0F, 0x10, 0x00, 0x8B, 0x87, 0xC0, 0x03, 0x00, 0x00, 0x83, 0xEC, 0x08, 0x42});
            writeBytes(gd::base + 0x208114, {0xF3, 0x0F, 0x5E, 0x87, 0xB4, 0x03, 0x00, 0x00, 0xC7, 0x02, 0x25, 0x2E, 0x30, 0x66, 0xC7, 0x42, 0x04, 0x25, 0x25, 0x00, 0x00, 0x8B, 0xB0, 0x04, 0x01, 0x00, 0x00, 0xF3, 0x0F, 0x5A, 0xC0, 0xF2, 0x0F, 0x11, 0x04, 0x24, 0x52});
            writeBytes(gd::base + 0x20813F, {0x83, 0xC4, 0x0C});
        }
    }

    static void trailoff(bool active)
    {
        if (active)
            writeBytes(libcocosbase + 0xAEDCC, {0xE9, 0x69, 0x02, 0x00, 0x00, 0x90});
        else
            writeBytes(libcocosbase + 0xAEDCC, {0x0F, 0x84, 0x68, 0x02, 0x00, 0x00});
    }

    static void trailon(bool active)
    {
        if (active)
            writeBytes(libcocosbase + 0xAEDCC, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
        else
            writeBytes(libcocosbase + 0xAEDCC, {0x0F, 0x84, 0x68, 0x02, 0x00, 0x00});
    }

    static void inversed(bool active)
    {
        if (active)
            writeBytes(libcocosbase + 0xAEDCC, {0x0F, 0x85, 0x68, 0x02, 0x00, 0x00});
        else
            writeBytes(libcocosbase + 0xAEDCC, {0x0F, 0x84, 0x68, 0x02, 0x00, 0x00});
    }

    static void trailType(int type)
    {
        trailoff(false);
        trailon(false);
        inversed(false);
        switch (type)
        {
        case 1:
            trailoff(true);
            break;
        case 2:
            trailon(true);
            break;
        case 3:
            inversed(true);
            break;
        default:
            break;
        }
    }

    static void hitboxType(int type)
    {
        nospikes(false);
        nohitbox(false);
        nosolid(false);
        forceblocktype(false);
        everythinghurts(false);
        switch (type)
        {
        case 1:
            nospikes(true);
            break;
        case 2:
            nohitbox(true);
            break;
        case 3:
            nosolid(true);
            break;
        case 4:
            forceblocktype(true);
            break;
        case 5:
            everythinghurts(true);
            break;
        default:
            break;
        }
    }

    static void solidwavetrail(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x1E8153, {0x8B, 0x86, 0x14, 0x05, 0x00, 0x00, 0xC6, 0x80, 0x30, 0x01, 0x00, 0x00, 0x01, 0xEB, 0x2A});
        else
            writeBytes(gd::base + 0x1E8153, {0x8B, 0x81, 0x3C, 0x02, 0x00, 0x00, 0x2B, 0x81, 0x40, 0x02, 0x00, 0x00, 0x83, 0xF8, 0x0F});
    }

    static void noflash(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0x1EEC50, {0xC3});
            writeBytes(gd::base + 0x1E8365, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
            writeBytes(gd::base + 0x1EE31E, {0xE9, 0x91, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90});
        }
        else
        {
            writeBytes(gd::base + 0x1EEC50, {0x55});
            writeBytes(gd::base + 0x1E8365, {0x50, 0x8B, 0xCB, 0xE8, 0x03, 0xF5, 0x00, 0x00});
            writeBytes(gd::base + 0x1EE31E, {0x56, 0x8B, 0xCF, 0xE8, 0x4A, 0x95, 0x00, 0x00});
        }
    }

    static void noparticles(bool active)
    {
        if (active)
            writeBytes(libcocosbase + 0xB8ED6, {0x00});
        else
            writeBytes(libcocosbase + 0xB8ED6, {0x01});
    }

    static void samedualcolor(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0x1FC142, {0x8B, 0x88, 0x3C, 0x02, 0x00, 0x00, 0x2B, 0x88, 0x40, 0x02, 0x00, 0x00});
            writeBytes(gd::base + 0x1FC1C3, {0x8B, 0x81, 0x48, 0x02, 0x00, 0x00, 0x2B, 0x81, 0x4C, 0x02, 0x00, 0x00});
        }
        else
        {
            writeBytes(gd::base + 0x1FC142, {0x8B, 0x88, 0x48, 0x02, 0x00, 0x00, 0x2B, 0x88, 0x4C, 0x02, 0x00, 0x00});
            writeBytes(gd::base + 0x1FC1C3, {0x8B, 0x81, 0x3C, 0x02, 0x00, 0x00, 0x2B, 0x81, 0x40, 0x02, 0x00, 0x00});
        }
    }

    //---------------BYPASS--------------------

    static void unlockIcons(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0xC50A8, {0xB0, 0x01, 0x90, 0x90, 0x90});
            writeBytes(gd::base + 0xC54BA, {0xB0, 0x01, 0x90, 0x90, 0x90});
        }
        else
        {
            writeBytes(gd::base + 0xC50A8, {0xE8, 0x7A, 0xCD, 0x19, 0x00});
            writeBytes(gd::base + 0xC54BA, {0xE8, 0x68, 0xC9, 0x19, 0x00});
        }
    }

    static void sliderLimit(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0x2E5CA, {0xEB});
            writeBytes(gd::base + 0x2E5F8, {0xEB});
        }
        else
        {
            writeBytes(gd::base + 0x2E5CA, {0x76});
            writeBytes(gd::base + 0x2E5F8, {0x76});
        }
    }

    static void MainLevels(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x188CE1, {0xE9, 0x8A, 0x00, 0x00, 0x00, 0x90});
        else
            writeBytes(gd::base + 0x188CE1, {0x0F, 0x8E, 0x89, 0x00, 0x00, 0x00});
    }

    static void TextLength(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x21ACB, {0xEB, 0x04});
        else
            writeBytes(gd::base + 0x21ACB, {0x7C, 0x04});
    }

    static void FilterBypass(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x21A99, {0x90, 0x90});
        else
            writeBytes(gd::base + 0x21A99, {0x75, 0x04});
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

    //---------------GLOBAL--------------------

    static void noTransition(bool active)
    {
        if (active)
            writeBytes(libcocosbase + 0xA5424, {0x90, 0x90, 0x90, 0x90, 0x90});
        else
            writeBytes(libcocosbase + 0xA5424, {0xF3, 0x0F, 0x10, 0x45, 0x08});
    }

    static void noGlow(bool active)
    {
        if (active)
            writeBytes(gd::base + 0xCFF2D, {0xE9, 0xFD, 0x01, 0x00, 0x00, 0x90});
        else
            writeBytes(gd::base + 0xCFF2D, {0x8A, 0x80, 0x89, 0x02, 0x00, 0x00});
    }

    static void fastalttab(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x3D02E, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
        else
            writeBytes(gd::base + 0x3D02E, {0x8B, 0x03, 0x8B, 0xCB, 0xFF, 0x50, 0x18});
    }

    static void freeResize(bool active)
    {
        if (active)
        {
            writeBytes(libcocosbase + 0x11388B, {0x90, 0x90, 0x90, 0x90, 0x90});
            writeBytes(libcocosbase + 0x11339D, {0xB9, 0xFF, 0xFF, 0xFF, 0x7F, 0x90, 0x90});
            writeBytes(libcocosbase + 0x1133C0, {0x48});
            writeBytes(libcocosbase + 0x1133C6, {0x48});
            writeBytes(libcocosbase + 0x112536, {0xEB, 0x11, 0x90});
        }
        else
        {
            writeBytes(libcocosbase + 0x11388B, {0xE8, 0xB0, 0xF3, 0xFF, 0xFF});
            writeBytes(libcocosbase + 0x11339D, {0xE8, 0xEE, 0xF6, 0xFF, 0xFF, 0x8B, 0xC8});
            writeBytes(libcocosbase + 0x1133C0, {0x50});
            writeBytes(libcocosbase + 0x1133C6, {0x50});
            writeBytes(libcocosbase + 0x112536, {0x50, 0x6A, 0x00});
        }
    }

    static void transparentLists(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0x15C02C, {0x00, 0x00, 0x00, 0x40});
            writeBytes(gd::base + 0x5C70A, {0x60});
            writeBytes(gd::base + 0x5C6D9, {0x20, 0x20});
            writeBytes(gd::base + 0x5C6DC, {0x20});
            writeBytes(gd::base + 0x5C6CF, {0x40, 0x40});
            writeBytes(gd::base + 0x5C6D2, {0x40});
        }
        else
        {
            writeBytes(gd::base + 0x15C02C, {0xBF, 0x72, 0x3E, 0xFF});
            writeBytes(gd::base + 0x5C70A, {0xFF});
            writeBytes(gd::base + 0x5C6D9, {0xA1, 0x58});
            writeBytes(gd::base + 0x5C6DC, {0x2C});
            writeBytes(gd::base + 0x5C6CF, {0xC2, 0x72});
            writeBytes(gd::base + 0x5C6D2, {0x3E});
        }
    }

    static void transparentMenus(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0x17DEB0, {0x6A, 0x00, 0x90, 0x90, 0x90});
            writeBytes(gd::base + 0x17E3BB, {0x6A, 0x30, 0x90, 0x90, 0x90});
            writeBytes(gd::base + 0x17E8DD, {0x6A, 0x00, 0x90, 0x90, 0x90});
            writeBytes(gd::base + 0x17EBB9, {0x6A, 0x00, 0x90, 0x90, 0x90});
            writeBytes(gd::base + 0x17DF73, {0x6A, 0x00, 0x90, 0x90, 0x90});
            writeBytes(gd::base + 0x6F9BE, {0x6A, 0x00, 0x90, 0x90, 0x90});
            writeBytes(gd::base + 0x6FC49, {0x6A, 0x00, 0x90, 0x90, 0x90});
        }
        else
        {
            writeBytes(gd::base + 0x17DEB0, {0x68, 0xFF, 0x00, 0x00, 0x00});
            writeBytes(gd::base + 0x17E3BB, {0x68, 0xFF, 0x00, 0x00, 0x00});
            writeBytes(gd::base + 0x17E8DD, {0x68, 0xFF, 0x00, 0x00, 0x00});
            writeBytes(gd::base + 0x17EBB9, {0x68, 0xFF, 0x00, 0x00, 0x00});
            writeBytes(gd::base + 0x17DF73, {0x68, 0xFF, 0x00, 0x00, 0x00});
            writeBytes(gd::base + 0x6F9BE, {0x68, 0xFF, 0x00, 0x00, 0x00});
            writeBytes(gd::base + 0x6FC49, {0x68, 0xFF, 0x00, 0x00, 0x00});
        }
    }

    static void coinsonpractice(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x204F10, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
        else
            writeBytes(gd::base + 0x204F10, {0x80, 0xBE, 0x95, 0x04, 0x00, 0x00, 0x00, 0x0F, 0x85, 0xDE, 0x00, 0x00, 0x00});
    }

    static void practicemusic(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0x20C925, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
            writeBytes(gd::base + 0x20D143, {0x90, 0x90});
            writeBytes(gd::base + 0x20A563, {0x90, 0x90});
            writeBytes(gd::base + 0x20A595, {0x90, 0x90});
        }
        else
        {
            writeBytes(gd::base + 0x20C925, {0x0F, 0x85, 0xF7, 0x00, 0x00, 0x00});
            writeBytes(gd::base + 0x20D143, {0x75, 0x41});
            writeBytes(gd::base + 0x20A563, {0x75, 0x3E});
            writeBytes(gd::base + 0x20A595, {0x75, 0x0C});
        }
    }

    static void safemode(bool active)
    {
        if(!active && ReplayPlayer::getInstance().IsPlaying())
        {
            hacks.safemode = true;
            return;
        }

        if (active)
        {
            writeBytes(gd::base + 0x20A3B2, {0xE9, 0x9A, 0x01, 0x00, 0x00, 0x90, 0x90});
            writeBytes(gd::base + 0x1FD40F, {0xE9, 0x13, 0x06, 0x00, 0x00, 0x90, 0x90});
        }
        else
        {
            writeBytes(gd::base + 0x20A3B2, {0x80, 0xBB, 0x94, 0x04, 0x00, 0x00, 0x00});
            writeBytes(gd::base + 0x1FD40F, {0x83, 0xB9, 0x64, 0x03, 0x00, 0x00, 0x01});
        }
    }

    static void practicepulse(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0x205536, {0x90, 0x90});
            writeBytes(gd::base + 0x20555D, {0x90, 0x90});
            writeBytes(gd::base + 0x20553E, {0xEB});
        }
        else
        {
            writeBytes(gd::base + 0x205536, {0x75, 0x08});
            writeBytes(gd::base + 0x20555D, {0x75, 0x03});
            writeBytes(gd::base + 0x20553E, {0x74});
        }
    }

    static void noprogressbar(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0x1FCE89, {0x0F, 0x57, 0xC0, 0x90, 0x90, 0x90});
            writeBytes(gd::base + 0x1FCF38, {0x0D});
            writeBytes(gd::base + 0x1FCF6B, {0x3F});
        }
        else
        {
            writeBytes(gd::base + 0x1FCE89, {0xF3, 0x0F, 0x10, 0x44, 0x24, 0x48});
            writeBytes(gd::base + 0x1FCF38, {0x05});
            writeBytes(gd::base + 0x1FCF6B, {0x00});
        }
    }

    //---------------CREATOR--------------------

    static void copyhack(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0x179B8E, {0x90, 0x90});
            writeBytes(gd::base + 0x176F5C, {0x8B, 0xCA, 0x90});
            writeBytes(gd::base + 0x176FE5, {0xB0, 0x01, 0x90});
        }
        else
        {
            writeBytes(gd::base + 0x179B8E, {0x75, 0x0E});
            writeBytes(gd::base + 0x176F5C, {0x0F, 0x44, 0xCA});
            writeBytes(gd::base + 0x176FE5, {0x0F, 0x95, 0xC0});
        }
    }

    static void objectHack(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0x7A100, {0xEB});
            writeBytes(gd::base + 0x7A022, {0xEB});
            writeBytes(gd::base + 0x7A203, {0x90, 0x90});
        }
        else
        {
            writeBytes(gd::base + 0x7A100, {0x72});
            writeBytes(gd::base + 0x7A022, {0x76});
            writeBytes(gd::base + 0x7A203, {0x77, 0x3A});
        }
    }

    static void leveledit(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x1E4A32, {0x90, 0x90});
        else
            writeBytes(gd::base + 0x1E4A32, {0x75, 0x6C});
    }

    static void testmodebypass(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x1FD270, {0xE9, 0xB7, 0x00, 0x00, 0x00, 0x90});
        else
            writeBytes(gd::base + 0x1FD270, {0x0F, 0x84, 0xB6, 0x00, 0x00, 0x00});
    }

    static void hideui(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x8720A, {0xB3, 0x00, 0x90});
        else
            writeBytes(gd::base + 0x8720A, {0x0F, 0x44, 0xD9});
    }

    static void nocmark(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0xA6B8B, {0x2B, 0x87, 0xCC, 0x02, 0x00, 0x00});
            writeBytes(gd::base + 0x70E87, {0xEB, 0x26});
        }
        else
        {
            writeBytes(gd::base + 0xA6B8B, {0x2B, 0x87, 0xD0, 0x02, 0x00, 0x00});
            writeBytes(gd::base + 0x70E87, {0x74, 0x26});
        }
    }

    static void objectlimit(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0x73169, {0xFF, 0xFF, 0xFF, 0x7F});
            writeBytes(gd::base + 0x856A4, {0xFF, 0xFF, 0xFF, 0x7F});
            writeBytes(gd::base + 0x87B17, {0xFF, 0xFF, 0xFF, 0x7F});
            writeBytes(gd::base + 0x87BC7, {0xFF, 0xFF, 0xFF, 0x7F});
            writeBytes(gd::base + 0x87D95, {0xFF, 0xFF, 0xFF, 0x7F});
            writeBytes(gd::base + 0x880F4, {0xFF, 0xFF, 0xFF, 0x7F});
            writeBytes(gd::base + 0x160B06, {0xFF, 0xFF, 0xFF, 0x7F});
            writeBytes(gd::base + 0x7A100, {0xEB});
            writeBytes(gd::base + 0x7A022, {0xEB});
            writeBytes(gd::base + 0x7A203, {0x90, 0x90});
        }
        else
        {
            writeBytes(gd::base + 0x73169, {0x80, 0x38, 0x01, 0x00});
            writeBytes(gd::base + 0x856A4, {0x80, 0x38, 0x01, 0x00});
            writeBytes(gd::base + 0x87B17, {0x80, 0x38, 0x01, 0x00});
            writeBytes(gd::base + 0x87BC7, {0x80, 0x38, 0x01, 0x00});
            writeBytes(gd::base + 0x87D95, {0x80, 0x38, 0x01, 0x00});
            writeBytes(gd::base + 0x880F4, {0x80, 0x38, 0x01, 0x00});
            writeBytes(gd::base + 0x160B06, {0x80, 0x38, 0x01, 0x00});
            writeBytes(gd::base + 0x7A100, {0x72});
            writeBytes(gd::base + 0x7A022, {0x76});
            writeBytes(gd::base + 0x7A203, {0x77, 0x3A});
        }
    }

    static void zoomhack(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0x87801, {0x90, 0x90, 0x90});
            writeBytes(gd::base + 0x87806, {0x90, 0x90, 0x90});
            writeBytes(gd::base + 0x87871, {0x90, 0x90, 0x90});
            writeBytes(gd::base + 0x87876, {0x90, 0x90, 0x90});
        }
        else
        {
            writeBytes(gd::base + 0x87801, {0x0F, 0x2F, 0xC8});
            writeBytes(gd::base + 0x87806, {0x0F, 0x28, 0xC8});
            writeBytes(gd::base + 0x87871, {0x0F, 0x2F, 0xC8});
            writeBytes(gd::base + 0x87876, {0x0F, 0x28, 0xC8});
        }
    }

    static void defaultsong(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0x174407, {0x90, 0x90});
            writeBytes(gd::base + 0x174411, {0x90, 0x90, 0x90});
            writeBytes(gd::base + 0x174456, {0x90, 0x90});
            writeBytes(gd::base + 0x174460, {0x90, 0x90, 0x90});
        }
        else
        {
            writeBytes(gd::base + 0x174407, {0x74, 0x2F});
            writeBytes(gd::base + 0x174411, {0x0F, 0x4F, 0xC6});
            writeBytes(gd::base + 0x174456, {0x74, 0x2F});
            writeBytes(gd::base + 0x174460, {0x0F, 0x4F, 0xC6});
        }
    }

    static void editorextension(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0x2E67A4, {0x00, 0x60, 0xEA, 0x4B});
            writeBytes(gd::base + 0x8FA4D, {0x0F, 0x60, 0xEA, 0x4B});
        }
        else
        {
            writeBytes(gd::base + 0x2E67A4, {0x00, 0x60, 0x6A, 0x48});
            writeBytes(gd::base + 0x8FA4D, {0x80, 0x67, 0x6A, 0x48});
        }
    }

    static void placeover(bool active)
    {
        if (active)
        {
            writeBytes(gd::base + 0x160EE1, {0x8B, 0xC1, 0x90});
            writeBytes(gd::base + 0x160EF2, {0xE9, 0x23, 0x02, 0x00, 0x00, 0x90});
        }
        else
        {
            writeBytes(gd::base + 0x160EE1, {0x0F, 0x48, 0xC1});
            writeBytes(gd::base + 0x160EF2, {0x0F, 0x8F, 0x22, 0x02, 0x00, 0x00});
        }
    }

    static void verifyhack(bool active)
    {
        if (active)
            writeBytes(gd::base + 0x71D48, {0xEB});
        else
            writeBytes(gd::base + 0x71D48, {0x74});
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
        // Speedhack::SetSpeed(value);
        SpeedhackAudio::set(value);
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
    }

    static std::string GetSongFolder()
    {
        std::filesystem::path path;
        PWSTR path_tmp;
        auto get_folder_path_ret = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &path_tmp);

        if (get_folder_path_ret != S_OK)
        {
            CoTaskMemFree(path_tmp);
        }
        else
        {
            path = path_tmp;
            path = path.parent_path();
            path = path / "Local/GeometryDash";
            CoTaskMemFree(path_tmp);
            return utf16ToUTF8(path.c_str());
        }
        return "";
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
                gd::GameSoundManager::sharedState()->playBackgroundMusic(true, musicPaths[hacks.musicIndex]);
            }
        }
    }
}