#pragma once
#include <string>

class MainDevIL {
public:
    static void Initialize();
    static unsigned int LoadTexture(const std::string& filepath);
    static unsigned int LoadTextureFromMemory(void* data, unsigned int size);
    static unsigned int CreateWhiteTexture();
private:
    static unsigned int LoadInternal(void* data, unsigned int size);
};