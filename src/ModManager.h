#pragma once

#include <string>
#include <fstream>
#include <sstream>
#include <regex>

struct ModSettings {
    float playerSpeed = 320.f;
    float enemySpeed = 90.f;
    float enemyDamage = 12.f;
    float fireRate = 0.18f;
    float bulletSpeed = 900.f;
    float bulletDamage = 25.f;
    float scopeZoom = 2.5f;
};

class ModManager {
public:
    ModManager() = default;
    void loadFromFile(const std::string& path);
    const ModSettings& settings() const { return settings_; }

private:
    ModSettings settings_;
};
