#pragma once
#include "pch.h"
#include <filesystem>
#include <fstream>
#include <vector>

struct Action
{
	bool press, player2;
	uint32_t frame;
	double yAccel;
	float px, py;
};

struct ReplayInfo
{
	std::string name;
	int actionSize;
	float fps;
};

class Replay
{
  protected:
	std::vector<Action> actions;

  public:
	float fps = 360.0f;
	Replay(){};

	uint32_t GetActionsSize()
	{
		return actions.size();
	}
	std::vector<Action>& getActions()
	{
		return actions;
	}
	void AddAction(Action a)
	{
		actions.push_back(a);
	}

	void ClearActions()
	{
		actions.clear();
	}

	void RemoveActionsAfter(uint32_t frame)
	{
		const auto check = [&](const Action& action) -> bool { return action.frame >= frame; };
		actions.erase(std::remove_if(actions.begin(), actions.end(), check), actions.end());
	}

	static ReplayInfo GetInfo(const std::filesystem::path& name);

	void Load(std::string name);

	void Save(std::string name);
};