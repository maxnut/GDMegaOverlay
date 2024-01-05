#include "HitboxNode.hpp"

using namespace cocos2d;

HitboxNode::HitboxNode()
{
}

HitboxNode* HitboxNode::create()
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

HitboxNode* HitboxNode::getInstance()
{
	static HitboxNode* instance;
	if (!instance)
		instance = create();
	return instance;
}

void HitboxNode::drawPolygonHitbox(std::vector<cocos2d::CCPoint> const& points, cocos2d::ccColor4B col)
{
	col.a = 150;
	auto color1 = ccc4FFromccc4B(col);
	auto colorA = (cocos2d::ccColor4B(col.r, col.g, col.b, 50));

	this->drawPolygon(const_cast<cocos2d::CCPoint*>(points.data()), points.size(), ccc4FFromccc4B(colorA),
					  0.5f, color1);
}

void HitboxNode::drawRectangleHitbox(cocos2d::CCRect const& rect, cocos2d::ccColor4B col)
{
	constexpr size_t N = 4;
	std::vector<cocos2d::CCPoint> points(N);

	points[0] = cocos2d::CCPointMake(rect.getMinX(), rect.getMinY());
	points[1] = cocos2d::CCPointMake(rect.getMinX(), rect.getMaxY());
	points[2] = cocos2d::CCPointMake(rect.getMaxX(), rect.getMaxY());
	points[3] = cocos2d::CCPointMake(rect.getMaxX(), rect.getMinY());

	this->drawPolygonHitbox(points, col);
}

/* void HitboxNode::clearQueue()
{
	boo1.clear();
	boo2.clear();
}
void HitboxNode::addToPlayer1Queue(cocos2d::CCRect const& rect)
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

void HitboxNode::addToPlayer2Queue(CCRect const& rect)
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

void HitboxNode::drawForPlayer1(gd::PlayerObject* obj)
{
	const ccColor4B playersolid = {(GLubyte)(hacks.playerHitboxColor[0] * 255.f),
								   (GLubyte)(hacks.playerHitboxColor[1] * 255.f),
								   (GLubyte)(hacks.playerHitboxColor[2] * 255.f)};
	const ccColor4B playerrotated = {(GLubyte)(hacks.rotatedHitboxColor[0] * 255.f),
									 (GLubyte)(hacks.rotatedHitboxColor[1] * 255.f),
									 (GLubyte)(hacks.rotatedHitboxColor[2] * 255.f)};
	const ccColor4B playercenter = {(GLubyte)(hacks.centerHitboxColor[0] * 255.f),
									(GLubyte)(hacks.centerHitboxColor[1] * 255.f),
									(GLubyte)(hacks.centerHitboxColor[2] * 255.f)};
	if (boo1.size() <= 0)
		this->drawRectangleHitbox(obj->getObjectRect(), playersolid);
	else
	{
		for (size_t i = 0; i < boo1.size(); i++)
		{
			this->drawRectangleHitbox(boo1[i], playersolid);
		}
		for (size_t i = 0; i < boo1.size(); i++)
		{
			this->drawRectangleHitbox(obj->m_vehicleSize >= 1 ? obj->getObjectRect(boo1[i], 0.25, 0.25)
												 : obj->getObjectRect(boo1[i], 0.4, 0.4),
									  playercenter);
		}
	}
	this->drawPolygonHitbox(this->quadrilateralForObject(obj), playerrotated);
	this->drawRectangleHitbox(obj->m_vehicleSize >= 1.f ? obj->getObjectRect(0.25, 0.25) : obj->getObjectRect(0.4, 0.4),
							  playercenter);
}

void HitboxNode::drawForPlayer2(gd::PlayerObject* obj)
{
	const ccColor4B playersolid = {(GLubyte)(hacks.playerHitboxColor[0] * 255.f),
								   (GLubyte)(hacks.playerHitboxColor[1] * 255.f),
								   (GLubyte)(hacks.playerHitboxColor[2] * 255.f)};
	const ccColor4B playerrotated = {(GLubyte)(hacks.rotatedHitboxColor[0] * 255.f),
									 (GLubyte)(hacks.rotatedHitboxColor[1] * 255.f),
									 (GLubyte)(hacks.rotatedHitboxColor[2] * 255.f)};
	const ccColor4B playercenter = {(GLubyte)(hacks.centerHitboxColor[0] * 255.f),
									(GLubyte)(hacks.centerHitboxColor[1] * 255.f),
									(GLubyte)(hacks.centerHitboxColor[2] * 255.f)};
	if (boo2.size() <= 0)
		this->drawRectangleHitbox(obj->getObjectRect(), playersolid);
	else
	{
		for (size_t i = 0; i < boo2.size(); i++)
		{
			this->drawRectangleHitbox(boo2[i], playersolid);
		}
		for (size_t i = 0; i < boo2.size(); i++)
		{
			this->drawRectangleHitbox(obj->m_vehicleSize >= 1.f ? obj->getObjectRect(boo2[i], 0.25, 0.25)
												 : obj->getObjectRect(boo2[i], 0.4, 0.4),
									  playercenter);
		}
	}
	this->drawPolygonHitbox(this->quadrilateralForObject(obj), playerrotated);
	this->drawRectangleHitbox(obj->m_vehicleSize >= 1.f ? obj->getObjectRect(0.25, 0.25) : obj->getObjectRect(0.4, 0.4),
							  playercenter);
} */

