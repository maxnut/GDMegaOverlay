#include "RenderTexture.h"

std::shared_ptr<RenderTexture> RenderTexture::create(ImVec2 resolution)
{
	std::shared_ptr<RenderTexture> ptr = std::make_shared<RenderTexture>();

	if (ptr->init(resolution))
		return ptr;

	return nullptr;
}

bool RenderTexture::init(ImVec2 resolution)
{
	glGenFramebuffers(1, &framebuffer);

	glGenTextures(1, &textureID);
	glGenRenderbuffers(1, &renderbuffer);

	resize(resolution);

	return true;
}

void RenderTexture::resize(ImVec2 resolution)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glBindTexture(GL_TEXTURE_2D, textureID);

	this->resolution = resolution;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, resolution.x, resolution.y, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);
	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_BACK);
	glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, resolution.x, resolution.y);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTexture::bind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glViewport(0, 0, resolution.x, resolution.y);
}

void RenderTexture::unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTexture::clear()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}