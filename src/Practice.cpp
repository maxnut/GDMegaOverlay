#include "Practice.h"
#include "Hacks.h"
#include "PlayLayer.h"
#include "ReplayPlayer.h"
#include "imgui.h"

void CustomCheckpoint::GetCheckpoint()
{
	c = ReplayPlayer::getInstance().GetPractice().CreateCheckpoint();
}

void runNormalRotation(gd::PlayerObject* player, float rate)
{
	cocos2d::CCRotateBy* rotateBy;
	cocos2d::CCAction* action;

	if (!player->m_isShip && !*((bool*)player + 1532) && !*((bool*)player + 1533) && !*((bool*)player + 1537))
	{
		if (*((float*)player + 385) == 1.0)
			rotateBy = (cocos2d::CCRotateBy*)1054727646;
		else
			rotateBy = (cocos2d::CCRotateBy*)1051372203;

		int flipMod = rate > 0 ? 1 : -1;

		action = rotateBy->create(0.421, (float)(180 * flipMod));

		action->setTag(0);
		player->runAction(action);
	}
}

Checkpoint Practice::CreateCheckpoint()
{
	gd::PlayLayer* pl = gd::GameManager::sharedState()->getPlayLayer();
	Checkpoint c;
	if (pl)
	{
		c.dt = CCDirector::sharedDirector()->getDeltaTime();
		ReplayPlayer& rp = ReplayPlayer::getInstance();
		c.p1 = CheckpointData::fromPlayer(pl->m_pPlayer1);
		c.p2 = CheckpointData::fromPlayer(pl->m_pPlayer2);
		for (size_t i = 0; i < pl->m_screenRingObjects->count(); i++)
		{
			auto ring = dynamic_cast<cocos2d::CCNode*>(pl->m_screenRingObjects->objectAtIndex(i));
			c.ringRotations.push_back(ring->getRotation());
		}
		c.frameOffset = rp.GetFrame();
		c.activatedObjectsSize = activatedObjects.size();
		c.activatedObjectsP2Size = activatedObjectsP2.size();
	}
	return c;
}

CustomCheckpoint* CustomCheckpoint::createHook()
{
	CustomCheckpoint* cc = new CustomCheckpoint;
	if (cc && cc->init())
	{
		cc->autorelease();
		cc->GetCheckpoint();
	}
	else
	{
		CC_SAFE_DELETE(cc);
	}
	if (PlayLayer::hadAction)
	{
		PlayLayer::respawnAction = cc->c.p1.isHolding + 1;
		if (gd::GameManager::sharedState()->getPlayLayer()->m_pLevelSettings->m_twoPlayerMode)
			PlayLayer::respawnAction2 = cc->c.p2.isHolding + 1;
	}
	else
		PlayLayer::respawnAction = 0;
	return cc;
}

CheckpointData CheckpointData::fromPlayer(gd::PlayerObject* p)
{
	gd::PlayLayer* pl = gd::GameManager::sharedState()->getPlayLayer();
	CheckpointData cd;
	bool isp1 = p == pl->m_pPlayer1;
	cd.xAccel = p->m_xAccel;
	cd.yAccel = p->m_yAccel;
	cd.jumpAccel = p->m_jumpAccel;
	cd.xPos = p->m_position.x;
	cd.yPos = p->m_position.y;
	cd.rotation = p->getRotation();
	cd.rotRate = isp1 ? PlayLayer::player1RotRate : PlayLayer::player2RotRate;
	cd.unkRot = p->m_fUnkRotationField;
	cd.playerSpeed = p->m_playerSpeed;
	cd.vehichleSize = p->m_vehicleSize;
	cd.decelerationRate = p->m_decelerationRate;
	cd.isHolding = p->m_isHolding;
	cd.lastJumpTime = p->m_lastJumpTime;
	cd.mouseDown = ImGui::GetIO().MouseDown[0] || ImGui::GetIO().KeysDown[ImGuiKey_UpArrow] ||
				   ImGui::GetIO().KeysDown[ImGuiKey_Space];
	cd.isHolding2 = p->m_isHolding2;
	cd.canRobotJump = p->m_canRobotJump;
	cd.isUpsideDown = p->m_isUpsideDown;
	cd.isOnGround = p->m_isOnGround;
	cd.isDashing = p->m_isDashing;
	cd.isSliding = p->m_isSliding;
	cd.isRising = p->m_isRising;
	cd.isLocked = p->m_isLocked;
	cd.isOnSlope = p->m_isOnSlope;
	cd.wasOnSlope = p->m_wasOnSlope;
	cd.isDropping = p->m_isDropping;
	cd.touchRing = p->m_touchingRings->count();
	cd.gamemode = GetGamemode(p);
	auto ac = static_cast<cocos2d::CCRotateBy*>(p->getActionByTag(1));
	if (ac && p->m_isBall && !p->m_isOnGround)
	{
		ac->setTag(isp1 ? 5000 : 5001);
		auto r = static_cast<CCRotateBy*>(ac);
		cd.ballRotationElapsed = ac->getElapsed();
	}
	ac = static_cast<cocos2d::CCRotateBy*>(p->getActionByTag(isp1 ? 5000 : 5001));
	if (ac && p->m_isBall && !p->m_isOnGround)
	{
		cd.ballRotationElapsed = ac->getElapsed();
	}
	return cd;
}