/* std::vector<CCPoint> HitboxNode::triangleForObject(gd::GameObject* obj)
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
std::vector<CCPoint> HitboxNode::quadrilateralForObject(gd::GameObject* obj)
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

std::vector<cocos2d::CCPoint> HitboxNode::drawCircleHitbox(cocos2d::CCPoint const& center, float radius,
														   cocos2d::ccColor4B col)
{
	constexpr size_t N = 64;
	constexpr double PI = 3.1415926535;
	std::vector<cocos2d::CCPoint> points(N);
	for (size_t i = 0; i < N; ++i)
	{
		points[i] = center + CCPointMake(std::cos(2 * PI * i / N), std::sin(2 * PI * i / N)) * radius;
	}
	this->drawPolygonHitbox(points, col);
	return points;
}

extern struct HacksStr hacks;

void HitboxNode::drawForObject(gd::GameObject* obj)
{
	if (obj->m_nObjectID != 749 && obj->getType() == gd::GameObjectType::kGameObjectTypeDecoration &&
		!hacks.showDecorations)
		return;
	if (!obj->m_bActive)
		return;

	ccColor4B color;
	switch (obj->getType())
	{
	case gd::GameObjectType::kGameObjectTypeSolid:
		color = {(GLubyte)(hacks.solidHitboxColor[0] * 255.f), (GLubyte)(hacks.solidHitboxColor[1] * 255.f),
				 (GLubyte)(hacks.solidHitboxColor[2] * 255.f)};
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
	case gd::GameObjectType::kGameObjectTypeInverseGravityPortal:
	case gd::GameObjectType::kGameObjectTypeInverseMirrorPortal:
	case gd::GameObjectType::kGameObjectTypeDualPortal:
	case gd::GameObjectType::kGameObjectTypeNormalGravityPortal:
	case gd::GameObjectType::kGameObjectTypeNormalMirrorPortal:
	case gd::GameObjectType::kGameObjectTypeSoloPortal:
	case gd::GameObjectType::kGameObjectTypeRegularSizePortal:
	case gd::GameObjectType::kGameObjectTypeMiniSizePortal:
	case gd::GameObjectType::kGameObjectTypeCubePortal:
	case gd::GameObjectType::kGameObjectTypeShipPortal:
	case gd::GameObjectType::kGameObjectTypeBallPortal:
	case gd::GameObjectType::kGameObjectTypeUfoPortal:
	case gd::GameObjectType::kGameObjectTypeWavePortal:
	case gd::GameObjectType::kGameObjectTypeRobotPortal:
	case gd::GameObjectType::kGameObjectTypeSpiderPortal:
	case gd::GameObjectType::kGameObjectTypeTeleportPortal:
		color = {(GLubyte)(hacks.portalHitboxColor[0] * 255.f), (GLubyte)(hacks.portalHitboxColor[1] * 255.f),
				 (GLubyte)(hacks.portalHitboxColor[2] * 255.f)};

		break;
	case gd::GameObjectType::kGameObjectTypeModifier:
		switch (obj->m_nObjectID)
		{
		case 200:
		case 201:
		case 202:
		case 203:
		case 1334:
			color = {(GLubyte)(hacks.modifierHitboxColor[0] * 255.f), (GLubyte)(hacks.modifierHitboxColor[1] * 255.f),
					 (GLubyte)(hacks.modifierHitboxColor[2] * 255.f)};
			break;
		default:
			if (!hacks.showDecorations)
				return;
		}
		break;
	case gd::GameObjectType::kGameObjectTypeSlope:
		color = {(GLubyte)(hacks.slopeHitboxColor[0] * 255.f), (GLubyte)(hacks.slopeHitboxColor[1] * 255.f),
				 (GLubyte)(hacks.slopeHitboxColor[2] * 255.f)};
		this->drawPolygonHitbox(this->triangleForObject(obj), color);
		return;
	case gd::GameObjectType::kGameObjectTypeHazard:
		color = {(GLubyte)(hacks.hazardHitboxColor[0] * 255.f), (GLubyte)(hacks.hazardHitboxColor[1] * 255.f),
				 (GLubyte)(hacks.hazardHitboxColor[2] * 255.f)};
		break;

	case gd::GameObjectType::kGameObjectTypeYellowJumpPad:
	case gd::GameObjectType::kGameObjectTypePinkJumpPad:
	case gd::GameObjectType::kGameObjectTypeGravityPad:
	case gd::GameObjectType::kGameObjectTypeRedJumpPad:
	case gd::GameObjectType::kGameObjectTypeYellowJumpRing:
		color = {(GLubyte)(hacks.padHitboxColor[0] * 255.f), (GLubyte)(hacks.padHitboxColor[1] * 255.f),
				 (GLubyte)(hacks.padHitboxColor[2] * 255.f)};
		break;

	case gd::GameObjectType::kGameObjectTypePinkJumpRing:
	case gd::GameObjectType::kGameObjectTypeGravityRing:
	case gd::GameObjectType::kGameObjectTypeRedJumpRing:
	case gd::GameObjectType::kGameObjectTypeGreenRing:
	case gd::GameObjectType::kGameObjectTypeDashRing:
	case gd::GameObjectType::kGameObjectTypeGravityDashRing:
		color = {(GLubyte)(hacks.ringHitboxColor[0] * 255.f), (GLubyte)(hacks.ringHitboxColor[1] * 255.f),
				 (GLubyte)(hacks.ringHitboxColor[2] * 255.f)};
		break;
	case gd::GameObjectType::kGameObjectTypeSecretCoin:
	case gd::GameObjectType::kGameObjectTypeUserCoin:
	case gd::GameObjectType::kGameObjectTypeCustomRing:
	case gd::GameObjectType::kGameObjectTypeCollectible:
		color = {(GLubyte)(hacks.collectibleHitboxColor[0] * 255.f), (GLubyte)(hacks.collectibleHitboxColor[1] * 255.f),
				 (GLubyte)(hacks.collectibleHitboxColor[2] * 255.f)};
		break;
	case gd::GameObjectType::kGameObjectTypeDropRing:
		color = ccc4(31, 31, 31, 255);
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

void HitboxNode::drawObjectWithRotation(gd::GameObject* obj)
{
	if (obj->getObjectRadius() > 0)
	{
		drawForObject(obj);
		return;
	}

	std::vector<CCPoint> points = this->quadrilateralForObject(obj);

	CCPoint position = obj->getPosition();
	float angle = obj->getRotation();

	std::vector<CCPoint> rotatedPoints;

	for (auto& point : points)
	{
		CCPoint offset = point - position;

		float rotatedX =
			offset.x * cosf(-CC_DEGREES_TO_RADIANS(angle)) - offset.y * sinf(-CC_DEGREES_TO_RADIANS(angle));
		float rotatedY =
			offset.x * sinf(-CC_DEGREES_TO_RADIANS(angle)) + offset.y * cosf(-CC_DEGREES_TO_RADIANS(angle));

		CCPoint rotatedPoint = position + CCPoint(rotatedX, rotatedY);

		rotatedPoints.push_back(rotatedPoint);
	}

	if (obj->m_nObjectID != 749 && obj->getType() == gd::GameObjectType::kGameObjectTypeDecoration &&
		!hacks.showDecorations)
		return;
	if (!obj->m_bActive)
		return;

	ccColor4B color;
	switch (obj->getType())
	{
	case gd::GameObjectType::kGameObjectTypeSolid:
		color = {(GLubyte)(hacks.solidHitboxColor[0] * 255.f), (GLubyte)(hacks.solidHitboxColor[1] * 255.f),
				 (GLubyte)(hacks.solidHitboxColor[2] * 255.f)};
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
	case gd::GameObjectType::kGameObjectTypeInverseGravityPortal:
	case gd::GameObjectType::kGameObjectTypeInverseMirrorPortal:
	case gd::GameObjectType::kGameObjectTypeDualPortal:
	case gd::GameObjectType::kGameObjectTypeNormalGravityPortal:
	case gd::GameObjectType::kGameObjectTypeNormalMirrorPortal:
	case gd::GameObjectType::kGameObjectTypeSoloPortal:
	case gd::GameObjectType::kGameObjectTypeRegularSizePortal:
	case gd::GameObjectType::kGameObjectTypeMiniSizePortal:
	case gd::GameObjectType::kGameObjectTypeCubePortal:
	case gd::GameObjectType::kGameObjectTypeShipPortal:
	case gd::GameObjectType::kGameObjectTypeBallPortal:
	case gd::GameObjectType::kGameObjectTypeUfoPortal:
	case gd::GameObjectType::kGameObjectTypeWavePortal:
	case gd::GameObjectType::kGameObjectTypeRobotPortal:
	case gd::GameObjectType::kGameObjectTypeSpiderPortal:
	case gd::GameObjectType::kGameObjectTypeTeleportPortal:
		color = {(GLubyte)(hacks.portalHitboxColor[0] * 255.f), (GLubyte)(hacks.portalHitboxColor[1] * 255.f),
				 (GLubyte)(hacks.portalHitboxColor[2] * 255.f)};

		break;
	case gd::GameObjectType::kGameObjectTypeModifier:
		switch (obj->m_nObjectID)
		{
		case 200:
		case 201:
		case 202:
		case 203:
		case 1334:
			color = {(GLubyte)(hacks.modifierHitboxColor[0] * 255.f), (GLubyte)(hacks.modifierHitboxColor[1] * 255.f),
					 (GLubyte)(hacks.modifierHitboxColor[2] * 255.f)};
			break;
		default:
			if (!hacks.showDecorations)
				return;
		}
		break;
	case gd::GameObjectType::kGameObjectTypeSlope:
		color = {(GLubyte)(hacks.slopeHitboxColor[0] * 255.f), (GLubyte)(hacks.slopeHitboxColor[1] * 255.f),
				 (GLubyte)(hacks.slopeHitboxColor[2] * 255.f)};
		this->drawPolygonHitbox(this->triangleForObject(obj), color);
		return;
	case gd::GameObjectType::kGameObjectTypeHazard:
		color = {(GLubyte)(hacks.hazardHitboxColor[0] * 255.f), (GLubyte)(hacks.hazardHitboxColor[1] * 255.f),
				 (GLubyte)(hacks.hazardHitboxColor[2] * 255.f)};
		break;

	case gd::GameObjectType::kGameObjectTypeYellowJumpPad:
	case gd::GameObjectType::kGameObjectTypePinkJumpPad:
	case gd::GameObjectType::kGameObjectTypeGravityPad:
	case gd::GameObjectType::kGameObjectTypeRedJumpPad:
	case gd::GameObjectType::kGameObjectTypeYellowJumpRing:
		color = {(GLubyte)(hacks.padHitboxColor[0] * 255.f), (GLubyte)(hacks.padHitboxColor[1] * 255.f),
				 (GLubyte)(hacks.padHitboxColor[2] * 255.f)};
		break;

	case gd::GameObjectType::kGameObjectTypePinkJumpRing:
	case gd::GameObjectType::kGameObjectTypeGravityRing:
	case gd::GameObjectType::kGameObjectTypeRedJumpRing:
	case gd::GameObjectType::kGameObjectTypeGreenRing:
	case gd::GameObjectType::kGameObjectTypeDashRing:
	case gd::GameObjectType::kGameObjectTypeGravityDashRing:
		color = {(GLubyte)(hacks.ringHitboxColor[0] * 255.f), (GLubyte)(hacks.ringHitboxColor[1] * 255.f),
				 (GLubyte)(hacks.ringHitboxColor[2] * 255.f)};
		break;
	case gd::GameObjectType::kGameObjectTypeSecretCoin:
	case gd::GameObjectType::kGameObjectTypeUserCoin:
	case gd::GameObjectType::kGameObjectTypeCustomRing:
	case gd::GameObjectType::kGameObjectTypeCollectible:
		color = {(GLubyte)(hacks.collectibleHitboxColor[0] * 255.f), (GLubyte)(hacks.collectibleHitboxColor[1] * 255.f),
				 (GLubyte)(hacks.collectibleHitboxColor[2] * 255.f)};
		break;
	case gd::GameObjectType::kGameObjectTypeDropRing:
		color = ccc4(31, 31, 31, 255);
		break;
	default:
		if (!hacks.showDecorations)
			return;
	}

	this->drawPolygonHitbox(rotatedPoints, color);
}
 */