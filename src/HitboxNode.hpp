#include <vector>
#include "pch.h"
#include <iostream>
#include <deque>

class HitboxNode : public cocos2d::CCDrawNode
{
public:
	HitboxNode();
	static HitboxNode *create();

	static HitboxNode *getInstance();

	virtual std::vector<cocos2d::CCPoint> drawCircleHitbox(cocos2d::CCPoint const &center, float radius, cocos2d::ccColor4B col);

	virtual void drawPolygonHitbox(std::vector<cocos2d::CCPoint> const &points, cocos2d::ccColor4B col);

	virtual void drawRectangleHitbox(cocos2d::CCRect const &rect, cocos2d::ccColor4B col);

	virtual void drawObjectWithRotation(gd::GameObject* obj);

	// i dont know why, but if i remove these or interact with them the game crashes, so i will just leave them here
	std::deque<cocos2d::CCRect> m_player1Queue;
	std::deque<cocos2d::CCRect> m_player2Queue;

	float m_thickness;
	float m_trailLength;
	bool m_drawTrail;
	bool m_noLimitTrail;

	virtual void clearQueue();
	virtual void addToPlayer1Queue(cocos2d::CCRect const &rect);
	virtual void addToPlayer2Queue(cocos2d::CCRect const &rect);

	virtual void drawForPlayer1(gd::PlayerObject *obj);
	virtual void drawForPlayer2(gd::PlayerObject *obj);
	virtual void drawForObject(gd::GameObject *obj);

	virtual std::vector<cocos2d::CCPoint> triangleForObject(gd::GameObject *obj);
	virtual std::vector<cocos2d::CCPoint> quadrilateralForObject(gd::GameObject *obj);

	// these are fine tho
	std::deque<cocos2d::CCRect> boo1;
	std::deque<cocos2d::CCRect> boo2;
};
