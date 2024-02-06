#pragma once

#include <imgui.h>
#include <Geode/cocos/robtop/keyboard_dispatcher/CCKeyboardDelegate.h>

using namespace cocos2d;

// TODO: no...
static std::set<unsigned long> cheatOpcodes = {
	3041235
};

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

static int ConvertImGuiKeyToEnum(ImGuiKey imguiKey)
{
    switch (imguiKey)
    {
    case ImGuiKey_None:
        return KEY_None;
    case ImGuiKey_Backspace:
        return KEY_Backspace;
    case ImGuiKey_Tab:
        return KEY_Tab;
    case ImGuiKey_Enter:
        return KEY_Enter;
    case ImGuiKey_ModShift:
        return KEY_Shift;
    case ImGuiKey_ModCtrl:
        return KEY_Control;
    case ImGuiKey_ModAlt:
        return KEY_Alt;
    case ImGuiKey_Pause:
        return KEY_Pause;
    case ImGuiKey_CapsLock:
        return KEY_CapsLock;
    case ImGuiKey_Escape:
        return KEY_Escape;
    case ImGuiKey_Space:
        return KEY_Space;
    case ImGuiKey_PageUp:
        return KEY_PageUp;
    case ImGuiKey_PageDown:
        return KEY_PageDown;
    case ImGuiKey_End:
        return KEY_End;
    case ImGuiKey_Home:
        return KEY_Home;
    case ImGuiKey_LeftArrow:
        return KEY_Left;
    case ImGuiKey_UpArrow:
        return KEY_Up;
    case ImGuiKey_RightArrow:
        return KEY_Right;
    case ImGuiKey_DownArrow:
        return KEY_Down;
    case ImGuiKey_PrintScreen:
        return KEY_PrintScreen;
    case ImGuiKey_Insert:
        return KEY_Insert;
    case ImGuiKey_Delete:
        return KEY_Delete;
    case ImGuiKey_0:
        return KEY_Zero;
    case ImGuiKey_1:
        return KEY_One;
    case ImGuiKey_2:
        return KEY_Two;
    case ImGuiKey_3:
        return KEY_Three;
    case ImGuiKey_4:
        return KEY_Four;
    case ImGuiKey_5:
        return KEY_Five;
    case ImGuiKey_6:
        return KEY_Six;
    case ImGuiKey_7:
        return KEY_Seven;
    case ImGuiKey_8:
        return KEY_Eight;
    case ImGuiKey_9:
        return KEY_Nine;
    case ImGuiKey_A:
        return KEY_A;
    case ImGuiKey_B:
        return KEY_B;
    case ImGuiKey_C:
        return KEY_C;
    case ImGuiKey_D:
        return KEY_D;
    case ImGuiKey_E:
        return KEY_E;
    case ImGuiKey_F:
        return KEY_F;
    case ImGuiKey_G:
        return KEY_G;
    case ImGuiKey_H:
        return KEY_H;
    case ImGuiKey_I:
        return KEY_I;
    case ImGuiKey_J:
        return KEY_J;
    case ImGuiKey_K:
        return KEY_K;
    case ImGuiKey_L:
        return KEY_L;
    case ImGuiKey_M:
        return KEY_M;
    case ImGuiKey_N:
        return KEY_N;
    case ImGuiKey_O:
        return KEY_O;
    case ImGuiKey_P:
        return KEY_P;
    case ImGuiKey_Q:
        return KEY_Q;
    case ImGuiKey_R:
        return KEY_R;
    case ImGuiKey_S:
        return KEY_S;
    case ImGuiKey_T:
        return KEY_T;
    case ImGuiKey_U:
        return KEY_U;
    case ImGuiKey_V:
        return KEY_V;
    case ImGuiKey_W:
        return KEY_W;
    case ImGuiKey_X:
        return KEY_X;
    case ImGuiKey_Y:
        return KEY_Y;
    case ImGuiKey_Z:
        return KEY_Z;
    case ImGuiKey_Keypad0:
        return KEY_NumPad0;
    case ImGuiKey_Keypad1:
        return KEY_NumPad1;
    case ImGuiKey_Keypad2:
        return KEY_NumPad2;
    case ImGuiKey_Keypad3:
        return KEY_NumPad3;
    case ImGuiKey_Keypad4:
        return KEY_NumPad4;
    case ImGuiKey_Keypad5:
        return KEY_NumPad5;
    case ImGuiKey_Keypad6:
        return KEY_NumPad6;
    case ImGuiKey_Keypad7:
        return KEY_NumPad7;
    case ImGuiKey_Keypad8:
        return KEY_NumPad8;
    case ImGuiKey_Keypad9:
        return KEY_NumPad9;
    case ImGuiKey_KeypadMultiply:
        return KEY_Multiply;
    case ImGuiKey_KeypadAdd:
        return KEY_Add;
    case ImGuiKey_KeypadEnter:
        return KEY_Seperator;
    case ImGuiKey_KeypadSubtract:
        return KEY_Subtract;
    case ImGuiKey_KeypadDecimal:
        return KEY_Decimal;
    case ImGuiKey_KeypadDivide:
        return KEY_Divide;
    case ImGuiKey_F1:
        return KEY_F1;
    case ImGuiKey_F2:
        return KEY_F2;
    case ImGuiKey_F3:
        return KEY_F3;
    case ImGuiKey_F4:
        return KEY_F4;
    case ImGuiKey_F5:
        return KEY_F5;
    case ImGuiKey_F6:
        return KEY_F6;
    case ImGuiKey_F7:
        return KEY_F7;
    case ImGuiKey_F8:
        return KEY_F8;
    case ImGuiKey_F9:
        return KEY_F9;
    case ImGuiKey_F10:
        return KEY_F10;
    case ImGuiKey_F11:
        return KEY_F11;
    case ImGuiKey_F12:
        return KEY_F12;
    case ImGuiKey_NumLock:
        return KEY_Numlock;
    case ImGuiKey_ScrollLock:
        return KEY_ScrollLock;
    case ImGuiKey_LeftShift:
        return KEY_LeftShift;
    case ImGuiKey_RightShift:
        return KEY_RightShift;
    case ImGuiKey_LeftCtrl:
        return KEY_LeftControl;
    case ImGuiKey_RightCtrl:
        return KEY_RightContol;
    case ImGuiKey_LeftAlt:
        return KEY_LeftMenu;
    case ImGuiKey_RightAlt:
        return KEY_RightMenu;
    default:
        return KEY_None; // Invalid ImGuiKey
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
