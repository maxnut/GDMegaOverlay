#define WIN32_LEAN_AND_MEAN
#include <sstream>
#include <string_view>
#include <filesystem>
#include "explorerUtils.hpp"
#include "explorer.hpp"
#include "Hacks.h"
#include "json.hpp"

using namespace cocos2d;
using json = nlohmann::json;

json j;
size_t index = 0;
std::vector<int> ids;
const char *get_node_name(CCNode *node)
{
	// works because msvc's typeid().name() returns undecorated name
	// typeid(CCNode).name() == "class cocos2d::CCNode"
	// the + 6 gets rid of the class prefix
	// "class cocos2d::CCNode" + 6 == "cocos2d::CCNode"
	return typeid(*node).name() + 6;
}

static CCNode *selected_node = nullptr;
static bool reached_selected_node = false;
static CCNode *hovered_node = nullptr;

void render_node_tree(CCNode *node, unsigned int index = 0)
{
	std::stringstream stream;
	stream << "[" << index << "] " << get_node_name(node);
	if (node->getTag() != -1)
		stream << " (" << node->getTag() << ")";
	const auto children_count = node->getChildrenCount();
	if (children_count)
		stream << " {" << children_count << "}";

	auto flags = ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_SpanFullWidth;
	if (selected_node == node)
	{
		flags |= ImGuiTreeNodeFlags_Selected;
		reached_selected_node = true;
	}
	if (node->getChildrenCount() == 0)
		flags |= ImGuiTreeNodeFlags_Leaf;

	ImGui::PushStyleColor(ImGuiCol_Text, node->isVisible() ? ImVec4{1.f, 1.f, 1.f, 1.f} : ImVec4{0.8f, 0.8f, 0.8f, 1.f});
	const bool is_open = ImGui::TreeNodeEx(node, flags, stream.str().c_str());
	if (ImGui::IsItemClicked())
	{
		if (node == selected_node && ImGui::GetIO().KeyAlt)
		{
			selected_node = nullptr;
			reached_selected_node = false;
		}
		else
		{
			selected_node = node;
			reached_selected_node = true;
		}
	}
	if (ImGui::IsItemHovered())
		hovered_node = node;
	if (is_open)
	{
		auto children = node->getChildren();
		for (unsigned int i = 0; i < children_count; ++i)
		{
			auto child = children->objectAtIndex(i);
			render_node_tree(static_cast<CCNode *>(child), i);
		}
		ImGui::TreePop();
	}
	ImGui::PopStyleColor();
}

#define CONCAT_(a, b) a##b
#define CONCAT(a, b) CONCAT_(a, b)

