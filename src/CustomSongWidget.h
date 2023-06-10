#pragma once
#include "pch.h"
#include <imgui.h>
#include <regex>


namespace CustomSongWidget
{
	inline bool(__thiscall* init)(gd::CustomSongWidget* self, gd::SongInfoObject* songInfo, gd::LevelSettingsObject* levelSettings, bool p2, bool p3, bool p4, bool hasDefaultSong, bool hideBackground);
	bool __fastcall initHook(gd::CustomSongWidget* self, void*, gd::SongInfoObject* songInfo, gd::LevelSettingsObject* levelSettings, bool p2, bool p3, bool p4, bool hasDefaultSong, bool hideBackground);

	inline void(__thiscall* onPlaySongButton)(gd::CustomSongWidget* self);
	void __fastcall onPlaySongButtonHook(gd::CustomSongWidget* self);

	class ButtonsClass final : public gd::FLAlertLayer, public cocos2d::CCTextFieldDelegate, public gd::FLAlertLayerProtocol
	{
	public:
		void onCopySongCallback(CCObject* sender)
		{
			std::string songInfo = dynamic_cast<CCLabelProtocol*>(this->getChildren()->objectAtIndex(4))->getString();
			std::smatch matches;

			std::regex_search(songInfo, matches, std::regex(R"(\w+: (\d+) .*)"));

			ImGui::SetClipboardText(matches[1].str().c_str());
		}
	};
}