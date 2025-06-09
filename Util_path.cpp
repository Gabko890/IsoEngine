#include <string>
#include <windows.h>

#include "Utils.hpp"

std::string Utils::GetFullPath(const char* relative_path){
    char exe_path[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, exe_path, MAX_PATH);
    if (len == 0 || len == MAX_PATH) {
        return "";
    }

    for (int i = len - 1; i >= 0; --i) {
        if (exe_path[i] == '\\') {
            exe_path[i + 1] = '\0';
            break;
        }
    }

    return std::string(exe_path) + relative_path;
}