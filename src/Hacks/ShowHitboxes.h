#pragma once

#include <Geode/Geode.hpp>
#include <Geode/modify/GJBaseGameLayer.hpp>

#include <deque>

using namespace geode::prelude;

namespace ShowHitboxes
{
    inline bool debugDrawing = false;
    inline bool dead = false;
    inline std::deque<std::pair<CCRect, CCRect>> player1Queue;
    inline std::deque<std::pair<CCRect, CCRect>> player2Queue;

    CCDrawNode* getDrawNode();
    CCRect getObjectRect(cocos2d::CCRect r, float a, float b);

    void drawForPlayer(CCDrawNode* node, PlayerObject* player);
    void drawRectangleHitbox(CCDrawNode* node, cocos2d::CCRect const& rect, ccColor4B col);
    void forceDraw(GJBaseGameLayer* self, bool editor);
}