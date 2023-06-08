#include "Replay.h"
#include "Hacks.h"

void Replay::Load(std::string path)
{
	ClearActions();
	std::ifstream file(path, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, ("Could not find macro"))->show();
		return;
	}

	size_t acSize;
	size_t fcSize;

	file.read((char*)&fps, sizeof(float));

	file.read((char*)&acSize, sizeof(size_t));
	file.read((char*)&fcSize, sizeof(size_t));

	bool frameCapture = fcSize > 0;

	actions.resize(acSize);
	frameCaptures.resize(fcSize);

	file.read((char*)&actions[0], sizeof(Action) * acSize);
	if (frameCapture)
		file.read((char*)&frameCaptures[0], sizeof(FrameCapture) * fcSize);

	file.close();

	if (frameCapture)
	{
		gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
								 ("Macro loaded with " + std::to_string(acSize) + " actions and " +
								  std::to_string(fcSize) + " frame captures."))
			->show();
	}
	else
	{
		gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
								 ("Macro loaded with " + std::to_string(acSize) + " actions."))
			->show();
	}
}

void Replay::Merge(std::string path)
{
	std::ifstream file(path, std::ios::in | std::ios::binary);
	if (!file.is_open())
	{
		gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, ("Could not find macro"))->show();
		return;
	}

	size_t acSize;
	size_t fcSize;

	file.read((char*)&fps, sizeof(float));

	file.read((char*)&acSize, sizeof(size_t));
	file.read((char*)&fcSize, sizeof(size_t));

	bool frameCapture = fcSize > 0;

	std::vector<Action> _actions;
	std::vector<FrameCapture> _captures;

	_actions.resize(acSize);
	_captures.resize(fcSize);

	file.read((char*)&_actions[0], sizeof(Action) * acSize);
	if (frameCapture)
		file.read((char*)&_captures[0], sizeof(FrameCapture) * fcSize);

	file.close();

	for (size_t i = 0; i < actions.size(); i++)
		if (actions[i].px >= _actions[0].px)
		{
			actions.erase(actions.begin() + i, actions.end());
			break;
		}

	for (size_t i = 0; i < frameCaptures.size(); i++)
		if (frameCaptures[i].px >= _captures[0].px)
		{
			frameCaptures.erase(frameCaptures.begin() + 1, frameCaptures.end());
			break;
		}

	for (auto ac : _actions)
		actions.push_back(ac);

	for (auto fc : _captures)
		frameCaptures.push_back(fc);

	if (frameCapture)
	{
		gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
								 ("Macro merged with " + std::to_string(actions.size()) + " actions and " +
								  std::to_string(frameCaptures.size()) + " frame captures."))
			->show();
	}
	else
	{
		gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
								 ("Macro merged with " + std::to_string(actions.size()) + " actions."))
			->show();
	}
}

void Replay::Save(std::string name)
{
	if (GetActionsSize() <= 0)
	{
		gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr, ("Macro must contain at least one action"))->show();
		return;
	}
	if (!std::filesystem::is_directory("GDMenu/macros") || !std::filesystem::exists("GDMenu/macros"))
		std::filesystem::create_directory("GDMenu/macros");

	std::ofstream file("GDMenu/macros/" + name + ".macro", std::ios::out | std::ios::binary);

	size_t acSize = actions.size();
	size_t fcSize = frameCaptures.size();

	bool frameCapture = fcSize > 0;

	file.write((char*)&fps, sizeof(float));

	file.write((char*)&acSize, sizeof(size_t));
	file.write((char*)&fcSize, sizeof(size_t));

	file.write((char*)&actions[0], sizeof(Action) * acSize);
	if (frameCapture)
		file.write((char*)&frameCaptures[0], sizeof(FrameCapture) * fcSize);

	file.close();

	if (frameCapture)
	{
		gd::FLAlertLayer::create(
			nullptr, "Info", "Ok", nullptr,
			("Macro saved with " + std::to_string(acSize) + " actions and " + std::to_string(fcSize) + " captures."))
			->show();
	}
	else
	{
		gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
								 ("Macro saved with " + std::to_string(acSize) + " actions."))
			->show();
	}
}

ReplayInfo Replay::GetInfo(const std::filesystem::path& name)
{
	ReplayInfo ri;
	ri.name = name.stem().string();
	std::ifstream file(name, std::ios::in | std::ios::binary);

	if (!file.is_open())
	{
		return ri;
	}

	file.read((char*)&ri.fps, sizeof(float));

	file.read((char*)&ri.actionSize, sizeof(size_t));
	file.read((char*)&ri.capturesSize, sizeof(size_t));

	return ri;
}