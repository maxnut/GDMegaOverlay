#include "TrajectorySimulation.h"
#include "Hacks.h"
#include "HitboxNode.hpp"
#include "PlayLayer.h"
#include "Practice.h"

void TrajectorySimulation::createPlayersForSimulation()
{
	auto playLayer = gd::GameManager::sharedState()->getPlayLayer();
	if (!playLayer)
		return;

	m_pPlayer1ForSimulation = gd::PlayerObject::create(1, 1, 0);
	m_pPlayer1ForSimulation->setPosition({0, 105});
	m_pPlayer1ForSimulation->setVisible(false);
	reinterpret_cast<void(__thiscall*)(gd::PlayerObject*)>(gd::base + 0x1f78e0)(m_pPlayer1ForSimulation);
	Hacks::WriteRef<bool>((int)m_pPlayer1ForSimulation + 0x2FC, 1);
	playLayer->m_batchNodePlayer->addChild(m_pPlayer1ForSimulation);

	m_pPlayer2ForSimulation = gd::PlayerObject::create(1, 1, 0);
	m_pPlayer2ForSimulation->setPosition({0, 105});
	m_pPlayer2ForSimulation->setVisible(false);
	reinterpret_cast<void(__thiscall*)(gd::PlayerObject*)>(gd::base + 0x1f78e0)(m_pPlayer2ForSimulation);
	Hacks::WriteRef<bool>((int)m_pPlayer2ForSimulation + 0x2FC, 1);
	playLayer->m_batchNodePlayer->addChild(m_pPlayer2ForSimulation);
}

void TrajectorySimulation::createSimulation()
{
	this->createPlayersForSimulation();
}

void playerupdateSpecial(gd::PlayerObject* player, float dt)
{
	__asm movss xmm1, dt;
	reinterpret_cast<void(__thiscall*)(gd::PlayerObject*)>(gd::base + 0x1e8ab0)(player);
}

void playerupdate(gd::PlayerObject* player, float dt)
{
	reinterpret_cast<void(__thiscall*)(gd::PlayerObject*, float dt)>(gd::base + 0x1e8200)(player, dt);
}

void playerupdateshiprotation(gd::PlayerObject* player, float dt)
{
	__asm movss xmm1, dt;
	reinterpret_cast<void(__thiscall*)(gd::PlayerObject*)>(gd::base + 0x1eba60)(player);
}

void playerupdateRotation(gd::PlayerObject* player, float dt)
{
	__asm movss xmm1, dt;
	reinterpret_cast<void(__thiscall*)(gd::PlayerObject*)>(gd::base + 0x1ebc00)(player);
}

bool playercheckCollisions(gd::PlayLayer* self, gd::PlayerObject* player, float dt)
{
	__asm movss xmm2, dt;
	return reinterpret_cast<bool(__thiscall*)(gd::PlayLayer*, gd::PlayerObject*)>(gd::base + 0x203CD0)(self, player);
}

