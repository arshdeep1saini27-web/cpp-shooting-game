#include "ModManager.h"

#include <filesystem>
#include <iostream>
#include <regex>

namespace {
std::string readText(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return {};
    }
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

float parseFloatValue(const std::string& jsonText, const std::string& key, float fallback) {
    const std::regex pattern("\"" + key + "\"\\s*:\\s*([-0-9.]+)");
    std::smatch match;
    if (std::regex_search(jsonText, match, pattern)) {
        try {
            return std::stof(match[1].str());
        } catch (const std::exception&) {
            return fallback;
        }
    }
    return fallback;
}
}

void ModManager::loadFromFile(const std::string& path) {
    const std::string content = readText(path);
    if (content.empty()) {
        std::cout << "Mod file not found: " << path << ". Using default values.\n";
        return;
    }

    settings_.playerSpeed = parseFloatValue(content, "playerSpeed", settings_.playerSpeed);
    settings_.enemySpeed = parseFloatValue(content, "enemySpeed", settings_.enemySpeed);
    settings_.enemyDamage = parseFloatValue(content, "enemyDamage", settings_.enemyDamage);
    settings_.fireRate = parseFloatValue(content, "fireRate", settings_.fireRate);
    settings_.bulletSpeed = parseFloatValue(content, "bulletSpeed", settings_.bulletSpeed);
    settings_.bulletDamage = parseFloatValue(content, "bulletDamage", settings_.bulletDamage);
    settings_.scopeZoom = parseFloatValue(content, "scopeZoom", settings_.scopeZoom);

    std::cout << "Loaded mod configuration from " << path << "\n";
}
