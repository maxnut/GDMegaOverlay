#include "ShowHitboxes.h"
#include "Common.h"
#include "Settings.hpp"

#include <Geode/modify/CCDrawNode.hpp>
#include <Geode/modify/LevelEditorLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>

using namespace ShowHitboxes;

CCDrawNode* ShowHitboxes::getDrawNode()
{
    return MBO(CCDrawNode*, GameManager::get()->getGameLayer(), 11632);
}

CCRect ShowHitboxes::getObjectRect(cocos2d::CCRect r, float a, float b)
{
	r.origin.x += r.size.width / 2;
	r.origin.y += r.size.height / 2;
	r.size.width *= a;
	r.size.height *= b;
	r.origin.x -= r.size.width / 2;
	r.origin.y -= r.size.height / 2;
	return r;
}

void ShowHitboxes::drawRectangleHitbox(CCDrawNode* node, cocos2d::CCRect const& rect, ccColor4B colBase)
{
	constexpr size_t N = 4;
	std::vector<cocos2d::CCPoint> points(N);

	points[0] = CCPointMake(rect.getMinX(), rect.getMinY());
	points[1] = CCPointMake(rect.getMinX(), rect.getMaxY());
	points[2] = CCPointMake(rect.getMaxX(), rect.getMaxY());
	points[3] = CCPointMake(rect.getMaxX(), rect.getMinY());

	node->drawPolygon(const_cast<cocos2d::CCPoint*>(points.data()), points.size(), ccc4FFromccc4B(colBase),
					    0.25, ccc4FFromccc4B(colBase));
}

void ShowHitboxes::drawForPlayer(CCDrawNode* node, PlayerObject* player)
{
    if(!player)
        return;

	CCRect* rect1 = reinterpret_cast<CCRect*(__thiscall*)(GameObject*)>(base::get() + 0x13a570)(player);
    CCRect rect2 = player->m_vehicleSize >= 1.f ? getObjectRect(*rect1, 0.25f, 0.25f) : getObjectRect(*rect1, 0.4f, 0.4f);
	drawRectangleHitbox(node, *rect1, ccColor4B(255, 0, 0, 255));
	drawRectangleHitbox(node, rect2, ccColor4B(0, 255, 0, 255));
}

void ShowHitboxes::forceDraw(GJBaseGameLayer* self, bool editor)
{
    bool show = Settings::get<bool>("level/show_hitbox/enabled", false);
    getDrawNode()->setVisible(show);

    GameManager::get()->setGameVariable("0045", show);

    if(!show)
        return;

    bool onDeath = Settings::get<bool>("level/show_hitbox/on_death", false);

    if(onDeath)
        getDrawNode()->setVisible(dead);

    debugDrawing = true;
    reinterpret_cast<void(__thiscall*)(cocos2d::CCLayer*)>(base::get() + (editor ? 0x248420 : 0x1986f0))(
		self);

    bool isDual = MBO(bool, self, 878);
    bool trail = Settings::get<bool>("level/show_hitbox/queue_enabled", false);

    drawForPlayer(getDrawNode(), self->m_player1);
    if(isDual)
        drawForPlayer(getDrawNode(), self->m_player2);

    if(!trail)
    {
        debugDrawing = false;
        return;
    }

    for(auto&pair : player1Queue)
    {
        drawRectangleHitbox(getDrawNode(), pair.first, ccColor4B(255, 0, 0, 255));
	    drawRectangleHitbox(getDrawNode(), pair.second, ccColor4B(0, 255, 0, 255));
    }

    for(auto&pair : player2Queue)
    {
        drawRectangleHitbox(getDrawNode(), pair.first, ccColor4B(255, 0, 0, 255));
	    drawRectangleHitbox(getDrawNode(), pair.second, ccColor4B(0, 255, 0, 255));
    }
    debugDrawing = false;
}

void LevelEditorLayerUpdateEditor(LevelEditorLayer* self, float dt)
{
    reinterpret_cast<void(__thiscall *)(LevelEditorLayer *, float)>(base::get() + 0x23f720)(self, dt);

    dead = false;

    forceDraw(self, true);
}

