#pragma once
#include <cocos2d.h>
#include <array>
#include <string>


namespace SafeMode
{
	struct Opcode
	{
		const unsigned long address;
		const std::string off;
		const std::string on;

		Opcode(unsigned long address, std::string off, std::string on)
			: address(address), off(std::move(off)), on(std::move(on))
		{};
	};

	void initHooks();

	void updateState();

	inline const std::array<Opcode, 14> opcodes{
		Opcode{ 0x2DDD6A, "0F 85 EA 01 00 00", "0F 84 EA 01 00 00" },
		{ 0x2DDD70, "80 BB 7C 2A 00 00 00", "90 90 90 90 90 90 90" },
		{ 0x2DDD77, "0F 85 DD 01 00 00", "90 90 90 90 90 90" },

		{ 0x2DDF6E, "0F 85 C2 02 00 00", "0F 84 C2 02 00 00" },
		{ 0x2DDC7E, "0F 85 CA 00 00 00", "0F 84 CA 00 00 00" },

		{ 0x2E6986, "0F 85 92 01 00 00", "0F 84 92 01 00 00" },
		{ 0x2E698C, "80 BB C6 29 00 00 00", "90 90 90 90 90 90 90" },
		{ 0x2E6993, "0F 85 85 01 00 00", "90 90 90 90 90 90" },

		{ 0x2E6B32, "75 0D", "74 0D" },

		{ 0x2EACD0, "FF 81 AC 2E 00 00", "90 90 90 90 90 90" },
		{ 0x2EACD6, "FF 81 AC 2E 00 00", "90 90 90 90 90 90" }, // i have no clue what this inc instruction increments

		{ 0x2EA81F, "6A 01", "6A 00" },
		{ 0x2EA83D, "46", "90" },
		{ 0x2EACF7, "46", "90" }
	};

	inline void(__thiscall* endLevelLayerCustomSetup)(cocos2d::CCLayer*);
	void __fastcall endLevelLayerCustomSetupHook(cocos2d::CCLayer*, void*);
}
