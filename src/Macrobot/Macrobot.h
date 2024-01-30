#pragma once

#include <cocos2d.h>
#include <gdr.hpp>
#include <vector>
#include <unordered_map>

#include <Geode/modify/PlayerObject.hpp>

using namespace geode::prelude;

namespace Macrobot
{

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

	std::vector<float> randomProperties;

	void apply(PlayerObject* player, bool fullRestore);
	void fromPlayer(PlayerObject* player, bool fullCapture);
};

struct CheckpointData
{
	double time;
	PlayerCheckpoint p1;
	PlayerCheckpoint p2;
};

struct Correction
{
	double time;
	bool player2;
	PlayerCheckpoint checkpoint;
};

struct Action : gdr::Input
{
	double time = 0.0;
	std::optional<Correction> correction;

	Action() = default;

	Action(double time, int button, bool player2, bool down) : gdr::Input(0, button, player2, down), time(time)
	{
	}

	void parseExtension(gdr::json::object_t obj) override
	{
		time = obj["time"];

		if(obj.contains("correction"))
		{
			Correction c;
			c.time = obj["correction"]["time"];
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

		obj["time"] = time;

		if(correction.has_value())
		{
			Correction c = correction.value();
			obj["correction"]["time"] = c.time;
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
	Macro() : gdr::Replay<Macro, Action>("Macrobot", "1.0") {}
};

inline float framerate = 60.f;
inline int playerMode = -1;

inline double gameTime = 0;
inline unsigned int actionIndex = 0;
inline unsigned int correctionIndex = 0;

inline int8_t direction = 0;

inline PlayerObject* playerObject1 = nullptr;
inline PlayerObject* playerObject2 = nullptr;

inline Macro macro;

inline std::unordered_map<void*, Macrobot::CheckpointData> checkpoints;
inline std::vector<std::string> macroList;

inline std::string macroName;
inline std::string macroDescription;

void GJBaseGameLayerProcessCommands(GJBaseGameLayer* self);

Action* recordAction(PlayerButton key, double frame, bool press, bool player1);

void save(const std::string& file);
void load(const std::string& file);

void getMacros();

void drawWindow();
}; // namespace Macrobot
