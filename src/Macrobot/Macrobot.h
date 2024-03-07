#pragma once

#include <cocos2d.h>
#include <gdr.hpp>
#include <vector>
#include <unordered_map>

#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

namespace Macrobot
{
	enum PlayerMode
	{
		DISABLED = -1,
		PLAYBACK = 0,
		RECORDING = 1
	};

	class PlayerCheckpoint
	{
	public:
		double yVel;
		double xVel;
		float xPos;
		float yPos;
		float nodeXPos;
		float nodeYPos;
		float rotation;
		float rotationRate;
		GameObject* lastSnappedTo = nullptr;
		GameObject* lastSnappedTo2 = nullptr;

		bool isOnSlope;
		bool wasOnSlope;

		std::vector<float> randomProperties;

		void apply(PlayerObject* player, bool fullRestore);
		void fromPlayer(PlayerObject* player, bool fullCapture);
	};

	struct CheckpointData
	{
		uint32_t frame;
		PlayerCheckpoint p1;
		PlayerCheckpoint p2;
	};

	struct Correction
	{
		uint32_t frame;
		bool player2;
		PlayerCheckpoint checkpoint;
	};

	struct Action : gdr::Input
	{
		std::optional<Correction> correction;

		Action() = default;

		Action(uint32_t frame, int button, bool player2, bool down) : gdr::Input(frame, button, player2, down)
		{
		}

		void parseExtension(gdr::json::object_t obj) override
		{
			if (obj.contains("correction"))
			{
				Correction c;
				c.frame = obj["correction"]["frame"];
				c.player2 = obj["correction"]["player2"];
				c.checkpoint.xVel = obj["correction"]["xVel"];
				c.checkpoint.yVel = obj["correction"]["yVel"];
				c.checkpoint.xPos = obj["correction"]["xPos"];
				c.checkpoint.yPos = obj["correction"]["yPos"];
				c.checkpoint.nodeXPos = obj["correction"]["nodeXPos"];
				c.checkpoint.nodeYPos = obj["correction"]["nodeYPos"];
				c.checkpoint.rotation = obj["correction"]["rotation"];
				c.checkpoint.rotationRate = obj["correction"]["rotationRate"];
				correction = c;
			}
		}

		gdr::json::object_t saveExtension() const override
		{
			gdr::json::object_t obj = gdr::json::object();

			if (correction.has_value())
			{
				Correction c = correction.value();
				obj["correction"]["frame"] = c.frame;
				obj["correction"]["player2"] = c.player2;
				obj["correction"]["xVel"] = c.checkpoint.xVel;
				obj["correction"]["yVel"] = c.checkpoint.yVel;
				obj["correction"]["xPos"] = c.checkpoint.xPos;
				obj["correction"]["yPos"] = c.checkpoint.yPos;
				obj["correction"]["nodeXPos"] = c.checkpoint.nodeXPos;
				obj["correction"]["nodeYPos"] = c.checkpoint.nodeYPos;
				obj["correction"]["rotation"] = c.checkpoint.rotation;
				obj["correction"]["rotationRate"] = c.checkpoint.rotationRate;
			}

			return obj;
		}
	};

	struct Macro : gdr::Replay<Macro, Action>
	{
		Macro() : gdr::Replay<Macro, Action>("Macrobot", "1.1") {}
	};

	inline bool botInput = false;
	inline bool resetFrame = false;
	inline bool resetFromStart = true;

	inline PlayerMode playerMode = DISABLED;

	inline unsigned int actionIndex = 0;
	inline unsigned int correctionIndex = 0;

	inline int8_t direction = 0;

	inline Macro macro;

	inline std::unordered_map<void*, Macrobot::CheckpointData> checkpoints;
	inline std::vector<std::string> macroList;

	inline std::string macroName;
	inline std::string macroDescription;

	inline std::unordered_map<int, bool> downForKey1;
	inline std::unordered_map<int, bool> downForKey2;
	inline std::unordered_map<int, float> timeForKey1;
	inline std::unordered_map<int, float> timeForKey2;

	inline FMOD::Channel* clickChannel = nullptr;

	void GJBaseGameLayerProcessCommands(GJBaseGameLayer* self);
	void handleAction(bool down, int button, bool player1, float timestamp);

	Action* recordAction(PlayerButton key, uint32_t frame, bool press, bool player1);

	void save(const std::string& file);
	void load(const std::string& file);

	void getMacros();

	void drawWindow();
};
