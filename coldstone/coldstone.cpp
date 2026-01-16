#undef _UNICODE
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <string>
#include <iostream>
#include "RenderGraph.h"
#include "GLCommandBuffer.h"

struct ImageData {
    int width;
    int height;
    int channels;
    unsigned char* pixels;
};
ImageData LoadImage(const std::string& path) {
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
int main(void) {
    GLFWwindow* window;

    
    if (!glfwInit())
        return -1;

    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window) {
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

    struct GBufferData {
        TextureHandle albedo;
        TextureHandle normal;
        TextureHandle depth;
    };
	RenderGraph renderGraph;
	renderGraph.addPass<GBufferData>("GBuffer Pass",
        [&](RGBuilder& builder, GBufferData& data) {
            TextureDesc desc = { 640, 480, TextureFormat::RGBA8, false };
            data.albedo = builder.create(desc, "Albedo Texture");
            data.normal = builder.create(desc, "Normal Texture");
            data.depth = builder.create(desc, "Depth Texture");
		},
        [&](GBufferData& data, RGRegistry& registry, ICommandBuffer* cmd) {
            
        }
	);
    

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
		

		

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    

    glfwTerminate();
    return 0;
}