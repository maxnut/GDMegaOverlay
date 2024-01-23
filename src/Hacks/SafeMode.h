#pragma once
#include <cocos2d.h>
#include <array>
#include <vector>
#include <string>

#include <Geode/loader/Hook.hpp>


inline constexpr std::size_t PATCHES_SIZE = 15;

namespace SafeMode
{
	struct Opcode
	{
		const unsigned long address;
		const std::vector<uint8_t> off;
		const std::vector<uint8_t> on;

		Opcode(unsigned long address, std::vector<uint8_t> off, std::vector<uint8_t> on)
			: address(address), off(std::move(off)), on(std::move(on))
		{};
	};

	void updateState();

	// move to JsonHacks
	inline const std::array<Opcode, PATCHES_SIZE> opcodes{
		// PlayLayer::levelComplete
		Opcode{ 0x2DDC7E, { 0x0F, 0x85, 0xCA, 0x00, 0x00, 0x00 }, { 0x0F, 0x84, 0xCA, 0x00, 0x00, 0x00 } },
		{ 0x2DDD6A, { 0x0F, 0x85, 0xEA, 0x01, 0x00, 0x00 },       { 0x0F, 0x84, 0xEA, 0x01, 0x00, 0x00 } },
		{ 0x2DDD70, { 0x80, 0xBB, 0x7C, 0x2A, 0x00, 0x00, 0x00 }, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 } },
		{ 0x2DDD77, { 0x0F, 0x85, 0xDD, 0x01, 0x00, 0x00 },       { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 } },
		{ 0x2DDEE5, { 0x40, }, { 0x90 } },
		{ 0x2DDF6E, { 0x0F, 0x85, 0xC2, 0x02, 0x00, 0x00 },       { 0x0F, 0x84, 0xC2, 0x02, 0x00, 0x00 } },

		// PlayLayer::destroyPlayer
		{ 0x2E6BDE, { 0x0F, 0x85, 0xAD, 0x00, 0x00, 0x00 }, { 0x90, 0xE9, 0xAD, 0x00, 0x00, 0x00 } },
		{ 0x2E6B32, { 0x75, 0x0D },                         { 0xEB, 0x0D } },
		{ 0x2E69F4, { 0x0F, 0x4F, 0xC1 },                   { 0x0F, 0x4C, 0xC1 } },
		{ 0x2E6993, { 0x0F, 0x85, 0x85, 0x01, 0x00, 0x00 }, { 0x90, 0xE9, 0x85, 0x01, 0x00, 0x00 } },

		// PlayLayer::incrementJumps
		{ 0x2EACD0, { 0xFF, 0x81, 0xAC, 0x2E, 0x00, 0x00 }, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 } },
		{ 0x2EACD6, { 0xFF, 0x81, 0xAC, 0x2E, 0x00, 0x00 }, { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 } }, // i have no clue what this inc instruction increments
		{ 0x2EACF7, { 0x46 }, { 0x90 } },

		// PlayLayer::resetLevel
		{ 0x2EA81F, { 0x6A, 0x01 }, { 0x6A, 0x00 } },
		{ 0x2EA83D, { 0x46 }, { 0x90 } }
	};
	inline std::array<geode::Patch*, PATCHES_SIZE> patches;

	void endLevelLayerCustomSetupHook(cocos2d::CCLayer*);
}
