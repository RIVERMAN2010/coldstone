#include "RenderFont.h"
#include <GL/glew.h>
#include <Windows.h>
#include <vector>
#include <format>

static unsigned int baseList = 0;
static std::string currentText = "";
static bool initialized = false;

void RenderFont::Initialize() {
    HDC hdc = wglGetCurrentDC();
    if (!hdc) return;

    if (baseList != 0) glDeleteLists(baseList, 96);
    baseList = glGenLists(96);

    HFONT hFont = CreateFontA(-16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, FF_DONTCARE | DEFAULT_PITCH, "Arial");

    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    wglUseFontBitmaps(hdc, 32, 96, baseList);
    SelectObject(hdc, hOldFont);
    DeleteObject(hFont);

    initialized = true;
}

void RenderFont::UpdateText(const std::string& text) {
    currentText = text;
}

void RenderFont::Draw(int screenWidth, int screenHeight) {
    if (!initialized || currentText.empty()) return;

    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    glOrtho(0.0, (double)screenWidth, (double)screenHeight, 0.0, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_CULL_FACE);

    glColor4f(0.0f, 1.0f, 0.0f, 1.0f);
    glRasterPos2i(10, 20);

    glListBase(baseList - 32);
    glCallLists((GLsizei)currentText.length(), GL_UNSIGNED_BYTE, currentText.c_str());

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
}

void RenderFont::Cleanup() {
    if (baseList != 0) glDeleteLists(baseList, 96);
}
