#include "MainDevIL.h"
#include "Utils.h"
#include <IL/il.h>
#include <IL/ilu.h>
#include <GL/glew.h>
#include <format>
#include <fstream>
#include <vector>

#pragma comment(lib, "DevIL.lib")
#pragma comment(lib, "ILU.lib")

void MainDevIL::Initialize() {
    ilInit();
    iluInit();
    Log("DevIL Initialized.");
}

unsigned int MainDevIL::LoadInternal(void* data, unsigned int size) {
    ILuint imgID;
    ilGenImages(1, &imgID);
    ilBindImage(imgID);

    if (!ilLoadL(IL_TYPE_UNKNOWN, data, (ILuint)size)) {
        return 0; // Error handling caller überlassen
    }

    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT),
        0, GL_RGBA, GL_UNSIGNED_BYTE, ilGetData());

    ilDeleteImages(1, &imgID);
    return texID;
}

unsigned int MainDevIL::LoadTexture(const std::string& filepath) {
    if (filepath.empty()) return 0;

    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        Log(std::format("DevIL IO Error: Could not open file {}", filepath));
        return 0;
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size)) {
        Log(std::format("DevIL IO Error: Could not read file {}", filepath));
        return 0;
    }

    unsigned int id = LoadInternal(buffer.data(), (unsigned int)size);
    if (id == 0) {
        Log(std::format("DevIL Failed to decode: {}", filepath));
    }
    else {
        Log(std::format("Texture loaded: {}", filepath));
    }
    return id;
}

unsigned int MainDevIL::LoadTextureFromMemory(void* data, unsigned int size) {
    unsigned int id = LoadInternal(data, size);
    if (id == 0) Log("DevIL Failed to decode embedded texture.");
    else Log("Embedded texture loaded.");
    return id;
}

unsigned int MainDevIL::CreateWhiteTexture() {
    unsigned int texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    unsigned char whiteData[] = { 255, 255, 255, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whiteData);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return texID;
}
