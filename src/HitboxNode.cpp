#include "HitboxNode.hpp"
#include "bools.h"
#include "Hacks.h"

extern struct HacksStr hacks;

HitboxNode::HitboxNode()
{
}

HitboxNode *HitboxNode::create()
{
	auto ret = new HitboxNode();
	if (ret && ret->init())
	{
		// ret->autorelease();
		return ret;
	}
	CC_SAFE_RELEASE(ret);
	return nullptr;
}

HitboxNode *HitboxNode::getInstance()
{
	static HitboxNode *instance;
	if (!instance)
		instance = create();
	return instance;
}

void HitboxNode::drawPolygonHitbox(std::vector<cocos2d::CCPoint> const &points, cocos2d::ccColor4B col)
{
	col.a = hacks.borderOpacity;
	auto color1 = ccc4FFromccc4B(col);
	auto colorA = (ccc4(col.r, col.g, col.b, hacks.hitboxOpacity));

	this->drawPolygon(const_cast<cocos2d::CCPoint *>(points.data()), points.size(), ccc4FFromccc4B(colorA), hacks.hitboxThickness, color1);
}

void HitboxNode::drawRectangleHitbox(cocos2d::CCRect const &rect, cocos2d::ccColor4B col)
{
	constexpr size_t N = 4;
	std::vector<cocos2d::CCPoint> points(N);

	points[0] = CCPointMake(rect.getMinX(), rect.getMinY());
	points[1] = CCPointMake(rect.getMinX(), rect.getMaxY());
	points[2] = CCPointMake(rect.getMaxX(), rect.getMaxY());
	points[3] = CCPointMake(rect.getMaxX(), rect.getMinY());

	this->drawPolygonHitbox(points, col);
}

void HitboxNode::clearQueue()
{
	boo1.clear();
	boo2.clear();
}
void HitboxNode::addToPlayer1Queue(cocos2d::CCRect const &rect)
{

	if (hacks.hitboxTrail)
	{
		while (boo1.size() > 0 && boo1.front().origin.x < rect.origin.x - hacks.hitboxTrailLength)
		{
			boo1.pop_front();
		}
	}
	else if (boo1.size() > 0)
	{
		boo1.clear();
	}
	boo1.push_back(rect);
}

void HitboxNode::addToPlayer2Queue(CCRect const &rect)
{
	if (hacks.hitboxTrail)
	{
		while (boo2.size() > 0 && boo2.front().origin.x < rect.origin.x - hacks.hitboxTrailLength)
		{
			boo2.pop_front();
		}
	}
	else if (boo1.size() > 0)
	{
		boo2.clear();
	}
	boo2.push_back(rect);
}

void HitboxNode::drawForPlayer1(gd::PlayerObject *obj)
{
	if (boo1.size() <= 0)
		this->drawRectangleHitbox(obj->getObjectRect(), ccc4(255, 63, 63, 255));
	else
	{
		for (size_t i = 0; i < boo1.size(); i++)
		{
			this->drawRectangleHitbox(boo1[i], ccc4(255, 63, 63, 255));
		}
	}
	this->drawPolygonHitbox(this->quadrilateralForObject(obj), ccc4(127, 0, 0, 255));
	this->drawRectangleHitbox(!m_isMini1 ? obj->getObjectRect(0.25, 0.25) : obj->getObjectRect(0.4, 0.4), ccc4(0, 255, 0, 255));
}

void HitboxNode::drawForPlayer2(gd::PlayerObject *obj)
{
	if (boo2.size() <= 0)
		this->drawRectangleHitbox(obj->getObjectRect(), ccc4(255, 63, 63, 255));
	else
	{
		for (size_t i = 0; i < boo2.size(); i++)
		{
			this->drawRectangleHitbox(boo2[i], ccc4(255, 63, 63, 255));
		}
	}
	this->drawPolygonHitbox(this->quadrilateralForObject(obj), ccc4(127, 0, 0, 255));
	this->drawRectangleHitbox(!m_isMini2 ? obj->getObjectRect(0.25, 0.25) : obj->getObjectRect(0.4, 0.4), ccc4(0, 255, 0, 255));
}