int CheckpointData::Apply(gd::PlayerObject* p, bool tp)
{
	auto pl = gd::GameManager::sharedState()->getPlayLayer();
	int out = 0;
	p->m_xAccel = xAccel;
	p->m_yAccel = yAccel;
	p->m_jumpAccel = jumpAccel;

	if (mouseDown != (ImGui::GetIO().MouseDown[0] || ImGui::GetIO().KeysDown[ImGuiKey_UpArrow] ||
					  ImGui::GetIO().KeysDown[ImGuiKey_Space]))
	{
		out = ImGui::GetIO().MouseDown[0] || ImGui::GetIO().KeysDown[ImGuiKey_UpArrow] ||
					  ImGui::GetIO().KeysDown[ImGuiKey_Space]
				  ? 2
				  : 1; // 2 == press, 1 == release
	}
	else if (touchRing > 0)
	{
		out = 1;
	}

	bool isp1 = p == pl->m_pPlayer1;

	if (tp && isHolding2 == p->m_isHolding2)
		out = p->m_isHolding2 ? 2 : 1;

	if (out == 0 && isp1 && PlayLayer::respawnAction > 0)
		out = PlayLayer::respawnAction;
	if (out == 0 && !isp1 && PlayLayer::respawnAction2 > 0)
		out = PlayLayer::respawnAction2;

	p->setPosition({xPos, yPos});
	p->m_position.x = xPos;
	p->m_position.y = yPos;
	p->setRotation(rotation);

	p->m_fUnkRotationField = unkRot;
	p->m_playerSpeed = playerSpeed;
	p->m_vehicleSize = vehichleSize;
	p->m_decelerationRate = decelerationRate;
	p->m_hasJustHeld = hasJustHeld;
	p->m_hasJustHeld2 = hasJustHeld2;
	p->m_isHolding = isHolding;
	p->m_isHolding2 = isHolding2;
	p->m_canRobotJump = canRobotJump;
	p->m_isUpsideDown = isUpsideDown;
	p->m_lastJumpTime = lastJumpTime;
	p->m_isOnGround = isOnGround;
	p->m_isDashing = isDashing;
	p->m_isSliding = isSliding;
	p->m_isRising = isRising;
	p->m_isLocked = isLocked;
	p->m_isOnSlope = isOnSlope;
	p->m_wasOnSlope = wasOnSlope;
	p->m_isDropping = isDropping;

	SetGamemode(p, gamemode);

	if (gamemode == gd::kGamemodeCube && !p->m_isOnGround)
	{
		runNormalRotation(p, rotRate);
	}
	if (gamemode == gd::kGamemodeBall && !p->m_isOnGround)
	{
		auto ac = static_cast<CCRotateBy*>(p->getActionByTag(isp1 ? 5000 : 5001));
		if (ac)
		{
			ac->step(-ac->getElapsed());
			ac->step(ballRotationElapsed);
		}
	}

	return out;
}

Checkpoint Practice::GetLast()
{
	auto playLayer = gd::GameManager::sharedState()->getPlayLayer();
	if (playLayer && playLayer->m_checkpoints->count())
	{
		auto checkpoint_obj = dynamic_cast<CustomCheckpoint*>(playLayer->m_checkpoints->lastObject());
		if (checkpoint_obj)
		{
			return checkpoint_obj->c;
		}
	}
	return {};
}

void Practice::ApplyCheckpoint()
{
	auto playLayer = gd::GameManager::sharedState()->getPlayLayer();
	if (playLayer)
	{
		Checkpoint c = GetLast();
		for (size_t i = 0; i < playLayer->m_screenRingObjects->count(); i++)
		{
			auto ring = static_cast<gd::EffectGameObject*>(playLayer->m_screenRingObjects->objectAtIndex(i));
			if (ring->m_nObjectID == 1330 || ring->m_nObjectID == 1022)
			{
				ring->stopAllActions();
				float rotDifference = c.ringRotations[i] - ring->getRotation();
				ring->setRotation(c.ringRotations[i]);
				ring->runAction(playLayer->m_pPlayer1->createRotateAction(rotDifference < 0 ? -360 : 360, 1));
			}
		}
		auto click1 = c.p1.Apply(playLayer->m_pPlayer1, playLayer->m_pLevelSettings->m_twoPlayerMode);
		if (click1 != 0)
		{
			if (click1 == 2 && c.p1.touchRing <= 0 && !PlayLayer::wasPaused)
			{
				PlayLayer::pushButton(playLayer->m_pPlayer1, 0);
				ReplayPlayer::getInstance().RecordAction(true, playLayer->m_pPlayer1, true);
			}
			else if (click1 == 1 && !PlayLayer::wasPaused)
			{
				PlayLayer::releaseButton(playLayer->m_pPlayer1, 0);
				ReplayPlayer::getInstance().RecordAction(false, playLayer->m_pPlayer1, true);
			}
			else
				PlayLayer::releaseButton(playLayer->m_pPlayer1, 0);
		}

		if (!playLayer->m_bIsDualMode)
			return;

		auto click2 = c.p2.Apply(playLayer->m_pPlayer2, playLayer->m_pLevelSettings->m_twoPlayerMode);
		if (click2 != 0)
		{
			if (click2 == 2 && c.p2.touchRing <= 0 && !PlayLayer::wasPaused)
			{
				PlayLayer::pushButton(playLayer->m_pPlayer2, 0);
				ReplayPlayer::getInstance().RecordAction(true, playLayer->m_pPlayer2, false);
			}
			else if (click2 == 1 && !PlayLayer::wasPaused)
			{
				PlayLayer::releaseButton(playLayer->m_pPlayer2, 0);
				ReplayPlayer::getInstance().RecordAction(false, playLayer->m_pPlayer2, false);
			}
			else
				PlayLayer::releaseButton(playLayer->m_pPlayer2, 0);
		}
	}
}