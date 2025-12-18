#undef _UNICODE
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <string>
#include <iostream>
#include "SpriteRenderer.h"

struct ImageData {
    int width;
    int height;
    int channels;
    unsigned char* pixels;
};
ImageData LoadImage(const std::string& path)
{
    ILuint image;
    ilGenImages(1, &image);
    ilBindImage(image);

    if (!ilLoadImage(path.c_str())) {
        ilDeleteImages(1, &image);
		std::cerr << "Failed to load image: " << path.c_str() << std::endl;
        return {};
    }

    // Convert to RGBA (safe for OpenGL)
    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
	iluFlipImage();
    ImageData data;
    data.width = ilGetInteger(IL_IMAGE_WIDTH);
    data.height = ilGetInteger(IL_IMAGE_HEIGHT);
    data.channels = 4;
    data.pixels = ilGetData();

    // IMPORTANT: Copy pixels if DevIL image will be destroyed
    size_t size = data.width * data.height * data.channels;
    unsigned char* copy = new unsigned char[size];
    std::memcpy(copy, data.pixels, size);
    data.pixels = copy;

    ilDeleteImages(1, &image);
    return data;
}
int main(void)
{
    GLFWwindow* window;

    
    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    ilInit();
    iluInit();

    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
    

    glfwMakeContextCurrent(window);

    

	// Initialize GLEW
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Failed to initialize GLEW" << std::endl;
		return -1;
	}
	Camera2D camera = Camera2D(0,640, 480, 0);
	SpriteRenderer spriteRenderer = SpriteRenderer(camera);

	ImageData imgData = LoadImage("steve1.png");
	std::shared_ptr<Texture> texture = std::make_shared<Texture>(imgData.pixels, imgData.width, imgData.height, GL_RGBA);
	Sprite sprite;
	sprite.texture = texture;
	Transform transform;
    transform.position = glm::vec3(100, 100, 0);
    transform.size = glm::vec2(200, 200);

	imgData = LoadImage("image.png");
    texture = std::make_shared<Texture>(imgData.pixels, imgData.width, imgData.height, GL_RGBA);
    Sprite sprite2;
    sprite2.texture = texture;
    Transform transform2;
    transform2.position = glm::vec3(300, 100, 0);
    transform2.size = glm::vec2(200, 200);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);


    while (!glfwWindowShouldClose(window))
    {
        glClear(GL_COLOR_BUFFER_BIT);
		
        spriteRenderer.begin();
        spriteRenderer.submit(sprite, transform);
		spriteRenderer.submit(sprite2, transform2);
        spriteRenderer.end();
        spriteRenderer.flush();

        glfwSwapBuffers(window);
        glfwPollEvents();

    }
	//call this before exiting to clean up GPU resources
	spriteRenderer.destroy();

    glfwTerminate();
    return 0;
}