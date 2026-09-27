#include "Game.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

namespace {
float length(const sf::Vector2f& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

sf::Vector2f normalize(const sf::Vector2f& v) {
    float len = length(v);
    if (len < 0.0001f) {
        return {0.f, 0.f};
    }
    return {v.x / len, v.y / len};
}

bool loadFontSafe(sf::Font& font) {
    const std::vector<std::string> paths = {
        "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        "/usr/share/fonts/truetype/liberation2/LiberationSans-Bold.ttf",
        "C:/Windows/Fonts/arialbd.ttf",
        "C:/Windows/Fonts/consola.ttf"
    };

    for (const auto& path : paths) {
        if (font.loadFromFile(path)) {
            return true;
        }
    }
    return false;
}
}

Player::Player() {
    torso_.setSize({22.f, 32.f});
    torso_.setFillColor(sf::Color(80, 180, 255));
    torso_.setOrigin(11.f, 16.f);
    torso_.setPosition(position);

    head_.setRadius(11.f);
    head_.setFillColor(sf::Color(245, 210, 180));
    head_.setOrigin(11.f, 11.f);
    head_.setPosition(position.x, position.y - 22.f);

    weapon_.setSize({32.f, 8.f});
    weapon_.setFillColor(sf::Color(30, 30, 30));
    weapon_.setOrigin(0.f, 4.f);
    weapon_.setPosition(position.x + 10.f, position.y + 4.f);

    legs_.setSize({14.f, 18.f});
    legs_.setFillColor(sf::Color(40, 40, 40));
    legs_.setOrigin(7.f, 9.f);
    legs_.setPosition(position.x, position.y + 26.f);
}

void Player::update(float dt, const sf::Vector2u& windowSize, const ModManager& modManager) {
    speed = modManager.settings().playerSpeed;
    fireRate = modManager.settings().fireRate;
    bulletSpeed = modManager.settings().bulletSpeed;
    bulletDamage = static_cast<float>(modManager.settings().bulletDamage);

    float moveX = 0.f;
    float moveY = 0.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        moveY -= 1.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        moveY += 1.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        moveX -= 1.f;
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        moveX += 1.f;
    }

    if (moveX != 0.f || moveY != 0.f) {
        auto direction = normalize({moveX, moveY});
        position += direction * speed * dt;
    }

    position.x = std::clamp(position.x, 25.f, static_cast<float>(windowSize.x - 25));
    position.y = std::clamp(position.y, 25.f, static_cast<float>(windowSize.y - 25));

    torso_.setPosition(position);
    head_.setPosition(position.x, position.y - 18.f);
    legs_.setPosition(position.x, position.y + 22.f);

    const sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition());
    const sf::Vector2f aimDir = normalize({mousePos.x - position.x, mousePos.y - position.y});
    const float angle = std::atan2(aimDir.y, aimDir.x) * 180.f / 3.14159265f;

    torso_.setRotation(angle);
    head_.setRotation(angle);
    weapon_.setRotation(angle);
    legs_.setRotation(angle * 0.35f);

    weapon_.setPosition(position.x + 16.f + (scoped ? 10.f : 0.f), position.y + 2.f);
    weapon_.setSize(scoped ? sf::Vector2f(36.f, 7.f) : sf::Vector2f(24.f, 7.f));
    if (fireCooldown > 0.f) {
        fireCooldown -= dt;
    }
}

void Player::draw(sf::RenderWindow& window) const {
    window.draw(torso_);
    window.draw(head_);
    window.draw(legs_);
    window.draw(weapon_);
}

void Player::fire(std::vector<Bullet>& bullets, const sf::Vector2f& aimDir) {
    if (fireCooldown > 0.f) {
        return;
    }

    const sf::Vector2f firingPos = position + normalize(aimDir) * 28.f;
    bullets.emplace_back(firingPos, normalize(aimDir) * bulletSpeed, static_cast<int>(bulletDamage), true);
    fireCooldown = fireRate;
}

