#include "Utils.h"
#include <fstream>
#include <filesystem>
#include <format>
#include <windows.h>
#include <iostream>

static std::ofstream g_LogFile;

void Log(const std::string& message) {
    if (!g_LogFile.is_open()) {
        std::string path = GetExeDirectory() + "/log.md";
        g_LogFile.open(path, std::ios::app);
    }
    std::string formatted = std::format("{}  \n", message);
    if (g_LogFile.is_open()) {
        g_LogFile << formatted;
        g_LogFile.flush();
    }
    std::cout << formatted;
}

std::wstring ToWString(const std::string& str) {
    return std::wstring(str.begin(), str.end());
}

std::string GetExeDirectory() {
    wchar_t buffer[MAX_PATH];
    GetModuleFileNameW(NULL, buffer, MAX_PATH);
    std::filesystem::path path(buffer);
    return path.parent_path().string();
}