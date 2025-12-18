#include "SpriteRenderer.h"

SpriteRenderer::SpriteRenderer(Camera2D& _camera) : camera(_camera) {
	glGenBuffers(1, &iboShared);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboShared);
	std::vector<GLuint> indices;
	indices.reserve(MAX_SPRITES * 6);
	for (GLuint i = 0; i < MAX_SPRITES; i++) {
		GLuint offset = i * 4;
		indices.push_back(offset + 0);
		indices.push_back(offset + 1);
		indices.push_back(offset + 2);
		indices.push_back(offset + 2);
		indices.push_back(offset + 3);
		indices.push_back(offset + 0);
	}
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);


	shader = new Shader("sprite_vertex.glsl", "sprite_frag.glsl");
}
void SpriteRenderer::begin(){
	shader->use();
	//upload camera matrix
	shader->setMat4("u_ViewProjection", camera.GetViewProjection());
}
void SpriteRenderer::submit(Sprite& component, Transform& transform){
	bool added = false;
	for(auto& batch : batches)
	{
		if(batch.add(component, transform))
		{
			added = true;
			break;
		}
	}
	if(!added)
	{
		batches.push_back(std::move(SpriteRenderBatch(shader, iboShared)));
		batches.back().add(component, transform);
	}
}

void SpriteRenderer::end() {
	for(auto& batch : batches)
	{
		batch.updateBuffer();
	}
}

void SpriteRenderer::flush() {
	for(auto& batch : batches)
	{
		batch.bindTextures();
		batch.render();
		batch.clear();
	}
	
}
void SpriteRenderer::destroy() {
	shader->destroy();
	glDeleteBuffers(1, &iboShared);
	for(auto& batch : batches)
	{
		batch.destroy();
	}
}