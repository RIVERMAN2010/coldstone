#pragma once
#include <GL/glew.h>
#include "Sprite.h"
#include "Transform.h"
#include <vector>
#include "Shader.h"

#define MAX_SPRITES 1000
#define MAX_TEXTURES 32

struct Vertex {
	GLfloat position[3];
	GLfloat texCoords[2];
	GLfloat textureID;
};

class SpriteRenderer;

class SpriteRenderBatch {
friend class SpriteRenderer;
public:
	SpriteRenderBatch(Shader* shader, GLuint ibo);
	bool add(Sprite& sprite, Transform& transform);
	void render();
	void clear();
	void updateBuffer();
	void bindTextures();
	void destroy();
private:
	GLuint vao;
	GLuint vbo;
	GLuint ibo;
	std::vector<GLuint> textureSlots;
	std::vector<Vertex> vertices;
	Shader* shader;
};

