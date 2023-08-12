#include "Practice.h"
#include "Hacks.h"
#include "PlayLayer.h"
#include "ReplayPlayer.h"
#include "imgui.h"
#include "TrajectorySimulation.h"

void CustomCheckpoint::GetCheckpoint()
{
	c = ReplayPlayer::getInstance().GetPractice().CreateCheckpoint();
}

CCRotateBy* runNormalRotation(gd::PlayerObject* player, float rate)
{
	cocos2d::CCRotateBy* rotateBy;
	cocos2d::CCRotateBy* action;

	if (!player->m_isShip && !player->m_isBird && !player->m_isDart && !player->m_isSpider && !player->m_isRobot &&
		!player->m_isDashing)
	{
		if (*((float*)player + 385) == 1.0)
			rotateBy = (cocos2d::CCRotateBy*)1054727646;
		else
			rotateBy = (cocos2d::CCRotateBy*)1051372203;

		int flipMod = rate >= 0 ? 1 : -1;

		action = rotateBy->create(0.421, (float)(180 * flipMod));

		action->setTag(0);
	}
	return action;
}

void runBallRotation2(gd::PlayerObject* pl, CheckpointData c)
{
	if (!pl->m_isDashing)
	{
		pl->stopActionByTag(1);

		float v2 = 1.0f;
		float v3 = 0;

		if (pl->m_vehicleSize == 1.0f)
			v3 = 1.0f;
		else
			v3 = 0.8f;

		if (pl->m_playerSpeed == 0.7f)
			v2 = 1.2405638f;
		else if (pl->m_playerSpeed == 1.1f)
		{
			v2 = 0.80424345f;
		}
		else if (pl->m_playerSpeed == 1.3f)
		{
			v2 = 0.66576928f;
		}
		else if (pl->m_playerSpeed == 1.6f)
		{
			v2 = 0.54093748f;
		}

		float v5 = v3 * 0.8f * v2;

		auto rotate = CCRotateBy::create(v5, (float)(-340 * (2 * !pl->m_isUpsideDown - 1)));
		auto easeout = CCEaseOut::create(rotate, 1.2f);
		easeout->setTag(1);

		pl->setRotation(c.rotationElapsed);
		pl->runAction(easeout);
		easeout->step(0);//rotrate 0
		easeout->step(c.ballRotationElapsed);
	}
}

