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
	file.seekg(0, std::ios::end);
	size_t left = static_cast<size_t>(file.tellg());
	file.seekg(0);
	left -= sizeof(float) * 2;
	for (size_t _ = 0; _ <= left / 22U; ++_)
	{
		Action a;
		file.read((char*)&a.press, sizeof(bool));
		file.read((char*)&a.player2, sizeof(bool));
		file.read((char*)&a.frame, sizeof(uint32_t));
		file.read((char*)&a.yAccel, sizeof(double));
		file.read((char*)&a.px, sizeof(float));
		file.read((char*)&a.py, sizeof(float));
		AddAction(a);
	}
	file.read((char*)&fps, sizeof(float));
	file.close();
	gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
							 ("Macro loaded with " + std::to_string(GetActionsSize()) + " actions."))
		->show();
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
	for (const auto& a : actions)
	{
		file.write((char*)&a.press, sizeof(bool));
		file.write((char*)&a.player2, sizeof(bool));
		file.write((char*)&a.frame, sizeof(uint32_t));
		file.write((char*)&a.yAccel, sizeof(double));
		file.write((char*)&a.px, sizeof(float));
		file.write((char*)&a.py, sizeof(float));
	}
	file.write((char*)&fps, sizeof(float));
	file.close();
	gd::FLAlertLayer::create(nullptr, "Info", "Ok", nullptr,
							 ("Macro saved with " + std::to_string(GetActionsSize()) + " actions."))
		->show();
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

	float fps = -1;

	file.seekg(0, std::ios::end);
	size_t left = static_cast<size_t>(file.tellg());

	left -= 4;
	file.seekg(left);
	file.read((char*)&fps, sizeof(float));
	ri.actionSize = left / 22U;
	ri.fps = fps;
	file.close();
	return ri;
}