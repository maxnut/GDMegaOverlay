#pragma once
#include "pch.h"

class TrajectorySimulation
{
  public:
	gd::PlayerObject* m_pPlayer1ForSimulation = nullptr;
	gd::PlayerObject* m_pPlayer2ForSimulation = nullptr;
	bool m_pIsSimulation = false;
	bool m_pDieInSimulation = false;
    CCActionInterval* rotateAction;

	void createPlayersForSimulation();
	void createSimulation();
	void simulationPerPlayer(gd::PlayerObject* player, gd::PlayerObject* playerBase, float dt);
	void processMainSimulation(float dt);
	bool shouldInterrumpHooksWithPlayer(gd::PlayerObject* player);
	void activateObjectsOnPlayerSimulations(gd::GameObject* obj, gd::PlayerObject* player);
	bool shouldInterrumpHooks();
	static TrajectorySimulation* getInstance();
	void onQuitPlayLayer();
};