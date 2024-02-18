#pragma once

#include <iostream>
#include <imgui.h>

class RenderTexture
{
  private:
	GLuint framebuffer = 0;
	GLuint renderbuffer = 0;
	GLuint textureID = 0;

  public:
    ImVec2 resolution = {0, 0};

  public:

    static std::shared_ptr<RenderTexture> create(ImVec2 resolution);
    bool init(ImVec2 resolution);

    void resize(ImVec2 resolution);

    void bind();
    void unbind();

    void clear();

	GLuint getTexture()
	{
		return textureID;
	}
};