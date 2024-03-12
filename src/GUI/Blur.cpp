#include "Blur.h"
#include "GUI.h"
#include "Settings.hpp"

#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/CCNode.hpp>

using namespace geode::prelude;
using namespace Blur;

struct Vertex {
	float position[3];  // 12 bytes
	unsigned char color[4];	 // 4 bytes
	float texCoords[2];  // 8 bytes
};

GLuint VAO, VBO;

class $modify(CCEGLView) {

	void toggleFullScreen(bool value, bool borderless) {
		gdRenderTexture = nullptr;
		CCEGLView::toggleFullScreen(value, borderless);
	}

	void resizeWindow(int width, int height)
	{
		CCEGLView::resizeWindow(width, height);

		if (gdRenderTexture)
			gdRenderTexture->resize({(float)width, (float)height});
	}
};

class $modify(CCNode) {
	void visit() {

		bool blur = Settings::get<bool>("menu/blur/enabled", false) || Settings::get<bool>("menu/blur/gd", false);

		if (this != (CCNode*)CCDirector::sharedDirector()->getRunningScene() || !GUI::shouldRender() || !blur || !blurProgram)
		{
			CCNode::visit();
			return;
		}

		if (!compiled)
			Blur::compileBlurShader();

		if (!gdRenderTexture)
		{
			auto winSize = CCDirector::sharedDirector()->getOpenGLView()->getViewPortRect();
			gdRenderTexture = RenderTexture::create({winSize.size.width, winSize.size.height});
		}

		gdRenderTexture->bind();
		gdRenderTexture->clear();
		CCNode::visit();
		gdRenderTexture->unbind();

		bool blurGD = Settings::get<bool>("menu/blur/gd", false);

		glActiveTexture(GL_TEXTURE0);
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture); 
		glBindTexture(GL_TEXTURE_2D, gdRenderTexture->getTexture());

		if (blurGD)
		{
			blurProgram->use();
			setBlurUniforms();
		}
		else
		{
			auto* shader = CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor);
			shader->use();
			shader->setUniformsForBuiltins();
		}

		glBindVertexArray(VAO);
		glDrawArrays(GL_QUADS, 0, 4);
		glBindVertexArray(0);

		glBindTexture(GL_TEXTURE_2D, oldTexture);
		auto *shader = CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor);
		shader->use();
		shader->setUniformsForBuiltins();
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
uniform float blurSize;		
uniform float blurAmount;		
uniform int blurSteps;	
											
void main()									
{
	float pi = 3.1415926;
	float sigma = 5.;

	vec4 sum = vec4(0.);

	float scaledBlurSize = blurAmount * blurSize * 0.001;

	for(int x = -blurSteps; x <= blurSteps; x++){
		for(int y = -blurSteps; y <= blurSteps; y++){
			vec2 newUV = v_texCoord + vec2(float(x) * scaledBlurSize, float(y) * scaledBlurSize);
			sum += texture(CC_Texture0, newUV) * (exp(-(pow(float(x), 2.) + pow(float(y), 2.)) / (2. * pow(sigma, 2.))) / (2. * pi * pow(sigma, 2.)));
		}   
	}

	//sum *= vec4(0.1, 0.1, 0.1, 1);

	sum.a = 1.1f - blurDarkness;			
											
	gl_FragColor = sum;
})";

void Blur::compileBlurShader()
{
	bool blur = Settings::get<bool>("menu/blur/enabled", false) || Settings::get<bool>("menu/blur/gd", false);

	if (compiled || !blur)
		return;

	blurProgram = new cocos2d::CCGLProgram();
	blurProgram->initWithVertexShaderByteArray(vertexShaderCode, fragmentShaderCode);
	blurProgram->addAttribute(kCCAttributeNamePosition, kCCVertexAttrib_Position);
	blurProgram->addAttribute(kCCAttributeNameColor, kCCVertexAttrib_Color);
	blurProgram->addAttribute(kCCAttributeNameTexCoord, kCCVertexAttrib_TexCoords);
	blurProgram->link();
	blurProgram->updateUniforms();

	auto screenSize = CCDirector::sharedDirector()->getWinSize();

	Vertex vertices[] = {
		{ 0.f, 0, 0.0f, 255, 255, 255, 255, 0.0f, 0.0f },
		{ screenSize.width + 0.1f, 0.f, 0.0f, 255, 255, 255, 255, 1.0f, 0.0f },
		{ screenSize.width + 0.1f, screenSize.height, 0.0f, 255, 255, 255, 255, 1.0f, 1.0f },
		{ 0, screenSize.height, 0.0f, 255, 255, 255, 255, 0.0f, 1.0f }
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (GLvoid*)12);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)16);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	compiled = true;
}

