#include <vector>
#include "pch.h"
#include <iostream>
#include <deque>

class HitboxNode final : public cocos2d::CCDrawNode
{
public:
	HitboxNode();
	static HitboxNode *create();

	static HitboxNode *getInstance();

	void drawCircleHitbox(cocos2d::CCPoint const &center, float radius, cocos2d::ccColor4B col) const;

	void drawPolygonHitbox(std::vector<cocos2d::CCPoint> const &points, cocos2d::ccColor4B col) const;

	void drawRectangleHitbox(cocos2d::CCRect const &rect, cocos2d::ccColor4B col) const;

	// i dont know why, but if i remove these or interact with them the game crashes, so i will just leave them here
	std::deque<cocos2d::CCRect> m_player1Queue;
	std::deque<cocos2d::CCRect> m_player2Queue;

	float m_thickness;
	float m_trailLength;
	bool m_drawTrail;
	bool m_noLimitTrail;

	bool m_isMini1, m_isMini2;

	void clearQueue() const;
	void addToPlayer1Queue(cocos2d::CCRect const &rect) const;
	void addToPlayer2Queue(cocos2d::CCRect const &rect) const;

	void drawForPlayer1(gd::PlayerObject *obj) const;
	void drawForPlayer2(gd::PlayerObject *obj) const;
	void drawForObject(gd::GameObject *obj) const;

	std::vector<cocos2d::CCPoint> triangleForObject(gd::GameObject *obj) const;
	std::vector<cocos2d::CCPoint> quadrilateralForObject(gd::GameObject *obj) const;

	// these are fine tho
	std::deque<cocos2d::CCRect> boo1;
	std::deque<cocos2d::CCRect> boo2;
};