Game::Game()
    : window_(sf::VideoMode(1280, 720), "Shadow Strike"),
      modManager_(),
      audioManager_() {
    modManager_.loadFromFile("mods/default_mod.json");
    window_.setFramerateLimit(60);
    window_.setKeyRepeatEnabled(false);

    if (!loadFontSafe(font_)) {
        std::cerr << "Warning: could not load a font. HUD text may be missing.\n";
    }

    hudText_.setFont(font_);
    hudText_.setCharacterSize(22);
    hudText_.setFillColor(sf::Color::White);
    hudText_.setPosition(20.f, 20.f);

    statusText_.setFont(font_);
    statusText_.setCharacterSize(20);
    statusText_.setFillColor(sf::Color(200, 200, 255));
    statusText_.setPosition(20.f, 52.f);

    titleText_.setFont(font_);
    titleText_.setCharacterSize(32);
    titleText_.setFillColor(sf::Color(255, 215, 120));
    titleText_.setString("SHADOW STRIKE");
    titleText_.setPosition(20.f, 20.f);
}

void Game::processEvents() {
    sf::Event event;
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            window_.close();
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Tab) {
            player_.scoped = !player_.scoped;
        }
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
        player_.speed = modManager_.settings().playerSpeed * 1.35f;
    }
}

void Game::spawnEnemy() {
    const float x = static_cast<float>(rand() % (window_.getSize().x - 40) + 20);
    const float y = static_cast<float>(rand() % (window_.getSize().y - 40) + 20);

    const float playerDistance = std::hypot(player_.position.x - x, player_.position.y - y);
    if (playerDistance < 220.f) {
        return;
    }

    Enemy e(x, y);
    e.speed = modManager_.settings().enemySpeed + (difficulty_ * 12.f);
    e.health = 60.f + difficulty_ * 15.f;
    e.maxHealth = e.health;
    enemies_.push_back(e);
}

void Game::handleBulletLifetime(float dt) {
    for (auto it = bullets_.begin(); it != bullets_.end();) {
        it->position += it->velocity * dt;
        it->shape.setPosition(it->position);

        if (it->position.x < -20.f || it->position.x > window_.getSize().x + 20.f ||
            it->position.y < -20.f || it->position.y > window_.getSize().y + 20.f) {
            it = bullets_.erase(it);
        } else {
            ++it;
        }
    }
}

void Game::handleCombat() {
    for (auto& bullet : bullets_) {
        if (bullet.fromPlayer) {
            for (auto& enemy : enemies_) {
                if (!enemy.alive) {
                    continue;
                }
                const float dist = std::hypot(enemy.position.x - bullet.position.x, enemy.position.y - bullet.position.y);
                if (dist < enemy.body.getRadius() + bullet.radius) {
                    enemy.health -= static_cast<float>(bullet.damage);
                    bullet.position = {-9999.f, -9999.f};
                    if (enemy.health <= 0.f) {
                        enemy.alive = false;
                        score_ += 10;
                        audioManager_.playHit();
                    } else {
                        audioManager_.playHit();
                    }
                }
            }
        } else {
            const float dist = std::hypot(player_.position.x - bullet.position.x, player_.position.y - bullet.position.y);
            if (dist < 24.f + bullet.radius) {
                player_.health -= static_cast<float>(bullet.damage);
                bullet.position = {-9999.f, -9999.f};
                audioManager_.playImpact();
            }
        }
    }

    bullets_.erase(std::remove_if(bullets_.begin(), bullets_.end(), [](const Bullet& bullet) {
        return bullet.position.x < -5000.f;
    }), bullets_.end());

    enemies_.erase(std::remove_if(enemies_.begin(), enemies_.end(), [](const Enemy& enemy) {
        return !enemy.alive;
    }), enemies_.end());
}

