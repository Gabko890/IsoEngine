#include "Utils.hpp"

#include <windows.h>
#include <string>
#include <Shlwapi.h>


std::string Utils::GetFullPath(const char* input_path) {
    if (!input_path || strlen(input_path) == 0) {
        return "";
    }

    if (!PathIsRelativeA(input_path)) {
        return std::string(input_path);
    }

    char exe_path[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    if (len == 0 || len == MAX_PATH) {
        return "";
    }

    for (int i = len - 1; i >= 0; --i) {
        if (exe_path[i] == '\\' || exe_path[i] == '/') {
            exe_path[i + 1] = '\0';
            break;
        }
    }

    std::string full_path = std::string(exe_path) + input_path;

    char absolute_path[MAX_PATH];
    if (_fullpath(absolute_path, full_path.c_str(), MAX_PATH)) {
        return std::string(absolute_path);
    }

    return full_path;  
}