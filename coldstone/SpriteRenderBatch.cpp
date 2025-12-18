#include "SpriteRenderBatch.h"

SpriteRenderBatch::SpriteRenderBatch(Shader* _shader, GLuint _ibo) : ibo(_ibo), shader(_shader) {
	vertices.reserve(MAX_SPRITES * 4);
	textureSlots.reserve(MAX_TEXTURES);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, MAX_SPRITES * 4 * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);

	glEnableVertexArrayAttrib(vbo, 0);
	//glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::position));

	//glEnableVertexAttribArray(1);
	glEnableVertexArrayAttrib(vbo, 1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::texCoords));

	//glEnableVertexAttribArray(2);
	glEnableVertexArrayAttrib(vbo, 2);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Vertex::textureID));

}
bool SpriteRenderBatch::add(Sprite& sprite, Transform& transform) {
	if(vertices.size() / 4 >= MAX_SPRITES) {
		return false;
	}
	bool hasTexture = std::find(textureSlots.begin(), textureSlots.end(), sprite.texture->id) != textureSlots.end();
	if(textureSlots.size() >= MAX_TEXTURES && !hasTexture) {
		return false;
	}
	if (!hasTexture) {
		textureSlots.push_back(sprite.texture->id);
	}

	GLfloat mappedTexture = std::distance(textureSlots.begin(),
		std::find(textureSlots.begin(), textureSlots.end(), sprite.texture->id));

	vertices.push_back({
		{transform.position.x, transform.position.y, transform.position.z},
		{0, 0},
		mappedTexture
		});
	vertices.push_back({
		{transform.position.x + transform.size.x, transform.position.y, transform.position.z},
		{1, 0},
		mappedTexture
		});
	vertices.push_back({
		{transform.position.x + transform.size.x, transform.position.y + transform.size.y, transform.position.z},
		{1, 1},
		mappedTexture
		});
	vertices.push_back({
		{transform.position.x,  transform.position.y + transform.size.y, transform.position.z},
		{0, 1},
		mappedTexture
		});

	
	return true;
}

void SpriteRenderBatch::updateBuffer() {
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
}
void SpriteRenderBatch::bindTextures() {
	for(size_t i = 0; i < textureSlots.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, textureSlots[i]);
	}
}

void SpriteRenderBatch::render() {
	for(size_t i = 0; i < textureSlots.size(); i++) {
		shader->setInt("u_Textures[" + std::to_string(i) + "]", i);
	}
	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glDrawElements(GL_TRIANGLES, (vertices.size()/4)*6, GL_UNSIGNED_INT, 0);
}

void SpriteRenderBatch::clear() {
	vertices.clear();
	textureSlots.clear();
}

void SpriteRenderBatch::destroy() {
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}