void Game::update(float dt) {
    const sf::Vector2u windowSize = window_.getSize();
    player_.update(dt, windowSize, modManager_);

    const sf::Vector2f mousePos = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window_));
    const sf::Vector2f aimDir = normalize({mousePos.x - player_.position.x, mousePos.y - player_.position.y});

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        player_.fire(bullets_, aimDir);
    }

    if (player_.health <= 0.f) {
        player_.health = 100.f;
        score_ = 0;
        enemies_.clear();
    }

    spawnTimer_ += dt;
    if (spawnTimer_ > 1.7f) {
        spawnEnemy();
        spawnTimer_ = 0.f;
        difficulty_ += 0.08f;
    }

    for (auto& enemy : enemies_) {
        if (!enemy.alive) {
            continue;
        }

        enemy.fireCooldown = std::max(0.f, enemy.fireCooldown - dt);
        sf::Vector2f toPlayer = player_.position - enemy.position;
        const float distance = length(toPlayer);
        if (distance > 0.0001f) {
            sf::Vector2f dir = toPlayer / distance;
            if (distance > 150.f) {
                enemy.position += dir * enemy.speed * dt;
            } else if (distance < 120.f) {
                enemy.position -= dir * 40.f * dt;
            }

            if (distance < 420.f && enemy.fireCooldown <= 0.f) {
                const sf::Vector2f bulletVelocity = dir * 500.f;
                bullets_.emplace_back(enemy.position + bulletVelocity * 0.15f, bulletVelocity, static_cast<int>(modManager_.settings().enemyDamage), false);
                enemy.fireCooldown = 1.1f;
                audioManager_.playShoot();
            }
        }

        enemy.body.setPosition(enemy.position);
    }

    handleBulletLifetime(dt);
    handleCombat();

    hudText_.setString("Health: " + std::to_string(static_cast<int>(player_.health)) +
        "  |  Score: " + std::to_string(score_) +
        "  |  Scope: " + std::string(player_.scoped ? "OPEN" : "CLOSED") +
        "  |  Wave: " + std::to_string(static_cast<int>(difficulty_ * 10.f)));

    statusText_.setString("Enemies alive: " + std::to_string(enemies_.size()) +
        "  |  Press Tab to toggle scope");
}

void Game::drawCrosshair(const sf::Vector2f& mousePosition) {
    sf::CircleShape crosshair(10.f);
    crosshair.setFillColor(sf::Color::Transparent);
    crosshair.setOutlineColor(sf::Color::White);
    crosshair.setOutlineThickness(2.f);
    crosshair.setPosition(mousePosition.x - 10.f, mousePosition.y - 10.f);
    window_.draw(crosshair);

    sf::RectangleShape lineH({18.f, 2.f});
    lineH.setFillColor(sf::Color::White);
    lineH.setPosition(mousePosition.x - 9.f, mousePosition.y);
    window_.draw(lineH);

    sf::RectangleShape lineV({2.f, 18.f});
    lineV.setFillColor(sf::Color::White);
    lineV.setPosition(mousePosition.x, mousePosition.y - 9.f);
    window_.draw(lineV);
}

void Game::drawScopeOverlay(const sf::Vector2f& mousePosition) {
    if (!player_.scoped) {
        return;
    }

    const float radius = 110.f;
    sf::CircleShape scopeCircle(radius);
    scopeCircle.setFillColor(sf::Color::Transparent);
    scopeCircle.setOutlineColor(sf::Color(255, 255, 255, 200));
    scopeCircle.setOutlineThickness(3.f);
    scopeCircle.setPosition(mousePosition.x - radius, mousePosition.y - radius);
    window_.draw(scopeCircle);

    sf::RectangleShape horizontal({500.f, 2.f});
    horizontal.setFillColor(sf::Color(255, 255, 255, 150));
    horizontal.setPosition(0.f, mousePosition.y);
    window_.draw(horizontal);

    sf::RectangleShape vertical({2.f, 500.f});
    vertical.setFillColor(sf::Color(255, 255, 255, 150));
    vertical.setPosition(mousePosition.x, 0.f);
    window_.draw(vertical);
}

void Game::render() {
    window_.clear(sf::Color(18, 21, 34));

    sf::RectangleShape floor({static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y)});
    floor.setFillColor(sf::Color(58, 70, 85));
    window_.draw(floor);

    for (int i = 0; i < 20; ++i) {
        sf::RectangleShape line({2.f, 100.f});
        line.setFillColor(sf::Color(255, 255, 255, 12));
        line.setPosition(i * 70.f, 0.f);
        window_.draw(line);
    }

    for (auto& bullet : bullets_) {
        window_.draw(bullet.shape);
    }

    for (auto& enemy : enemies_) {
        if (enemy.alive) {
            enemy.body.setOutlineColor(sf::Color::White);
            window_.draw(enemy.body);
        }
    }

    player_.draw(window_);

    const sf::Vector2f mousePosition = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window_));
    drawCrosshair(mousePosition);
    drawScopeOverlay(mousePosition);

    window_.draw(titleText_);
    window_.draw(hudText_);
    window_.draw(statusText_);

    window_.display();
}

void Game::run() {
    loadingScreen_.show(window_);
    audioManager_.playMusic();

    sf::Clock frameClock;
    while (window_.isOpen()) {
        const float dt = frameClock.restart().asSeconds();
        processEvents();
        update(dt);
        render();
    }
}
