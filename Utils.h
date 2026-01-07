#pragma once
#include <string>
#include <vector>

void Log(const std::string& message);
std::wstring ToWString(const std::string& str);
std::string GetExeDirectory();