#pragma once
#include "pch.h"

class CustomAction : public CCActionInterval
{
  private:
	float _from, _to, _delta;
	float* _updateProperty;

    std::function<void()> _callback;

  public:
	virtual void update(float time) override;
	static CustomAction* create(float duration, float from, float to, float* updateProperty, std::function<void()> callback);
    bool initWithDuration(float duration, float from, float to, float* updateProperty, std::function<void()> callback);
    void startWithTarget(CCNode* target) override;
};