void TrajectorySimulation::simulationPerPlayer(gd::PlayerObject* player, gd::PlayerObject* playerBase, float dt)
{
	if (playerBase->m_position.x <= 0.1f)
		return;
	auto playLayer = gd::GameManager::sharedState()->getPlayLayer();
	auto winSize = cocos2d::CCDirector::sharedDirector()->getWinSize();
	auto res = CheckpointData::fromPlayer(playerBase);

	if(playLayer->m_isDead) return;

	bool isp1 = playLayer->m_pPlayer1 == player;

	res.Apply(player, false);
	PlayLayer::pushButton(player, 0);

	auto pl = gd::GameManager::sharedState()->getPlayLayer();

	PlayLayer::togglePlayerScaleHook(player, 0, true);
	player->m_vehicleSize = playerBase->m_vehicleSize;
	player->updatePlayerScale();

	float ratio = (1.0f / 60.0f) / (dt);
	float accuracyValue = (float)hacks.trajectoryAccuracy / 1000.0f;

	float endDt = dt / accuracyValue;

	rotateAction = nullptr;
	float elapsed = 0.0f;

	if (!playerBase->m_isOnGround && res.gamemode == gd::Gamemode::kGamemodeCube)
	{
		auto ac = static_cast<CCRotateBy*>(playerBase->getActionByTag(0));
		if (ac)
		{
			rotateAction = runNormalRotation(player, isp1 ? PlayLayer::player1RotRate : PlayLayer::player2RotRate);
			player->setRotation(isp1 ? ExternData::lastJumpRotP1 : ExternData::lastJumpRotP2);
			rotateAction->startWithTarget(player);
			elapsed = ac->getElapsed();
			rotateAction->step(0);
			rotateAction->step(elapsed);
		}
	}

	const double delta60 = endDt * 60.0;

	const double delta240 = delta60 * 4.0;
	const float newStepCount = delta240 < 0.0 ? ceil(delta240 - 0.5) : floor(delta240 + 0.5);
	int subStepCount = 4;
	if (newStepCount >= 4.0)
	{
		subStepCount = static_cast<int>(newStepCount);
	}

	const float stepDelta60 = delta60 / (float)subStepCount;
	const double stepDelta = endDt / (float)subStepCount;

	while (player->getPositionX() < playerBase->getPositionX() + 390)
	{
		auto prevPos = player->getPosition();
		player->m_collisionLog->removeAllObjects();
		player->m_collisionLog1->removeAllObjects();
		for (int curStep = 0; curStep < subStepCount; curStep++)
		{
			if (m_pDieInSimulation)
				break;
			playerupdate(player, stepDelta60);
			playerupdateshiprotation(player, stepDelta60);
			playercheckCollisions(playLayer, player, stepDelta60);
			playerupdateSpecial(player, stepDelta);
			
		}
		if (rotateAction && !player->m_isOnGround)
			rotateAction->step(endDt);
		playerupdateRotation(player, delta60);
		HitboxNode::getInstance()->drawSegment(prevPos, player->getPosition(), 0.65f, cocos2d::ccc4f(0, 1, 0.1, 1));
		if (m_pDieInSimulation)
			break;
	}

	HitboxNode::getInstance()->drawForPlayer1(player);
	res.Apply(player, false);
	PlayLayer::releaseButton(player, 0);
	m_pDieInSimulation = false;

	if (rotateAction)
	{
		rotateAction->step(-rotateAction->getElapsed());
		rotateAction->step(elapsed);
	}

	while (player->getPositionX() < playerBase->getPositionX() + 390)
	{
		auto prevPos = player->getPosition();
		for (int curStep = 0; curStep < subStepCount; curStep++)
		{
			if (m_pDieInSimulation)
				break;
			player->m_collisionLog->removeAllObjects();
			player->m_collisionLog1->removeAllObjects();
			playerupdate(player, stepDelta60);
			playerupdateshiprotation(player, stepDelta60);
			playercheckCollisions(playLayer, player, stepDelta60);
			playerupdateSpecial(player, stepDelta);
		}
		if (rotateAction && !player->m_isOnGround)
			rotateAction->step(endDt);
		playerupdateRotation(player, delta60);
		HitboxNode::getInstance()->drawSegment(prevPos, player->getPosition(), 0.65f, cocos2d::ccc4f(0, 1, 0.1, 1));
		if (m_pDieInSimulation)
			break;
	}

	HitboxNode::getInstance()->drawForPlayer1(player);
	res.Apply(player, true);
	player->stopAction(rotateAction);
}

