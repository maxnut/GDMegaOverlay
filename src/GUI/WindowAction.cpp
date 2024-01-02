#include "WindowAction.h"
#include <algorithm>

WindowAction* WindowAction::create(float duration, GUI::Window* window, ImVec2 targetPosition)
{
	WindowAction* ret = new WindowAction();
	if (ret->initWithDuration(duration, window, targetPosition))
		return ret;

	delete ret;
	return nullptr;
}

bool WindowAction::initWithDuration(float duration, GUI::Window* window, ImVec2 targetPosition)
{
	m_duration = duration;
	m_window = window;
	m_initialPosition = m_window->renderPosition;
	m_totalMovement = { targetPosition.x - m_initialPosition.x, targetPosition.y - m_initialPosition.y };
	m_finalPosition = targetPosition;

	return true;
}

float tweenFunc(float time)
{
    time = time * 2;
    if (time < 1)
        return 0.5f * time * time;
    --time;
    return -0.5f * (time * (time - 2) - 1);
}

void WindowAction::step(float dt)
{
	if (isDone())
		return;
	
	if (m_firstTick)
	{
		m_firstTick = false;
		m_elapsed = 0;
	}
	else
	{
		m_elapsed += dt;
	}

	float updateTime = m_elapsed / m_duration;

	float updateTimeTween = tweenFunc(updateTime);

	this->update(updateTimeTween);
}

bool WindowAction::isDone()
{
	return m_elapsed >= m_duration;
}

void WindowAction::update(float time)
{
	ImVec2 moveCurrent = { m_totalMovement.x * time, m_totalMovement.y * time };
	m_window->renderPosition = { m_initialPosition.x + moveCurrent.x, m_initialPosition.y + moveCurrent.y };

	if (isDone())
		m_window->renderPosition = m_finalPosition;
}