void Blur::setBlurUniforms()
{
	blurProgram->setUniformsForBuiltins();
	float blurDarkness = Settings::get<float>("menu/blur/darkness", 1.f);
	float blurSize = Settings::get<float>("menu/blur/size", 1.f);
	int blurSteps = Settings::get<int>("menu/blur/steps", 10);

	if (darknessUniform == -1)
		darknessUniform = blurProgram->getUniformLocationForName("blurDarkness");
	else
		blurProgram->setUniformLocationWith1f(darknessUniform, blurDarkness);

	if (stepsUniform == -1)
		stepsUniform = blurProgram->getUniformLocationForName("blurSteps");
	else
		blurProgram->setUniformLocationWith1i(stepsUniform, blurSteps);

	if (sizeUniform == -1)
		sizeUniform = blurProgram->getUniformLocationForName("blurSize");
	else
		blurProgram->setUniformLocationWith1f(sizeUniform, blurSize);

	if (amountUniform == -1)
		amountUniform = blurProgram->getUniformLocationForName("blurAmount");
	else
		blurProgram->setUniformLocationWith1f(amountUniform, blurAmount);
}

void Blur::blurCallback(const ImDrawList*, const ImDrawCmd*)
{
	glActiveTexture(GL_TEXTURE0);
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);
	blurProgram->use();
	glBindTexture(GL_TEXTURE_2D, gdRenderTexture->getTexture());
	setBlurUniforms();
}

void Blur::resetCallback(const ImDrawList*, const ImDrawCmd*)
{
	glBindTexture(GL_TEXTURE_2D, oldTexture);
	auto* shader = CCShaderCache::sharedShaderCache()->programForKey(kCCShader_PositionTextureColor);
	shader->use();
	shader->setUniformsForBuiltins();
}

void Blur::blurWindowBackground()
{
	if (!blurProgram || !gdRenderTexture)
		return;
	
	ImVec2 screen_size = ImGui::GetIO().DisplaySize;
	ImVec2 window_pos = ImGui::GetWindowPos();
	
	//dont blur window titles
	window_pos.y += 24.f;
	ImVec2 window_size = ImGui::GetWindowSize();
	ImVec2 rect_min = ImVec2(window_pos.x, window_pos.y);
	ImVec2 rect_max = ImVec2(window_pos.x + window_size.x, window_pos.y + window_size.y);
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	draw_list->PushClipRectFullScreen();

	draw_list->AddCallback(blurCallback, nullptr);

	//add rect filled doesnt have tex coords so i add them manually
	float l = window_pos.x / screen_size.x;
	float r = (window_pos.x + window_size.x) / screen_size.x;
	float t = window_pos.y / screen_size.y;
	t = 1 - t;
	float b = (window_pos.y + window_size.y) / screen_size.y;
	b = 1 - b;
	
	draw_list->AddRectFilled(rect_min, rect_max, IM_COL32_BLACK);
	draw_list->VtxBuffer[draw_list->VtxBuffer.size() - 4].uv = {l, t};
	draw_list->VtxBuffer[draw_list->VtxBuffer.size() - 3].uv = {r, t};
	draw_list->VtxBuffer[draw_list->VtxBuffer.size() - 2].uv = {r, b};
	draw_list->VtxBuffer[draw_list->VtxBuffer.size() - 1].uv = {l, b};
	
	draw_list->AddCallback(resetCallback, nullptr);

	draw_list->PopClipRect();
}