std::vector<CCPoint> HitboxNode::triangleForObject(gd::GameObject *obj)
{
	constexpr size_t N = 3;
	std::vector<cocos2d::CCPoint> points(N);
	int i = 0;
	obj->updateOrientedBox();
	if (obj->m_pObjectOBB2D)
	{
		auto obb = obj->m_pObjectOBB2D;
		if (obj->m_bIsFlippedX || !obj->m_bIsFlippedY)
			points[i++] = obb->m_p1_1;
		if (!obj->m_bIsFlippedX || !obj->m_bIsFlippedY)
			points[i++] = obb->m_p1_2;
		if (!obj->m_bIsFlippedX || obj->m_bIsFlippedY)
			points[i++] = obb->m_p1_3;
		if (obj->m_bIsFlippedX || obj->m_bIsFlippedY)
			points[i++] = obb->m_p1_4;
	}
	else
	{
		auto rect = obj->getObjectRect();
		if (obj->m_bIsFlippedX || !obj->m_bIsFlippedY)
			points[i++] = CCPointMake(rect.getMinX(), rect.getMinY());
		if (!obj->m_bIsFlippedX || !obj->m_bIsFlippedY)
			points[i++] = CCPointMake(rect.getMinX(), rect.getMaxY());
		if (!obj->m_bIsFlippedX || obj->m_bIsFlippedY)
			points[i++] = CCPointMake(rect.getMaxX(), rect.getMaxY());
		if (obj->m_bIsFlippedX || obj->m_bIsFlippedY)
			points[i++] = CCPointMake(rect.getMaxX(), rect.getMinY());
	}
	return points;
}
std::vector<CCPoint> HitboxNode::quadrilateralForObject(gd::GameObject *obj)
{
	constexpr size_t N = 4;
	std::vector<CCPoint> points(N);
	if (obj->m_pObjectOBB2D)
	{
		auto obb = obj->m_pObjectOBB2D;
		points[0] = obb->m_p1_1;
		points[1] = obb->m_p1_2;
		points[2] = obb->m_p1_3;
		points[3] = obb->m_p1_4;
	}
	else
	{
		auto rect = obj->getObjectRect();
		points[0] = CCPointMake(rect.getMinX(), rect.getMinY());
		points[1] = CCPointMake(rect.getMinX(), rect.getMaxY());
		points[2] = CCPointMake(rect.getMaxX(), rect.getMaxY());
		points[3] = CCPointMake(rect.getMaxX(), rect.getMinY());
	}
	return points;
}

void HitboxNode::drawCircleHitbox(cocos2d::CCPoint const &center, float radius, cocos2d::ccColor4B col)
{
	constexpr size_t N = 64;
	constexpr double PI = 3.1415926535;
	std::vector<cocos2d::CCPoint> points(N);
	for (size_t i = 0; i < N; ++i)
	{
		points[i] = center + CCPointMake(std::cos(2 * PI * i / N), std::sin(2 * PI * i / N)) * radius;
	}
	this->drawPolygonHitbox(points, col);
}

extern struct HacksStr hacks;