void TrajectorySimulation::processMainSimulation(float dt)
{
	auto playLayer = gd::GameManager::sharedState()->getPlayLayer();
	if (!playLayer || !m_pPlayer1ForSimulation || !m_pPlayer2ForSimulation || !HitboxNode::getInstance())
		return;

	HitboxNode::getInstance()->setVisible(true);
	HitboxNode::getInstance()->clear();

	if (playLayer->m_hasCompletedLevel || !hacks.trajectory)
		return;

	m_pIsSimulation = true;

	auto safeRead = Hacks::ReadBytes(gd::base + 0x20A3D1, 6);
	auto safeRead1 = Hacks::ReadBytes(gd::base + 0x1FD572, 7);
	auto safeRead2 = Hacks::ReadBytes(gd::base + 0x1E9A3F, 6);
	auto safeRead3 = Hacks::ReadBytes(gd::base + 0x1E9A5E, 1);
	auto safeRead4 = Hacks::ReadBytes(gd::base + 0x20C6B9, 1);
	auto safeRead5 = Hacks::ReadBytes(gd::base + 0x20C69B, 2);

	auto safeSpiderRead = Hacks::ReadBytes(gd::base + 0x1EDCE3, 6);
	auto safeSpiderRead1 = Hacks::ReadBytes(gd::base + 0x1EDA0D, 6);

	auto safeRespawn = Hacks::ReadBytes(gd::base + 0x1EF36D, 6);

	auto safePADNoTouch = Hacks::ReadBytes(gd::base + 0x10EDA7, 1);
	auto safeORBNoTouch = Hacks::ReadBytes(gd::base + 0x10EF1A, 1);

	auto safeCCcircleWave = Hacks::ReadBytes(gd::base + 0x1F8CAE, 6);

	auto safeDashA = Hacks::ReadBytes(gd::base + 0x1EE5DA, 7);
	auto safeDashA1 = Hacks::ReadBytes(gd::base + 0x1EE51C, 7);
	auto safeDashA2 = Hacks::ReadBytes(gd::base + 0x1EE4E9, 7);
	auto safeDashA3 = Hacks::ReadBytes(gd::base + 0x1EE581, 7);
	auto safeDashA4 = Hacks::ReadBytes(gd::base + 0x1EE4D6, 7);
	auto safeDashA5 = Hacks::ReadBytes(gd::base + 0x1EE488, 7);
	auto safeDashA6 = Hacks::ReadBytes(gd::base + 0x1EE4FE, 7);
	auto safeDashA7 = Hacks::ReadBytes(gd::base + 0x1EE07B, 7);
	auto safeDashA8 = Hacks::ReadBytes(gd::base + 0x1EE136, 6);
	auto safeDashA9 = Hacks::ReadBytes(gd::base + 0x1EE16B, 6);

	auto safeTouchRingMore = Hacks::ReadBytes(gd::base + 0xEF11D, 2);
	auto safeTouchRingSafeCircleRing = Hacks::ReadBytes(gd::base + 0x1F5959, 1);
	auto safeFlashAndMore = Hacks::ReadBytes(gd::base + 0x1FFE70, 7);

	Hacks::writeBytes(gd::base + 0x20A3D1, {0xE9, 0x7B, 0x01, 0x00, 0x00, 0x90});
	Hacks::writeBytes(gd::base + 0x1FD572, {0xE9, 0x8B, 0x03, 0x00, 0x00, 0x90, 0x90});
	Hacks::writeBytes(gd::base + 0x1E9A3F, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
	Hacks::writeBytes(gd::base + 0x1E9A5E, {0x90});
	Hacks::writeBytes(gd::base + 0x20C6B9, {0x90});
	Hacks::writeBytes(gd::base + 0x20C69B, {0x6A, 0x00});
	Hacks::writeBytes(gd::base + 0x1EDCE3, {0xE9, 0x90, 0x01, 0x00, 0x00, 0x90});
	Hacks::writeBytes(gd::base + 0x1EDA0D, {0xE9, 0x2F, 0x02, 0x00, 0x00, 0x90});
	Hacks::writeBytes(gd::base + 0x1EF36D, {0xE9, 0xA8, 0x00, 0x00, 0x00, 0x90});
	Hacks::writeBytes(gd::base + 0x10EDA7, {0xEB});
	Hacks::writeBytes(gd::base + 0x10EF1A, {0xEB});
	Hacks::writeBytes(gd::base + 0x1F8CAE, {0xE9, 0x93, 0x01, 0x00, 0x00, 0x90});

	Hacks::writeBytes(gd::base + 0x1EE5DA, {0xC7, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00});
	Hacks::writeBytes(gd::base + 0x1EE51C, {0xC7, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00});
	Hacks::writeBytes(gd::base + 0x1EE4E9, {0xC7, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00});
	Hacks::writeBytes(gd::base + 0x1EE581, {0xC7, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00});
	Hacks::writeBytes(gd::base + 0x1EE4D6, {0xC7, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00});
	Hacks::writeBytes(gd::base + 0x1EE488, {0xC7, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00});
	Hacks::writeBytes(gd::base + 0x1EE4FE, {0xC7, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00});
	Hacks::writeBytes(gd::base + 0x1EE07B, {0xC7, 0x04, 0x24, 0x00, 0x00, 0x00, 0x00});
	Hacks::writeBytes(gd::base + 0x1EE136, {0x90, 0x90, 0x90, 0x90, 0x90, 0x90});
	Hacks::writeBytes(gd::base + 0x1EE16B, {0xE9, 0x7F, 0x01, 0x00, 0x00, 0x90});

	Hacks::writeBytes(gd::base + 0xEF11D, {0x90, 0x90});
	Hacks::writeBytes(gd::base + 0x1F5959, {0xEB});

	Hacks::writeBytes(gd::base + 0x1FFE70, {0xC2, 0x20, 0x00, 0x90, 0x90, 0x68});

	auto safeCircles = playLayer->m_circleWaves->count();
	this->simulationPerPlayer(m_pPlayer1ForSimulation, playLayer->m_pPlayer1, dt);
	m_pDieInSimulation = false;
	if (playLayer->m_pPlayer2 && playLayer->m_bIsDualMode)
		this->simulationPerPlayer(m_pPlayer2ForSimulation, playLayer->m_pPlayer2, dt);
	m_pDieInSimulation = false;

	if (safeCircles < playLayer->m_circleWaves->count())
	{
		auto cantRemove = playLayer->m_circleWaves->count() - safeCircles;
		for (size_t i = 0; i < cantRemove; i++)
		{
			int index = playLayer->m_circleWaves->count() - 1;
			reinterpret_cast<gd::CCCircleWave*>(playLayer->m_circleWaves->objectAtIndex(index))->removeMeAndCleanup();
		}
	}

	Hacks::writeBytes(gd::base + 0x20A3D1, safeRead);
	Hacks::writeBytes(gd::base + 0x1FD572, safeRead1);
	Hacks::writeBytes(gd::base + 0x1E9A3F, safeRead2);
	Hacks::writeBytes(gd::base + 0x1E9A5E, safeRead3);
	Hacks::writeBytes(gd::base + 0x20C6B9, safeRead4);
	Hacks::writeBytes(gd::base + 0x20C69B, safeRead5);
	Hacks::writeBytes(gd::base + 0x1EDCE3, safeSpiderRead);
	Hacks::writeBytes(gd::base + 0x1EDA0D, safeSpiderRead1);
	Hacks::writeBytes(gd::base + 0x1EF36D, safeRespawn);
	Hacks::writeBytes(gd::base + 0x10EDA7, safePADNoTouch);
	Hacks::writeBytes(gd::base + 0x10EF1A, safeORBNoTouch);
	Hacks::writeBytes(gd::base + 0x1F8CAE, safeCCcircleWave);

	Hacks::writeBytes(gd::base + 0x1EE5DA, safeDashA);
	Hacks::writeBytes(gd::base + 0x1EE51C, safeDashA1);
	Hacks::writeBytes(gd::base + 0x1EE4E9, safeDashA2);
	Hacks::writeBytes(gd::base + 0x1EE581, safeDashA3);
	Hacks::writeBytes(gd::base + 0x1EE4D6, safeDashA4);
	Hacks::writeBytes(gd::base + 0x1EE488, safeDashA5);
	Hacks::writeBytes(gd::base + 0x1EE4FE, safeDashA6);
	Hacks::writeBytes(gd::base + 0x1EE07B, safeDashA7);
	Hacks::writeBytes(gd::base + 0x1EE136, safeDashA8);
	Hacks::writeBytes(gd::base + 0x1EE16B, safeDashA9);

	Hacks::writeBytes(gd::base + 0xEF11D, safeTouchRingMore);
	Hacks::writeBytes(gd::base + 0x1F5959, safeTouchRingSafeCircleRing);
	Hacks::writeBytes(gd::base + 0x1FFE70, safeFlashAndMore);

	m_pIsSimulation = false;
	m_pDieInSimulation = false;
}

bool TrajectorySimulation::shouldInterrumpHooksWithPlayer(gd::PlayerObject* player)
{
	auto playLayer = gd::GameManager::sharedState()->getPlayLayer();
	if (!playLayer)
		return false;
	if ((player == m_pPlayer1ForSimulation || player == m_pPlayer2ForSimulation || m_pIsSimulation))
	{
		return true;
	}
	return false;
}

void TrajectorySimulation::activateObjectsOnPlayerSimulations(gd::GameObject* obj, gd::PlayerObject* player)
{
	auto playLayer = gd::GameManager::sharedState()->getPlayLayer();
	if (!playLayer)
		return;
	auto typeObj = obj->getType();

	if (typeObj != gd::kGameObjectTypeBallPortal && typeObj != gd::kGameObjectTypeShipPortal &&
		typeObj != gd::kGameObjectTypeUfoPortal && typeObj != gd::kGameObjectTypeWavePortal &&
		typeObj != gd::kGameObjectTypeRobotPortal && typeObj != gd::kGameObjectTypeCubePortal)
	{
		return;
	}

	player->toggleFlyMode(false);
	player->toggleBirdMode(false);
	player->toggleRollMode(false);
	player->toggleDartMode(false);
	player->toggleRobotMode(false);
	player->toggleSpiderMode(false);

	debug.debugNumber = typeObj;

	if (typeObj == gd::kGameObjectTypeBallPortal)
	{
		player->toggleRollMode(true);
	}
	else if (typeObj == gd::kGameObjectTypeShipPortal)
	{
		player->toggleFlyMode(true);
	}
	else if (typeObj == gd::kGameObjectTypeUfoPortal)
	{
		player->toggleBirdMode(true);
	}
	else if (typeObj == gd::kGameObjectTypeWavePortal)
	{
		player->toggleDartMode(true);
	}
	else if (typeObj == gd::kGameObjectTypeRobotPortal)
	{
		player->toggleRobotMode(true);
	}
	else if (typeObj == gd::kGameObjectTypeSpiderPortal)
	{
		player->toggleSpiderMode(true);
	}
}

bool TrajectorySimulation::shouldInterrumpHooks()
{
	auto playLayer = gd::GameManager::sharedState()->getPlayLayer();
	if (!playLayer)
		return false;
	return m_pIsSimulation;
}

TrajectorySimulation* TrajectorySimulation::getInstance()
{
	static TrajectorySimulation instance;
	return &instance;
}

void TrajectorySimulation::onQuitPlayLayer()
{
	m_pDieInSimulation = false;
	m_pIsSimulation = false;
	m_pPlayer1ForSimulation = nullptr;
	m_pPlayer2ForSimulation = nullptr;
}