#pragma once

#include <imgui.h>
#include <cocos2d.h>
#include "RenderTexture.h"

namespace Blur
{
    inline cocos2d::CCGLProgram* blurProgram;
	inline std::shared_ptr<RenderTexture> gdRenderTexture;

    void compileBlurShader();

    void blurWindowBackground();
};