void runBallRotation(gd::PlayerObject* pl, CheckpointData c)
{
	if (!pl->m_isDashing)
	{
		pl->stopActionByTag(0);

		float v4 = 1.0f;
		float v5 = 0.f;

		if (pl->m_vehicleSize == 1.0f)
			v5 = 1.0f;
		else
			v5 = 0.8f;

		if (pl->m_playerSpeed == 0.7f)
			v4 = 1.2405638f;
		else if (pl->m_playerSpeed == 1.1f)
		{
			v4 = 0.80424345f;
		}
		else if (pl->m_playerSpeed == 1.3f)
		{
			v4= 0.66576928f;
		}
		else if (pl->m_playerSpeed == 1.6f)
		{
			v4 = 0.54093748f;
		}

		auto rotate = CCRotateBy::create((v5 * 0.2f) * v4, (float)(120 * (2 * !pl->m_isUpsideDown - 1)));
		auto repeatForever = CCRepeatForever::create(rotate);
		repeatForever->setTag(0);

		pl->setRotation(c.rotationElapsed);
		pl->runAction(repeatForever);
		repeatForever->step(0);//rotrate 0
		repeatForever->step(c.ballRotationElapsed);
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

ImGuiKey ConvertKeyEnum(int key)
{
	switch (key)
	{
	case KEY_None:
		return ImGuiKey_None;
	case KEY_Backspace:
		return ImGuiKey_Backspace;
	case KEY_Tab:
		return ImGuiKey_Tab;
	case KEY_Clear:
		return ImGuiKey_None; // No equivalent key
	case KEY_Enter:
		return ImGuiKey_Enter;
	case KEY_Shift:
		return ImGuiKey_None; // No equivalent key
	case KEY_Control:
		return ImGuiKey_None; // No equivalent key
	case KEY_Alt:
		return ImGuiKey_None; // No equivalent key
	case KEY_Pause:
		return ImGuiKey_Pause;
	case KEY_CapsLock:
		return ImGuiKey_CapsLock;
	case KEY_Escape:
		return ImGuiKey_Escape;
	case KEY_Space:
		return ImGuiKey_Space;
	case KEY_PageUp:
		return ImGuiKey_PageUp;
	case KEY_PageDown:
		return ImGuiKey_PageDown;
	case KEY_End:
		return ImGuiKey_End;
	case KEY_Home:
		return ImGuiKey_Home;
	case KEY_Left:
		return ImGuiKey_LeftArrow;
	case KEY_Up:
		return ImGuiKey_UpArrow;
	case KEY_Right:
		return ImGuiKey_RightArrow;
	case KEY_Down:
		return ImGuiKey_DownArrow;
	case KEY_Select:
		return ImGuiKey_None; // No equivalent key
	case KEY_Print:
		return ImGuiKey_PrintScreen;
	case KEY_Execute:
		return ImGuiKey_None; // No equivalent key
	case KEY_PrintScreen:
		return ImGuiKey_PrintScreen;
	case KEY_Insert:
		return ImGuiKey_Insert;
	case KEY_Delete:
		return ImGuiKey_Delete;
	case KEY_Help:
		return ImGuiKey_None; // No equivalent key
	case KEY_Zero:
		return ImGuiKey_0;
	case KEY_One:
		return ImGuiKey_1;
	case KEY_Two:
		return ImGuiKey_2;
	case KEY_Three:
		return ImGuiKey_3;
	case KEY_Four:
		return ImGuiKey_4;
	case KEY_Five:
		return ImGuiKey_5;
	case KEY_Six:
		return ImGuiKey_6;
	case KEY_Seven:
		return ImGuiKey_7;
	case KEY_Eight:
		return ImGuiKey_8;
	case KEY_Nine:
		return ImGuiKey_9;
	case KEY_A:
		return ImGuiKey_A;
	case KEY_B:
		return ImGuiKey_B;
	case KEY_C:
		return ImGuiKey_C;
	case KEY_D:
		return ImGuiKey_D;
	case KEY_E:
		return ImGuiKey_E;
	case KEY_F:
		return ImGuiKey_F;
	case KEY_G:
		return ImGuiKey_G;
	case KEY_H:
		return ImGuiKey_H;
	case KEY_I:
		return ImGuiKey_I;
	case KEY_J:
		return ImGuiKey_J;
	case KEY_K:
		return ImGuiKey_K;
	case KEY_L:
		return ImGuiKey_L;
	case KEY_M:
		return ImGuiKey_M;
	case KEY_N:
		return ImGuiKey_N;
	case KEY_O:
		return ImGuiKey_O;
	case KEY_P:
		return ImGuiKey_P;
	case KEY_Q:
		return ImGuiKey_Q;
	case KEY_R:
		return ImGuiKey_R;
	case KEY_S:
		return ImGuiKey_S;
	case KEY_T:
		return ImGuiKey_T;
	case KEY_U:
		return ImGuiKey_U;
	case KEY_V:
		return ImGuiKey_V;
	case KEY_W:
		return ImGuiKey_W;
	case KEY_X:
		return ImGuiKey_X;
	case KEY_Y:
		return ImGuiKey_Y;
	case KEY_Z:
		return ImGuiKey_Z;
	case KEY_LeftWindowsKey:
		return ImGuiKey_None; // No equivalent key
	case KEY_RightWindowsKey:
		return ImGuiKey_None; // No equivalent key
	case KEY_ApplicationsKey:
		return ImGuiKey_None; // No equivalent key
	case KEY_NumPad0:
		return ImGuiKey_Keypad0;
	case KEY_NumPad1:
		return ImGuiKey_Keypad1;
	case KEY_NumPad2:
		return ImGuiKey_Keypad2;
	case KEY_NumPad3:
		return ImGuiKey_Keypad3;
	case KEY_NumPad4:
		return ImGuiKey_Keypad4;
	case KEY_NumPad5:
		return ImGuiKey_Keypad5;
	case KEY_NumPad6:
		return ImGuiKey_Keypad6;
	case KEY_NumPad7:
		return ImGuiKey_Keypad7;
	case KEY_NumPad8:
		return ImGuiKey_Keypad8;
	case KEY_NumPad9:
		return ImGuiKey_Keypad9;
	case KEY_Multiply:
		return ImGuiKey_KeypadMultiply;
	case KEY_Add:
		return ImGuiKey_KeypadAdd;
	case KEY_Seperator:
		return ImGuiKey_KeypadEnter;
	case KEY_Subtract:
		return ImGuiKey_KeypadSubtract;
	case KEY_Decimal:
		return ImGuiKey_KeypadDecimal;
	case KEY_Divide:
		return ImGuiKey_KeypadDivide;
	case KEY_F1:
		return ImGuiKey_F1;
	case KEY_F2:
		return ImGuiKey_F2;
	case KEY_F3:
		return ImGuiKey_F3;
	case KEY_F4:
		return ImGuiKey_F4;
	case KEY_F5:
		return ImGuiKey_F5;
	case KEY_F6:
		return ImGuiKey_F6;
	case KEY_F7:
		return ImGuiKey_F7;
	case KEY_F8:
		return ImGuiKey_F8;
	case KEY_F9:
		return ImGuiKey_F9;
	case KEY_F10:
		return ImGuiKey_F10;
	case KEY_F11:
		return ImGuiKey_F11;
	case KEY_F12:
		return ImGuiKey_F12;
	case KEY_Numlock:
		return ImGuiKey_NumLock;
	case KEY_ScrollLock:
		return ImGuiKey_ScrollLock;
	case KEY_LeftShift:
		return ImGuiKey_LeftShift;
	case KEY_RightShift:
		return ImGuiKey_RightShift;
	case KEY_LeftControl:
		return ImGuiKey_LeftCtrl;
	case KEY_RightContol:
		return ImGuiKey_RightCtrl;
	case KEY_LeftMenu:
		return ImGuiKey_LeftAlt;
	case KEY_RightMenu:
		return ImGuiKey_RightAlt;
	default:
		return ImGuiKey_COUNT; // Invalid key
	}
}

bool isKeyPressed(gd::PlayLayer* pl, bool player1)
{
	const auto& io = ImGui::GetIO();

	if (!pl->m_pLevelSettings->m_twoPlayerMode)
	{
		if (io.MouseDown[0] || io.KeysDown[ImGuiKey_UpArrow] || io.KeysDown[ImGuiKey_Space] ||
			io.KeysDown[ConvertKeyEnum(hacks.customJumpKey)] || io.KeysDown[ConvertKeyEnum(hacks.customJumpKey2)])
			return true;
	}
	else
	{
		if (!pl->m_bIsDualMode)
		{
			if (player1)
			{
				if (io.MouseDown[0] || io.KeysDown[ImGuiKey_Space] || io.KeysDown[ConvertKeyEnum(hacks.customJumpKey)])
					return true;
			}
			else
			{
				if (io.KeysDown[ImGuiKey_UpArrow] || io.KeysDown[ConvertKeyEnum(hacks.customJumpKey2)])
					return true;
			}
		}
		else
		{
			if (player1)
			{
				if (io.KeysDown[ImGuiKey_Space] || io.KeysDown[ConvertKeyEnum(hacks.customJumpKey)])
					return true;
			}
			else
			{
				if (io.MouseDown[0] || io.KeysDown[ImGuiKey_UpArrow] ||
					io.KeysDown[ConvertKeyEnum(hacks.customJumpKey2)])
					return true;
			}
		}
	}

	return false;
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
	cd.mouseDown = isKeyPressed(pl, isp1);
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
	cd.objSnap = p->m_objectSnappedTo;
	cd.ballRotationElapsed = 0;
	if (p->m_isBall)
	{
		cd.rotateTag = 1;
		auto ac = static_cast<cocos2d::CCActionInterval*>(p->getActionByTag(1));
		if (!ac && p->getPositionX() != pl->m_playerStartPosition.x && !TrajectorySimulation::getInstance()->shouldInterrumpHooks())
		{
			ac = static_cast<cocos2d::CCActionInterval*>(static_cast<cocos2d::CCRepeatForever*>(p->getActionByTag(0))->getInnerAction());
			cd.rotateTag = 0;
		}

		if (ac)
		{
			float el = ac->getElapsed();
			cd.ballRotationElapsed = el;
			if (!p->m_isOnGround)
			{
				ac->step(-el);
				cd.rotationElapsed = p->getRotation();
				ac->step(el);
			}
		}
		else
		{
			cd.rotationElapsed = 0;
			cd.ballRotationElapsed = 0;
		}
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

	bool isp1 = p == pl->m_pPlayer1;

	if (mouseDown != isKeyPressed(pl, isp1))
	{
		out = isKeyPressed(pl, isp1) ? 2 : 1; // 2 == press, 1 == release
	}
	else if (touchRing > 0)
	{
		out = 1;
	}

	if (out == 0 && isp1 && PlayLayer::respawnAction > 0)
		out = PlayLayer::respawnAction;
	else if (out == 0 && !isp1 && PlayLayer::respawnAction2 > 0)
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
	p->m_objectSnappedTo = objSnap;

	SetGamemode(p, gamemode);

	if (gamemode == gd::kGamemodeCube && !p->m_isOnGround && !p->m_isDashing)
	{
		p->runAction(runNormalRotation(p, rotRate));
	}
	if (p->m_isBall && !p->m_isOnGround && ballRotationElapsed != 0)
	{
		if(rotateTag == 1)
			runBallRotation2(p, *this);
		else 
			runBallRotation(p, *this);
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