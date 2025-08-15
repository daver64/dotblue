
#define _CRT_SECURE_NO_WARNINGS
#include "KosmosBase.h"
#include <cstdlib>
#include <filesystem>

std::string FileSystem::getKosmosConfigDir() {
    std::string path;
#if defined(_WIN32)
    const char* appdata = std::getenv("APPDATA");
    if (appdata)
        path = std::string(appdata) + "\\Kosmos\\";
    else
        path = ".\\Kosmos\\";
#elif defined(__APPLE__)
    const char* home = std::getenv("HOME");
    if (home)
        path = std::string(home) + "/Library/Application Support/Kosmos/";
    else
        path = "./Kosmos/";
#else // Linux and others
    const char* xdg = std::getenv("XDG_CONFIG_HOME");
    if (xdg)
        path = std::string(xdg) + "/Kosmos/";
    else {
        const char* home = std::getenv("HOME");
        if (home)
            path = std::string(home) + "/.config/Kosmos/";
        else
            path = "./Kosmos/";
    }
#endif
    std::filesystem::create_directories(path);
    return path;
}
