#pragma once

#include <imgui.h>
#include <Geode/cocos/robtop/keyboard_dispatcher/CCKeyboardDelegate.h>

using namespace cocos2d;

static ImGuiKey ConvertKeyEnum(int key)
{
	switch (key)
	{
	case KEY_None:
		return ImGuiKey_None;
	case KEY_Backspace:
		return ImGuiKey_Backspace;
	case KEY_Tab:
		return ImGuiKey_Tab;
	case KEY_Clear:
		return ImGuiKey_None; // No equivalent key
	case KEY_Enter:
		return ImGuiKey_Enter;
	case KEY_Shift:
		return ImGuiKey_ModShift;
	case KEY_Control:
		return ImGuiKey_ModCtrl;
	case KEY_Alt:
		return ImGuiKey_ModAlt;
	case KEY_Pause:
		return ImGuiKey_Pause;
	case KEY_CapsLock:
		return ImGuiKey_CapsLock;
	case KEY_Escape:
		return ImGuiKey_Escape;
	case KEY_Space:
		return ImGuiKey_Space;
	case KEY_PageUp:
		return ImGuiKey_PageUp;
	case KEY_PageDown:
		return ImGuiKey_PageDown;
	case KEY_End:
		return ImGuiKey_End;
	case KEY_Home:
		return ImGuiKey_Home;
	case KEY_Left:
		return ImGuiKey_LeftArrow;
	case KEY_Up:
		return ImGuiKey_UpArrow;
	case KEY_Right:
		return ImGuiKey_RightArrow;
	case KEY_Down:
		return ImGuiKey_DownArrow;
	case KEY_Select:
		return ImGuiKey_None; // No equivalent key
	case KEY_Print:
		return ImGuiKey_PrintScreen;
	case KEY_Execute:
		return ImGuiKey_None; // No equivalent key
	case KEY_PrintScreen:
		return ImGuiKey_PrintScreen;
	case KEY_Insert:
		return ImGuiKey_Insert;
	case KEY_Delete:
		return ImGuiKey_Delete;
	case KEY_Help:
		return ImGuiKey_None; // No equivalent key
	case KEY_Zero:
		return ImGuiKey_0;
	case KEY_One:
		return ImGuiKey_1;
	case KEY_Two:
		return ImGuiKey_2;
	case KEY_Three:
		return ImGuiKey_3;
	case KEY_Four:
		return ImGuiKey_4;
	case KEY_Five:
		return ImGuiKey_5;
	case KEY_Six:
		return ImGuiKey_6;
	case KEY_Seven:
		return ImGuiKey_7;
	case KEY_Eight:
		return ImGuiKey_8;
	case KEY_Nine:
		return ImGuiKey_9;
	case KEY_A:
		return ImGuiKey_A;
	case KEY_B:
		return ImGuiKey_B;
	case KEY_C:
		return ImGuiKey_C;
	case KEY_D:
		return ImGuiKey_D;
	case KEY_E:
		return ImGuiKey_E;
	case KEY_F:
		return ImGuiKey_F;
	case KEY_G:
		return ImGuiKey_G;
	case KEY_H:
		return ImGuiKey_H;
	case KEY_I:
		return ImGuiKey_I;
	case KEY_J:
		return ImGuiKey_J;
	case KEY_K:
		return ImGuiKey_K;
	case KEY_L:
		return ImGuiKey_L;
	case KEY_M:
		return ImGuiKey_M;
	case KEY_N:
		return ImGuiKey_N;
	case KEY_O:
		return ImGuiKey_O;
	case KEY_P:
		return ImGuiKey_P;
	case KEY_Q:
		return ImGuiKey_Q;
	case KEY_R:
		return ImGuiKey_R;
	case KEY_S:
		return ImGuiKey_S;
	case KEY_T:
		return ImGuiKey_T;
	case KEY_U:
		return ImGuiKey_U;
	case KEY_V:
		return ImGuiKey_V;
	case KEY_W:
		return ImGuiKey_W;
	case KEY_X:
		return ImGuiKey_X;
	case KEY_Y:
		return ImGuiKey_Y;
	case KEY_Z:
		return ImGuiKey_Z;
	case KEY_LeftWindowsKey:
		return ImGuiKey_None; // No equivalent key
	case KEY_RightWindowsKey:
		return ImGuiKey_None; // No equivalent key
	case KEY_ApplicationsKey:
		return ImGuiKey_None; // No equivalent key
	case KEY_NumPad0:
		return ImGuiKey_Keypad0;
	case KEY_NumPad1:
		return ImGuiKey_Keypad1;
	case KEY_NumPad2:
		return ImGuiKey_Keypad2;
	case KEY_NumPad3:
		return ImGuiKey_Keypad3;
	case KEY_NumPad4:
		return ImGuiKey_Keypad4;
	case KEY_NumPad5:
		return ImGuiKey_Keypad5;
	case KEY_NumPad6:
		return ImGuiKey_Keypad6;
	case KEY_NumPad7:
		return ImGuiKey_Keypad7;
	case KEY_NumPad8:
		return ImGuiKey_Keypad8;
	case KEY_NumPad9:
		return ImGuiKey_Keypad9;
	case KEY_Multiply:
		return ImGuiKey_KeypadMultiply;
	case KEY_Add:
		return ImGuiKey_KeypadAdd;
	case KEY_Seperator:
		return ImGuiKey_KeypadEnter;
	case KEY_Subtract:
		return ImGuiKey_KeypadSubtract;
	case KEY_Decimal:
		return ImGuiKey_KeypadDecimal;
	case KEY_Divide:
		return ImGuiKey_KeypadDivide;
	case KEY_F1:
		return ImGuiKey_F1;
	case KEY_F2:
		return ImGuiKey_F2;
	case KEY_F3:
		return ImGuiKey_F3;
	case KEY_F4:
		return ImGuiKey_F4;
	case KEY_F5:
		return ImGuiKey_F5;
	case KEY_F6:
		return ImGuiKey_F6;
	case KEY_F7:
		return ImGuiKey_F7;
	case KEY_F8:
		return ImGuiKey_F8;
	case KEY_F9:
		return ImGuiKey_F9;
	case KEY_F10:
		return ImGuiKey_F10;
	case KEY_F11:
		return ImGuiKey_F11;
	case KEY_F12:
		return ImGuiKey_F12;
	case KEY_Numlock:
		return ImGuiKey_NumLock;
	case KEY_ScrollLock:
		return ImGuiKey_ScrollLock;
	case KEY_LeftShift:
		return ImGuiKey_LeftShift;
	case KEY_RightShift:
		return ImGuiKey_RightShift;
	case KEY_LeftControl:
		return ImGuiKey_LeftCtrl;
	case KEY_RightContol:
		return ImGuiKey_RightCtrl;
	case KEY_LeftMenu:
		return ImGuiKey_LeftAlt;
	case KEY_RightMenu:
		return ImGuiKey_RightAlt;
	default:
		return ImGuiKey_COUNT; // Invalid key
	}
}

