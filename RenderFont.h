#pragma once
#include <string>

class RenderFont {
public:
    static void Initialize();
    static void UpdateText(const std::string& text);
    static void Draw(int screenWidth, int screenHeight);
    static void Cleanup();
};