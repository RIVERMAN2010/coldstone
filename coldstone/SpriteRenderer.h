#pragma once
#include <GL/glew.h>
#include "SpriteRenderBatch.h"
#include "Shader.h"
#include "Camera2D.h"

class SpriteRenderer {
public:
	SpriteRenderer(Camera2D& camera);
	void submit(Sprite& sprite, Transform& transform);
	void begin();
	void end();
	void flush();
	void destroy();

private:
	Shader* shader;
	GLuint iboShared;
	std::vector<SpriteRenderBatch> batches;
	Camera2D& camera;
};