void render_node_properties(CCNode *node)
{
	if (ImGui::Button("Delete"))
	{
		node->removeFromParentAndCleanup(true);
		return;
	}
	ImGui::Text("Addr: 0x%p", node);
	ImGui::SameLine();
	if (ImGui::Button("Copy##copyaddr"))
	{
		std::stringstream stream;
		stream << std::uppercase << std::hex << reinterpret_cast<uintptr_t>(node);
		set_clipboard_text(stream.str());
	}
	if (node->getUserData())
		ImGui::Text("User data: 0x%p", node->getUserData());

	if (node->getTag())
		ImGui::Text(("Tag: " + std::to_string(node->getTag())).c_str());

#define GET_SET_FLOAT2(name, label)                                      \
	{                                                                    \
		auto point = node->get##name();                                  \
		if (ImGui::DragFloat2(label, reinterpret_cast<float *>(&point))) \
			node->set##name(point);                                      \
	}

#define GET_SET_INT(name, label)            \
	{                                       \
		auto value = node->get##name();     \
		if (ImGui::InputInt(label, &value)) \
			node->set##name(value);         \
	}

#define GET_SET_CHECKBOX(name, label)       \
	{                                       \
		auto value = node->is##name();      \
		if (ImGui::Checkbox(label, &value)) \
			node->set##name(value);         \
	}

	GET_SET_FLOAT2(Position, "Position");

#define dragXYBothIDK(name, label, speed)                                                                                  \
	{                                                                                                                      \
		float values[3] = {node->get##name(), node->CONCAT(get, CONCAT(name, X))(), node->CONCAT(get, CONCAT(name, Y))()}; \
		if (ImGui::DragFloat3(label, values, speed))                                                                       \
		{                                                                                                                  \
			if (node->get##name() != values[0])                                                                            \
				node->set##name(values[0]);                                                                                \
			else                                                                                                           \
			{                                                                                                              \
				node->CONCAT(set, CONCAT(name, X))(values[1]);                                                             \
				node->CONCAT(set, CONCAT(name, Y))(values[2]);                                                             \
			}                                                                                                              \
		}                                                                                                                  \
	}
	dragXYBothIDK(Scale, "Scale", 0.025f);
	dragXYBothIDK(Rotation, "Rotation", 1.0f);

#undef dragXYBothIDK

	{
		auto anchor = node->getAnchorPoint();
		if (ImGui::DragFloat2("Anchor Point", &anchor.x, 0.05f, 0.f, 1.f))
			node->setAnchorPoint(anchor);
	}

	GET_SET_FLOAT2(ContentSize, "Content Size");
	GET_SET_INT(ZOrder, "Z Order");
	GET_SET_CHECKBOX(Visible, "Visible");

	if (auto rgba_node = dynamic_cast<CCNodeRGBA *>(node); rgba_node)
	{
		auto color = rgba_node->getColor();
		float colorValues[4] = {
			color.r / 255.f,
			color.g / 255.f,
			color.b / 255.f,
			rgba_node->getOpacity() / 255.f};
		if (ImGui::ColorEdit4("Color", colorValues))
		{
			rgba_node->setColor({static_cast<GLubyte>(colorValues[0] * 255),
								 static_cast<GLubyte>(colorValues[1] * 255),
								 static_cast<GLubyte>(colorValues[2] * 255)});
			rgba_node->setOpacity(static_cast<GLubyte>(colorValues[3] * 255.f));
		}
	}

	if (auto label_node = dynamic_cast<CCLabelProtocol *>(node); label_node)
	{
		std::string str = label_node->getString();
		char *test = (char *)str.c_str();
		if (ImGui::InputTextMultiline("Text", test, sizeof(test) / sizeof(char), {0, 50}))
			label_node->setString(str.c_str());
	}

	if (auto sprite_node = dynamic_cast<CCSprite *>(node); sprite_node)
	{
		auto *texture = sprite_node->getTexture();

		auto *texture_cache = CCTextureCache::sharedTextureCache();
		auto *cached_textures = public_cast(texture_cache, m_pTextures);
		CCDictElement *el;
		CCDICT_FOREACH(cached_textures, el)
		{
			if (el->getObject() == texture)
			{
				ImGui::TextWrapped("Texture name: %s", el->getStrKey());
				break;
			}
		}

		auto *frame_cache = CCSpriteFrameCache::sharedSpriteFrameCache();
		auto *cached_frames = public_cast(frame_cache, m_pSpriteFrames);
		const auto rect = sprite_node->getTextureRect();
		CCDICT_FOREACH(cached_frames, el)
		{
			auto *frame = static_cast<CCSpriteFrame *>(el->getObject());
			if (frame->getTexture() == texture && frame->getRect() == rect)
			{
				ImGui::Text("Frame name: %s", el->getStrKey());
				ImGui::SetClipboardText(el->getStrKey());
				break;
			}
		}
	}

	if (auto menu_item_node = dynamic_cast<CCMenuItem *>(node); menu_item_node)
	{
		const auto selector = public_cast(menu_item_node, m_pfnSelector);
		const auto addr = format_addr(union_cast<void *>(selector));
		ImGui::Text("CCMenuItem selector: %s", addr.c_str());
		ImGui::SameLine();
		if (ImGui::Button("Copy##copyselector"))
		{
			set_clipboard_text(addr);
		}
	}
}

void render_node_highlight(CCNode *node, bool selected)
{
	auto &foreground = *ImGui::GetForegroundDrawList();
	auto parent = node->getParent();
	auto bounding_box = node->boundingBox();
	CCPoint bb_min(bounding_box.getMinX(), bounding_box.getMinY());
	CCPoint bb_max(bounding_box.getMaxX(), bounding_box.getMaxY());

	auto camera_parent = node;
	while (camera_parent)
	{
		auto camera = camera_parent->getCamera();

		float off_x, off_y, off_z;
		camera->getEyeXYZ(&off_x, &off_y, &off_z);
		const CCPoint offset(off_x, off_y);
		bb_min -= offset;
		bb_max -= offset;

		camera_parent = camera_parent->getParent();
	}

	auto min = cocos_to_vec2(parent ? parent->convertToWorldSpace(bb_min) : bb_min);
	auto max = cocos_to_vec2(parent ? parent->convertToWorldSpace(bb_max) : bb_max);
	foreground.AddRectFilled(min, max, selected ? IM_COL32(200, 200, 255, 60) : IM_COL32(255, 255, 255, 70));
}

void CocosExplorer::draw()
{
	static bool highlight = false;
	hovered_node = nullptr;
	if (ImGui::BeginMenuBar())
	{
		ImGui::MenuItem("Highlight", nullptr, &highlight);
		ImGui::EndMenuBar();
	}

	const auto avail = ImGui::GetContentRegionAvail();

	ImGui::BeginChild("explorer.tree", ImVec2(avail.x * 0.5f, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

	auto node = CCDirector::sharedDirector()->getRunningScene();
	reached_selected_node = false;
	render_node_tree(node);

	ImGui::EndChild();

	if (!reached_selected_node)
		selected_node = nullptr;

	ImGui::SameLine();

	ImGui::BeginChild("explorer.options");

	if (selected_node)
		render_node_properties(selected_node);
	else
		ImGui::Text("Select a node to edit its properties :-)");

	if (ImGui::Button("Out Textures"))
	{
		ids.clear();
		index = 0;
		j.clear();
		j = json::array();
		GenerateJsonTextures(gd::GameManager::sharedState()->getPlayLayer()->getAllObjects(), false);
		Hacks::writeOutput(j.dump());
	}

	ImGui::EndChild();

	if (highlight && (selected_node || hovered_node))
	{
		if (selected_node)
			render_node_highlight(selected_node, true);
		if (hovered_node)
			render_node_highlight(hovered_node, false);
	}
}

void CocosExplorer::GenerateJsonTextures(cocos2d::CCArray *children, bool isAGObject)
{
	bool is = false;
	CCNode *obj;
	for (unsigned int i = 0; i < children->count(); ++i)
	{
		auto child = children->objectAtIndex(i);
		obj = static_cast<CCNode *>(child);
		if (auto sprite_node = dynamic_cast<CCSprite *>(obj); sprite_node)
		{
			if (auto gob = static_cast<gd::GameObject *>(sprite_node); gob && (gob->m_nObjectID > 0 && gob->m_nObjectID < 1911 || isAGObject) && !(gob->m_nObjectID == 8 && gob->getPositionX() == 0 && gob->getPositionY() == 105))
			{
				is = true;
				auto *texture = sprite_node->getTexture();
				CCDictElement *el;

				auto *frame_cache = CCSpriteFrameCache::sharedSpriteFrameCache();
				auto *cached_frames = public_cast(frame_cache, m_pSpriteFrames);
				const auto rect = sprite_node->getTextureRect();
				CCDICT_FOREACH(cached_frames, el)
				{
					auto *frame = static_cast<CCSpriteFrame *>(el->getObject());
					if (frame->getTexture() == texture && frame->getRect() == rect)
					{
						if(!isAGObject)
						{
							if (std::count(ids.begin(), ids.end(), gob->m_nObjectID)) break;
							auto objet = json::object();
							ids.push_back(gob->m_nObjectID);
							objet["texture"] = el->getStrKey();
							objet["id"] = gob->m_nObjectID;
							objet["child_count"] = gob->getChildrenCount();
							j.push_back(objet);
							index++;
						}
						else if(std::string(el->getStrKey()) != "square_02_001.png")
						{
							auto objet = json::object();
							objet["texture"] = el->getStrKey();
							objet["x"] = gob->getPositionX();
							objet["y"] = gob->getPositionY();
							objet["z"] = gob->getZOrder();
							objet["rx"] = gob->getRotationX();
							objet["ry"] = gob->getRotationY();
							objet["sx"] = gob->getScaleX();
							objet["sy"] = gob->getScaleY();
							j[index - 1]["children"].push_back(objet);
						}
						break;
					}
				}
			}
		}
		if (obj->getChildrenCount() > 0)
			GenerateJsonTextures(obj->getChildren(), is);
	}
}