const char* const KeyNames[] = {
    "None",
    "Tab",
    "LeftArrow",
    "RightArrow",
    "UpArrow",
    "DownArrow",
    "PageUp",
    "PageDown",
    "Home",
    "End",
    "Insert",
    "Delete",
    "Backspace",
    "Space",
    "Enter",
    "Escape",
    "LeftCtrl",
    "LeftShift",
    "LeftAlt",
    "LeftSuper",
    "RightCtrl",
    "RightShift",
    "RightAlt",
    "RightSuper",
    "Menu",
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J",
    "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T",
    "U", "V", "W", "X", "Y", "Z",
    "F1", "F2", "F3", "F4", "F5", "F6",
    "F7", "F8", "F9", "F10", "F11", "F12",
    "Apostrophe",
    "Comma",
    "Minus",
    "Period",
    "Slash",
    "Semicolon",
    "Equal",
    "LeftBracket",
    "Backslash",
    "RightBracket",
    "GraveAccent",
    "CapsLock",
    "ScrollLock",
    "NumLock",
    "PrintScreen",
    "Pause",
    "Keypad0", "Keypad1", "Keypad2", "Keypad3", "Keypad4",
    "Keypad5", "Keypad6", "Keypad7", "Keypad8", "Keypad9",
    "KeypadDecimal",
    "KeypadDivide",
    "KeypadMultiply",
    "KeypadSubtract",
    "KeypadAdd",
    "KeypadEnter",
    "KeypadEqual"
};

const char* const priorities[] = { "Low", "Below Normal", "Normal", "Above Normal", "High" };

const char* const correctionType[] = { "None", "Every Action" };

const char* const positions[] = { "Top Left", "Top Right", "Bottom Left", "Bottom Right" };
