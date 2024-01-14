#pragma once
#include <string>
#include <cocos2d.h>
#include "SeedValue.hpp"

namespace gd
{
	enum class GJDifficulty
	{
		kGJDifficultyAuto,
		kGJDifficultyEasy,
		kGJDifficultyNormal,
		kGJDifficultyHard,
		kGJDifficultyHarder,
		kGJDifficultyInsane,
		kGJDifficultyDemon,
		kGJDifficultyDemonEasy,
		kGJDifficultyDemonMedium,
		kGJDifficultyDemonInsane,
		kGJDifficultyDemonExtreme
	};

	enum class GJLevelType
	{
		kGJLevelTypeLocal = 1,
		kGJLevelTypeEditor,
		kGJLevelTypeSaved
	};

	class GJGameLevel : public cocos2d::CCNode
	{
	public:
		cocos2d::CCDictionary* m_lastBuildSave;
		SeedValueRSV m_levelID;
		std::string m_levelName; // 0x118
		std::string m_levelDesc; // 0x130
		std::string m_levelString; // 0x148
		std::string m_creatorName; // 0x160
		std::string m_recordString; // 0x178
		std::string m_uploadDate; // 0x190
		std::string m_updateDate; // 0x1A8
		std::string m_unkString1;
		std::string m_unkString2;

		cocos2d::CCPoint m_unkPoint;
		SeedValueRSV m_userID;
		SeedValueRSV m_accountID;
		GJDifficulty m_difficulty;
		int m_audioTrack;
		int m_songID;
		int m_levelRev;
		bool m_unlisted;
		bool m_friendsOnly;
		SeedValueRSV m_objectCount;
		int m_levelIndex;
		int m_ratings;
		int m_ratingsSum;
		int m_downloads;
		bool m_isEditable;
		bool m_gauntletLevel;
		bool m_gauntletLevel2;
		int m_workingTime;
		int m_workingTime2;
		bool m_lowDetailMode;
		bool m_lowDetailModeToggled;
		bool m_selected;
		bool m_localOrSaved;
		bool m_disableShake;
		SeedValueRS m_isVerified;
		bool m_isVerifiedRaw;
		bool m_isUploaded;
		bool m_hasBeenModified;
		int m_levelVersion;
		int m_gameVersion;

		SeedValueRSV m_attempts;
		SeedValueRSV m_jumps;
		SeedValueRSV m_clicks;
		SeedValueRSV m_attemptTime;
		int m_chk;
		bool m_isChkValid;
		bool m_isCompletionLegitimate;
		SeedValueVSR m_normalPercent;
		SeedValueRSV m_orbCompletion;
		SeedValueRSV m_newNormalPercent2;
		int m_practicePercent; // 0x2C4
		int m_likes; // 0x2C8
		int m_dislikes; // 0x2CC
		int m_levelLength; // 0x2D0
		int m_featured; // 0x2D4
		int m_isEpic; // 0x2D8
		bool m_levelFavorited; // 0x2D9
		int m_levelFolder; // 0x2DD
		SeedValueRSV m_dailyID; // 0x2E9
		SeedValueRSV m_demon; // 0x2F5
		int m_demonDifficulty; // 0x2F9
		SeedValueRSV m_stars; // 0x305
		bool m_autoLevel; // 0x306
		int m_coins; // 0x30A
		SeedValueRSV m_coinsVerified; // 0x316
		SeedValueRS m_password; // 0x31E
		SeedValueRSV m_originalLevel; // 0x32A
		bool m_twoPlayerMode; // 0x32B
		int m_failedPasswordAttempts; // 0x32F
		SeedValueRSV m_firstCoinVerified; // 0x33B
		SeedValueRSV m_secondCoinVerified; // 0x347
		SeedValueRSV m_thirdCoinVerified; // 0x353
		int m_starsRequested; // 0x357
		bool m_showedSongWarning; // 0x358
		int m_starRatings; // 0x35C
		int m_starRatingsSum; // 0x360
		int m_maxStarRatings; // 0x364
		int m_minStarRatings; // 0x368
		int m_demonVotes; // 0x36C
		int m_rateStars; // 0x370
		int m_rateFeature; // 0x374
		std::string m_rateUser;
		bool m_dontSave;
		bool m_levelNotDownloaded;
		int m_requiredCoins;
		bool m_isUnlocked;
		cocos2d::CCPoint m_lastCameraPos;
		float m_fastEditorZoom;
		int m_lastBuildTab;
		int m_lastBuildPage;
		int m_lastBuildGroupID;
		GJLevelType m_levelType;
		int m_M_ID;
		std::string m_tempName;
		std::string m_capacityString;
		bool m_highObjectsEnabled;
		bool m_unlimitedObjectsEnabled;
		std::string m_personalBests;
		int m_timestamp;
		int m_unkInt;
		std::string m_songIDs;
		std::string m_sfxIDs;
		int m_54;
		int m_bestTime;
		int m_bestPoints;
		int m_k111;
		std::string m_unkString3;
		std::string m_unkString4;
	};
}