void HitboxNode::drawForObject(gd::GameObject *obj)
{
	if (obj->m_nObjectID != 749 && obj->getType() == gd::GameObjectType::kGameObjectTypeDecoration && !hacks.showDecorations)
		return;
	if (!obj->m_bActive)
		return;

	ccColor4B color;
	switch (obj->getType())
	{
	case gd::GameObjectType::kGameObjectTypeSolid:
		color = ccc4(0, 0, 255, 255);
		break;
	case gd::GameObjectType::kGameObjectTypeDecoration:
		if (hacks.showDecorations)
			color = ccc4(127, 127, 255, 255);
		switch (obj->m_nObjectID)
		{
		case 749:
			color = ccc4(255, 127, 0, 255);
			break;
		default:
			if (!hacks.showDecorations)
				return;
		}
		break;
	case gd::GameObjectType::kGameObjectTypeTeleportPortal:
		color = ccc4(0, 127, 255, 255);
		break;
	case gd::GameObjectType::kGameObjectTypeModifier:
		switch (obj->m_nObjectID)
		{
		case 200:
		case 201:
		case 202:
		case 203:
		case 1334:
			color = ccc4(255, 255, 255, 255);
			break;
		default:
			if (!hacks.showDecorations)
				return;
		}
		break;
	case gd::GameObjectType::kGameObjectTypeSlope:
		color = ccc4(0, 0, 255, 255);
		this->drawPolygonHitbox(this->triangleForObject(obj), color);
		return;
	case gd::GameObjectType::kGameObjectTypeHazard:
		color = ccc4(255, 0, 0, 255);
		break;

	case gd::GameObjectType::kGameObjectTypeInverseGravityPortal:
	case gd::GameObjectType::kGameObjectTypeYellowJumpPad:
	case gd::GameObjectType::kGameObjectTypeYellowJumpRing:
		color = ccc4(255, 255, 0, 255);
		break;
	case gd::GameObjectType::kGameObjectTypeInverseMirrorPortal:
	case gd::GameObjectType::kGameObjectTypeDualPortal:
		color = ccc4(255, 127, 0, 255);
		break;
	case gd::GameObjectType::kGameObjectTypeNormalGravityPortal:
	case gd::GameObjectType::kGameObjectTypeNormalMirrorPortal:
	case gd::GameObjectType::kGameObjectTypeSoloPortal:
		color = ccc4(0, 127, 255, 255);
		break;
	case gd::GameObjectType::kGameObjectTypePinkJumpPad:
	case gd::GameObjectType::kGameObjectTypePinkJumpRing:
		color = ccc4(255, 0, 255, 255);
		break;
	case gd::GameObjectType::kGameObjectTypeGravityPad:
	case gd::GameObjectType::kGameObjectTypeGravityRing:
		color = ccc4(0, 255, 255, 255);
		break;
	case gd::GameObjectType::kGameObjectTypeRedJumpPad:
	case gd::GameObjectType::kGameObjectTypeRedJumpRing:
		color = ccc4(255, 0, 127, 255);
		break;
	case gd::GameObjectType::kGameObjectTypeGreenRing:
	case gd::GameObjectType::kGameObjectTypeDashRing:
	case gd::GameObjectType::kGameObjectTypeRegularSizePortal:
		color = ccc4(0, 255, 0, 255);
		break;
	case gd::GameObjectType::kGameObjectTypeGravityDashRing:
	case gd::GameObjectType::kGameObjectTypeMiniSizePortal:
		color = ccc4(127, 0, 255, 255);
		break;
	case gd::GameObjectType::kGameObjectTypeSecretCoin:
	case gd::GameObjectType::kGameObjectTypeUserCoin:
	case gd::GameObjectType::kGameObjectTypeCustomRing:
	case gd::GameObjectType::kGameObjectTypeCollectible:
		color = ccc4(223, 223, 223, 255);
		break;
	case gd::GameObjectType::kGameObjectTypeDropRing:
		color = ccc4(31, 31, 31, 255);
		break;

	case gd::GameObjectType::kGameObjectTypeCubePortal:
		color = ccc4(0, 255, 0, 255);
		break;
	case gd::GameObjectType::kGameObjectTypeShipPortal:
		color = ccc4(255, 127, 255, 255);
		break;
	case gd::GameObjectType::kGameObjectTypeBallPortal:
		color = ccc4(255, 127, 0, 255);
		break;
	case gd::GameObjectType::kGameObjectTypeUfoPortal:
		color = ccc4(255, 255, 0, 255);
		break;
	case gd::GameObjectType::kGameObjectTypeWavePortal:
		color = ccc4(0, 127, 255, 255);
		break;
	case gd::GameObjectType::kGameObjectTypeRobotPortal:
		color = ccc4(223, 223, 223, 255);
		break;
	case gd::GameObjectType::kGameObjectTypeSpiderPortal:
		color = ccc4(127, 0, 255, 255);
		break;
	default:
		if (!hacks.showDecorations)
			return;
	}
	if (obj->getObjectRadius() > 0)
	{
		this->drawCircleHitbox(obj->getPosition(), obj->getObjectRadius(), color);
	}
	else
	{
		this->drawPolygonHitbox(this->quadrilateralForObject(obj), color);
	}
}