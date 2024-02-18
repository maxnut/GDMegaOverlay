#include "Blur.h"
#include "GUI.h"
#include "Settings.hpp"

#include <Geode/modify/CCDirector.hpp>

using namespace geode::prelude;
using namespace Blur;

class $modify(CCDirector) {
    void drawScene() {

		bool blur = Settings::get<bool>("menu/blur/enabled", false);

		if(!GUI::isVisible && !blur)
		{
			CCDirector::drawScene();
			return;
		}

		CCDirector::drawScene();

		auto winSize = this->getOpenGLView()->getViewPortRect();
		if(!gdRenderTexture)
			gdRenderTexture = RenderTexture::create({winSize.size.width, winSize.size.height});

		gdRenderTexture->bind();
		gdRenderTexture->clear();
		this->getRunningScene()->visit();
       	gdRenderTexture->unbind();

	}
};

const char* const vertexShaderCode = R"(													
attribute vec4 a_position;							
attribute vec2 a_texCoord;							
attribute vec4 a_color;								
													
#ifdef GL_ES										
varying lowp vec4 v_fragmentColor;					
varying mediump vec2 v_texCoord;					
#else												
varying vec4 v_fragmentColor;						
varying vec2 v_texCoord;							
#endif												
													
void main()											
{													
    gl_Position = CC_MVPMatrix * a_position;		
	v_fragmentColor = a_color;						
	v_texCoord = a_texCoord;						
}													
)";

const char* const fragmentShaderCode = R"(
#ifdef GL_ES								
precision lowp float;						
#endif										
																					
varying vec4 v_fragmentColor;				
varying vec2 v_texCoord;					
uniform sampler2D CC_Texture0;
uniform float blurDarkness;			
											
void main()									
{
	float blurSize = 0.001;			
	int radius = 12;
	float pi = 3.1415926;
	float sigma = 5.;

	vec4 sum = vec4(0.);

	for(int x = -radius; x <= radius; x++){
        for(int y = -radius; y <= radius; y++){
            vec2 newUV = v_texCoord + vec2(float(x) * blurSize, float(y) * blurSize);
            sum += texture(CC_Texture0, newUV) * (exp(-(pow(float(x), 2.) + pow(float(y), 2.)) / (2. * pow(sigma, 2.))) / (2. * pi * pow(sigma, 2.)));
        }   
    }

	//sum *= vec4(0.1, 0.1, 0.1, 1);

	sum.a = 1.1f - blurDarkness;			
											
    gl_FragColor = sum;
})";

void Blur::compileBlurShader()
{
    blurProgram = new cocos2d::CCGLProgram();
	blurProgram->retain();
	blurProgram->initWithVertexShaderByteArray(vertexShaderCode, fragmentShaderCode);
	blurProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
    blurProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
    blurProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
	blurProgram->link();
	blurProgram->updateUniforms();
}

GLint oldTexture = -1;

void Blur::blurWindowBackground()
{
    ImVec2 screen_size = ImGui::GetIO().DisplaySize;
	ImVec2 window_pos = ImGui::GetWindowPos();
	
	//dont blur window titles
	window_pos.y += screen_size.y / 45.f;
    ImVec2 window_size = ImGui::GetWindowSize();
    ImVec2 rect_min = ImVec2(window_pos.x, window_pos.y);
    ImVec2 rect_max = ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y);
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	draw_list->PushClipRectFullScreen();
	draw_list->AddCallback([](const ImDrawList*, const ImDrawCmd*)
		{
			glActiveTexture(GL_TEXTURE0);
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);
			blurProgram->use();
			glBindTexture(GL_TEXTURE_2D, gdRenderTexture->getTexture());
			blurProgram->setUniformsForBuiltins();

			float blurDarkness = Settings::get<float>("menu/blur/darkness", 1.f);
			blurProgram->setUniformLocationWith1f(blurProgram->getUniformLocationForName("blurDarkness"), blurDarkness);
		}, nullptr);

	//add rect filled doesnt have tex coords so i add them manually
	float l = window_pos.x / screen_size.x;
	float r = (window_pos.x + window_size.x) / screen_size.x;
	float t = window_pos.y / screen_size.y;
	t = fabs(t - 1);
	float b = (window_pos.y + window_size.y) / screen_size.y;
	b = fabs(b - 1);
	
	draw_list->AddRectFilled(rect_min, rect_max, IM_COL32_BLACK);
	draw_list->VtxBuffer[draw_list->VtxBuffer.size() - 4].uv = {l, t};
	draw_list->VtxBuffer[draw_list->VtxBuffer.size() - 3].uv = {r, t};
	draw_list->VtxBuffer[draw_list->VtxBuffer.size() - 2].uv = {r, b};
	draw_list->VtxBuffer[draw_list->VtxBuffer.size() - 1].uv = {l, b};
	
	draw_list->AddCallback([](const ImDrawList*, const ImDrawCmd*)
	{
		glBindTexture(GL_TEXTURE_2D, oldTexture);
		auto* shader = CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor);
		shader->use();
		shader->setUniformsForBuiltins();
	}, nullptr);

	draw_list->PopClipRect();
}