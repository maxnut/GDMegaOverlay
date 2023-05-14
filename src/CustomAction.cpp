#include "CustomAction.h"
#include "Hacks.h"

CustomAction* CustomAction::create(float duration, float from, float to, float* updateProperty, std::function<void()> callback)
{
	CustomAction* ret = new CustomAction();
	if (ret->initWithDuration(duration, from, to, updateProperty, callback))
	{
		ret->autorelease();
		return ret;
	}

	delete ret;
	return nullptr;
}

bool CustomAction::initWithDuration(float duration, float from, float to, float* updateProperty, std::function<void()> callback)
{
	if (CCActionInterval::initWithDuration(duration))
	{
		_to = to;
		_from = from;
        _updateProperty = updateProperty;
        _callback = callback;
		return true;
	}

	return false;
}

void CustomAction::startWithTarget(CCNode* target)
{
	CCActionInterval::startWithTarget(target);
	_delta = _to - _from;
}

void CustomAction::update(float dt)
{
	*_updateProperty = _to - _delta * (1 - dt);
    if(dt >= 1 && _callback) _callback();
}