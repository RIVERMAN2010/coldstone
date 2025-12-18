#pragma once
#include <GL/glew.h>

class Texture
{
public:
	Texture(void* data, int width, int height, int format);
	~Texture();
	void bind();
	GLuint id;
private:
	friend class SpriteRenderBatch;
	int width, height, format;
	bool deleted = false;
	
};