void GJBaseGameLayerProcessCommands(GJBaseGameLayer* self)
{
    reinterpret_cast<void(__thiscall *)(GJBaseGameLayer *)>(base::get() + 0x1BD240)(self);

    bool show = Settings::get<bool>("level/show_hitbox/enabled", false);
    bool trail = Settings::get<bool>("level/show_hitbox/queue_enabled", false);

    if(!show || !trail)
        return;

    bool isDual = MBO(bool, self, 878);

    CCRect* rect1 = reinterpret_cast<CCRect*(__thiscall*)(GameObject*)>(base::get() + 0x13a570)(self->m_player1);
    CCRect rect2 = self->m_player1->m_vehicleSize >= 1.f ? getObjectRect(*rect1, 0.25f, 0.25f) : getObjectRect(*rect1, 0.4f, 0.4f);

    player1Queue.push_back({*rect1, rect2});

    int maxQueue = Settings::get<int>("level/show_hitbox/max_queue", 240);

    if(player1Queue.size() > maxQueue)
        player1Queue.pop_front();

    if(!isDual)
        return;

    rect1 = reinterpret_cast<CCRect*(__thiscall*)(GameObject*)>(base::get() + 0x13a570)(self->m_player2);
    rect2 = self->m_player2->m_vehicleSize >= 1.f ? getObjectRect(*rect1, 0.25f, 0.25f) : getObjectRect(*rect1, 0.4f, 0.4f);

    player2Queue.push_back({*rect1, rect2});

    if(player2Queue.size() > maxQueue)
        player2Queue.pop_front();
}

$execute
{
	Mod::get()->hook(reinterpret_cast<void *>(base::get() + 0x23f720), &LevelEditorLayerUpdateEditor, "LevelEditorLayer::UpdateEditor", tulip::hook::TulipConvention::Thiscall);
    Mod::get()->hook(reinterpret_cast<void *>(base::get() + 0x1BD240), &GJBaseGameLayerProcessCommands, "GJBaseGameLayer::processCommands", tulip::hook::TulipConvention::Thiscall);
}

class $modify(CCDrawNode)
{
    bool drawPolygon(CCPoint *verts, unsigned int count, const ccColor4F &colFill, float borderWidth, const ccColor4F& colBase)
    {
        ccColor4F colBaseNew = colBase;
        ccColor4F colFillNew = colBase;

        if(ShowHitboxes::debugDrawing)
        {
            borderWidth = Settings::get<float>("level/show_hitbox/size", 0.25f);
            colBaseNew.a = Settings::get<int>("level/show_hitbox/border_alpha", 255) / 255.f;
            colFillNew.a = Settings::get<int>("level/show_hitbox/fill_alpha", 50) / 255.f;
        }
        
        bool res = CCDrawNode::drawPolygon(verts, count, colFillNew, borderWidth, colBaseNew);

        return res;
    }
};

class $modify(PlayLayer)
{
    void postUpdate(float dt)
    {
        PlayLayer::postUpdate(dt);

        forceDraw(this, false);
    };

    void resetLevel()
    {
        PlayLayer::resetLevel();
        player1Queue.clear();
        player2Queue.clear();

        dead = false;
    }

    void fullReset()
    {
        PlayLayer::fullReset();

        dead = false;
    }

    void destroyPlayer(PlayerObject* p0, GameObject* p1)
    {
        PlayLayer::destroyPlayer(p0, p1);

        if(p1 && p1->getPositionX() == 0 && p1->getPositionY() == 105)
            return;

        dead = true;
    }
};

class $modify(LevelEditorLayer)
{
    void onPlaytest()
    {
        LevelEditorLayer::onPlaytest();

        player1Queue.clear();
        player2Queue.clear();
    }
};

class $modify(GJBaseGameLayer)
{
    bool init()
    {
        dead = false;
        return GJBaseGameLayer::init();
    }
};