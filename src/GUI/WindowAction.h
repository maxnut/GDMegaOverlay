#pragma once
#include "Window.h"
#include <cocos2d.h>

using namespace cocos2d;

class WindowAction
{
private:
	GUI::Window* m_window;
	ImVec2 m_initialPosition = { 0, 0 };
	ImVec2 m_finalPosition = { 0, 0 };

	ImVec2 m_totalMovement = { 0, 0 };
	float m_duration = 0, m_elapsed = 0;
	bool m_firstTick = false;

public:
	void update(float time);
	void step(float dt);
	bool isDone();
	static WindowAction* create(float duration, GUI::Window* window, ImVec2 targetPosition);
	bool initWithDuration(float duration, GUI::Window* window, ImVec2 targetPosition);
};
