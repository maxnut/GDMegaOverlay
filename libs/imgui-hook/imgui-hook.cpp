#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <cocos2d.h>
#include "imgui-hook.hpp"

using namespace cocos2d;

void _stub() {}
std::function<void()> g_drawFunc = _stub;
std::function<void()> g_toggleCallback = _stub;
std::function<void()> g_initFunc = _stub;

size_t g_keybind = VK_TAB;

void ImGuiHook::setRenderFunction(std::function<void()> func) {
    g_drawFunc = func;
}

void ImGuiHook::setToggleCallback(std::function<void()> func) {
    g_toggleCallback = func;
}

void ImGuiHook::setInitFunction(std::function<void()> func) {
    g_initFunc = func;
}

void ImGuiHook::setKeybind(size_t key) {
    g_keybind = key;
}

bool g_inited = false;

void (__thiscall* CCEGLView_swapBuffers)(CCEGLView*);
void __fastcall CCEGLView_swapBuffers_H(CCEGLView* self) {
    auto window = self->getWindow();

    if (!g_inited) {
        g_inited = true;
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO();
        auto hwnd = WindowFromDC(*reinterpret_cast<HDC*>(reinterpret_cast<uintptr_t>(window) + 0x244));
        ImGui_ImplWin32_Init(hwnd);
        ImGui_ImplOpenGL3_Init();
        g_initFunc();
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    g_drawFunc();

    ImGui::EndFrame();
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    glFlush();

    CCEGLView_swapBuffers(self);
}

// why is this an extern
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void (__thiscall* CCEGLView_pollEvents)(CCEGLView*);
void __fastcall CCEGLView_pollEvents_H(CCEGLView* self) {
    auto& io = ImGui::GetIO();

    bool blockInput = false;
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);

        if (io.WantCaptureMouse) {
            switch (msg.message) {
                case WM_LBUTTONDBLCLK:
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                case WM_MBUTTONDBLCLK:
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                case WM_MOUSEACTIVATE:
                case WM_MOUSEHOVER:
                case WM_MOUSEHWHEEL:
                case WM_MOUSELEAVE:
                case WM_MOUSEMOVE:
                case WM_MOUSEWHEEL:
                case WM_NCLBUTTONDBLCLK:
                case WM_NCLBUTTONDOWN:
                case WM_NCLBUTTONUP:
                case WM_NCMBUTTONDBLCLK:
                case WM_NCMBUTTONDOWN:
                case WM_NCMBUTTONUP:
                case WM_NCMOUSEHOVER:
                case WM_NCMOUSELEAVE:
                case WM_NCMOUSEMOVE:
                case WM_NCRBUTTONDBLCLK:
                case WM_NCRBUTTONDOWN:
                case WM_NCRBUTTONUP:
                case WM_NCXBUTTONDBLCLK:
                case WM_NCXBUTTONDOWN:
                case WM_NCXBUTTONUP:
                case WM_RBUTTONDBLCLK:
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                case WM_XBUTTONDBLCLK:
                case WM_XBUTTONDOWN:
                case WM_XBUTTONUP:
                    blockInput = true;
            }
        }

        if (io.WantCaptureKeyboard) {
            switch (msg.message) {
                case WM_HOTKEY:
                case WM_KEYDOWN:
                case WM_KEYUP:
                case WM_KILLFOCUS:
                case WM_SETFOCUS:
                case WM_SYSKEYDOWN:
                case WM_SYSKEYUP:
                    blockInput = true;
            }
        } else if (msg.message == WM_KEYDOWN && msg.wParam == g_keybind) {
            g_toggleCallback();
        }

        if (!blockInput)
            DispatchMessage(&msg);

        ImGui_ImplWin32_WndProcHandler(msg.hwnd, msg.message, msg.wParam, msg.lParam);
    }

    CCEGLView_pollEvents(self);
}

void (__thiscall* CCEGLView_toggleFullScreen)(cocos2d::CCEGLView*, bool);
void __fastcall CCEGLView_toggleFullScreen_H(cocos2d::CCEGLView* self, void*, bool toggle) {
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    g_inited = false;
    
    CCEGLView_toggleFullScreen(self, toggle);
}

void (__thiscall* AppDelegate_applicationWillEnterForeground)(void*);
void __fastcall AppDelegate_applicationWillEnterForeground_H(void* self) {
    AppDelegate_applicationWillEnterForeground(self);
    ImGui::GetIO().ClearInputKeys();
}

void ImGuiHook::setupHooks(std::function<void(void*, void*, void**)> hookFunc) {
    auto cocosBase = GetModuleHandleA("libcocos2d.dll");
    hookFunc(
        GetProcAddress(cocosBase, "?swapBuffers@CCEGLView@cocos2d@@UAEXXZ"),
        CCEGLView_swapBuffers_H,
        reinterpret_cast<void**>(&CCEGLView_swapBuffers)
    );
    hookFunc(
        GetProcAddress(cocosBase, "?pollEvents@CCEGLView@cocos2d@@QAEXXZ"),
        CCEGLView_pollEvents_H,
        reinterpret_cast<void**>(&CCEGLView_pollEvents)
    );
    hookFunc(
        GetProcAddress(cocosBase, "?toggleFullScreen@CCEGLView@cocos2d@@QAEX_N@Z"),
        CCEGLView_toggleFullScreen_H,
        reinterpret_cast<void**>(&CCEGLView_toggleFullScreen)
    );
    hookFunc(
        reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(GetModuleHandleA(0)) + 0x3d130),
        reinterpret_cast<void*>(&AppDelegate_applicationWillEnterForeground_H),
        reinterpret_cast<void**>(&AppDelegate_applicationWillEnterForeground)
    );
}