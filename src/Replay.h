#pragma once
#include "pch.h"
#include <filesystem>
#include <fstream>
#include <vector>
#include <deque>

struct Action
{
	bool press, player2;
	uint32_t frame;
	double yAccel;
	float px, py;
};

struct FrameCapture
{
	bool player2;
	uint32_t frame;
	double yAccel;
	float px, py, rot;
};

struct ReplayInfo
{
	std::string name;
	int actionSize, capturesSize;
	float fps;
};

class Replay
{
  protected:
	std::vector<Action> actions;
	std::vector<FrameCapture> frameCaptures;

  public:
	float fps = 360.0f;
	Replay(){};

	uint32_t GetActionsSize()
	{
		return actions.size();
	}
	uint32_t GetFrameCapturesSize()
	{
		return frameCaptures.size();
	}
	std::vector<Action>& getActions()
	{
		return actions;
	}
	std::vector<FrameCapture>& getCaptures()
	{
		return frameCaptures;
	}
	void AddAction(Action a)
	{
		actions.push_back(a);
	}

	void AddFrame(FrameCapture f)
	{
		frameCaptures.push_back(f);
	}

	void ClearActions()
	{
		actions.clear();
		frameCaptures.clear();
	}

	void RemoveActionsAfter(uint32_t frame)
	{
		const auto check = [&](const Action& action) -> bool { return action.frame >= frame; };
		actions.erase(std::remove_if(actions.begin(), actions.end(), check), actions.end());
		const auto check2 = [&](const FrameCapture& fc) -> bool { return fc.frame >= frame; };
		frameCaptures.erase(std::remove_if(frameCaptures.begin(), frameCaptures.end(), check2), frameCaptures.end());
	}

	static ReplayInfo GetInfo(const std::filesystem::path& name);

	void Load(std::string name);

	void Save(std::string name);
};