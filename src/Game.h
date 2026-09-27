#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "ModManager.h"
#include "LoadingScreen.h"
#include "AudioManager.h"

struct Bullet {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float radius = 5.f;
    int damage = 10;
    bool fromPlayer = true;
    sf::CircleShape shape;

    Bullet(const sf::Vector2f& pos, const sf::Vector2f& vel, int dmg, bool owner)
        : position(pos), velocity(vel), damage(dmg), fromPlayer(owner) {
        shape.setRadius(radius);
        shape.setOrigin(radius, radius);
        shape.setFillColor(fromPlayer ? sf::Color::Yellow : sf::Color::Red);
        shape.setPosition(position);
    }
};

struct Enemy {
    sf::Vector2f position;
    float health = 60.f;
    float maxHealth = 60.f;
    float speed = 90.f;
    float fireCooldown = 0.f;
    sf::CircleShape body;
    bool alive = true;

    Enemy(float x, float y)
        : position(x, y) {
        body.setRadius(18.f);
        body.setOrigin(18.f, 18.f);
        body.setFillColor(sf::Color(220, 80, 80));
        body.setOutlineThickness(3.f);
        body.setOutlineColor(sf::Color::White);
        body.setPosition(position);
    }
};

class Player {
public:
    Player();
    void update(float dt, const sf::Vector2u& windowSize, const sf::RenderWindow& window, const ModManager& modManager);
    void draw(sf::RenderWindow& window) const;
    void fire(std::vector<Bullet>& bullets, const sf::Vector2f& aimDir);
    void setWeapon(int weaponId);
    void setRecoil(float kick); 

    sf::Vector2f position{200.f, 200.f};
    float speed = 320.f;
    float health = 100.f;
    float fireCooldown = 0.f;
    float bulletSpeed = 900.f;
    float bulletDamage = 25.f;
    float fireRate = 0.18f;
    float recoil = 0.f;
    bool scoped = false;
    int currentWeapon = 0;

private:
    sf::RectangleShape torso_;
    sf::RectangleShape head_;
    sf::RectangleShape weapon_;
    sf::RectangleShape legs_;
};

enum class GameState {
    Menu,
    Playing,
    Paused,
    GameOver
};

class Game {
public:
    Game();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void renderMenu();
    void renderPauseOverlay();
    void showMainMenu();
    void showPauseMenu();
    void spawnEnemy();
    void handleBulletLifetime(float dt);
    void handleCombat();
    void drawCrosshair(const sf::Vector2f& mousePosition);
    void drawScopeOverlay(const sf::Vector2f& mousePosition);

    sf::RenderWindow window_;
    sf::Clock clock_;
    sf::Font font_;
    sf::Text hudText_;
    sf::Text statusText_;
    sf::Text titleText_;
    sf::Text menuText_;
    sf::Text pauseText_;

    Player player_;
    std::vector<Enemy> enemies_;
    std::vector<Bullet> bullets_;
    ModManager modManager_;
    LoadingScreen loadingScreen_;
    AudioManager audioManager_;

    GameState state_ = GameState::Menu;
    float spawnTimer_ = 0.f;
    float difficulty_ = 1.f;
    int score_ = 0;
};
