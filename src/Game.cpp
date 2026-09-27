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

void Player::setWeapon(int weaponId) {
    currentWeapon = weaponId;
    if (weaponId == 0) {
        fireRate = 0.12f;
        bulletSpeed = 1250.f;
        bulletDamage = 35.f;
    } else if (weaponId == 1) {
        fireRate = 0.18f;
        bulletSpeed = 1400.f;
        bulletDamage = 28.f;
    } else if (weaponId == 2) {
        fireRate = 0.06f;
        bulletSpeed = 1150.f;
        bulletDamage = 50.f;
    }
}

void Player::update(float dt, const sf::Vector2u& windowSize, const sf::RenderWindow& window, const ModManager& modManager) {
    speed = modManager.settings().playerSpeed;
    fireRate = modManager.settings().fireRate;
    bulletSpeed = modManager.settings().bulletSpeed;
    bulletDamage = static_cast<float>(modManager.settings().bulletDamage);

    float moveX = 0.f;
    float moveY = 0.f;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) moveY -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) moveY += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) moveX -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) moveX += 1.f;

    const float sprintBoost = sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? 1.85f : 1.f;

    if (moveX != 0.f || moveY != 0.f) {
        auto direction = normalize({moveX, moveY});
        position += direction * speed * sprintBoost * dt;
    }

    position.x = std::clamp(position.x, 25.f, static_cast<float>(windowSize.x - 25));
    position.y = std::clamp(position.y, 25.f, static_cast<float>(windowSize.y - 25));

    torso_.setPosition(position);
    head_.setPosition(position.x, position.y - 18.f);
    legs_.setPosition(position.x, position.y + 22.f);

    const sf::Vector2f mousePos = sf::Mouse::getPosition(window);
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
    if (fireCooldown > 0.f) return;
    const sf::Vector2f firingPos = position + normalize(aimDir) * 28.f;
    bullets.emplace_back(firingPos, normalize(aimDir) * bulletSpeed, static_cast<int>(bulletDamage), true);
    fireCooldown = fireRate;
}

Game::Game()
    : window_(sf::VideoMode(1280, 720), "Shadow Strike"),
      modManager_(),
      audioManager_() {
    modManager_.loadFromFile("mods/default_mod.json");
    player_.setWeapon(0);
    window_.setFramerateLimit(60);
    window_.setKeyRepeatEnabled(false);

    if (!loadFontSafe(font_)) {
        std::cerr << "Warning: could not load a font. HUD text may be missing.\n";
    }

    hudText_.setFont(font_);
    hudText_.setCharacterSize(20);
    hudText_.setFillColor(sf::Color::White);
    hudText_.setPosition(20.f, 20.f);

    statusText_.setFont(font_);
    statusText_.setCharacterSize(18);
    statusText_.setFillColor(sf::Color(200, 200, 255));
    statusText_.setPosition(20.f, 52.f);

    titleText_.setFont(font_);
    titleText_.setCharacterSize(32);
    titleText_.setFillColor(sf::Color(255, 215, 120));
    titleText_.setString("SHADOW STRIKE");
    titleText_.setPosition(20.f, 20.f);

    menuText_.setFont(font_);
    menuText_.setCharacterSize(28);
    menuText_.setFillColor(sf::Color(240, 240, 240));
    menuText_.setString("Press ENTER to start\n1 = Rifle\n2 = SMG\n3 = Burst Gun\nESC to quit\nHold SHIFT to sprint");
    menuText_.setPosition(360.f, 320.f);

    pauseText_.setFont(font_);
    pauseText_.setCharacterSize(28);
    pauseText_.setFillColor(sf::Color::White);
    pauseText_.setString("PAUSED\nPress ENTER to resume\n1 / 2 / 3 = switch weapon\nESC to quit\nSHIFT = sprint");
    pauseText_.setPosition(360.f, 260.f);
}

void Game::processEvents() {
    sf::Event event;
    while (window_.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window_.close();
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
            if (state_ == GameState::Playing) {
                state_ = GameState::Paused;
                showPauseMenu();
            } else if (state_ == GameState::Paused) {
                state_ = GameState::Menu;
            } else if (state_ == GameState::Menu) {
                window_.close();
            }
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Tab) {
            if (state_ == GameState::Playing) {
                player_.scoped = !player_.scoped;
            }
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
            if (state_ == GameState::Menu) {
                state_ = GameState::Playing;
                audioManager_.playMusic();
            } else if (state_ == GameState::Paused) {
                state_ = GameState::Playing;
            }
        }

        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Num1) {
            player_.setWeapon(0);
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Num2) {
            player_.setWeapon(1);
        }
        if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Num3) {
            player_.setWeapon(2);
        }
    }
}

void Game::showMainMenu() {
    state_ = GameState::Menu;
    player_.health = 100.f;
    score_ = 0;
    enemies_.clear();
    bullets_.clear();
    difficulty_ = 1.f;
}

void Game::showPauseMenu() {
    state_ = GameState::Paused;
}

void Game::spawnEnemy() {
    const float x = static_cast<float>(rand() % (window_.getSize().x - 40) + 20);
    const float y = static_cast<float>(rand() % (window_.getSize().y - 40) + 20);

    const float playerDistance = std::hypot(player_.position.x - x, player_.position.y - y);
    if (playerDistance < 220.f) {
        return;
    }

    Enemy e(x, y);
    e.speed = modManager_.settings().enemySpeed + (difficulty_ * 14.f);
    e.health = 60.f + difficulty_ * 18.f;
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
                if (!enemy.alive) continue;
                float dist = std::hypot(enemy.position.x - bullet.position.x, enemy.position.y - bullet.position.y);
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
            float dist = std::hypot(player_.position.x - bullet.position.x, player_.position.y - bullet.position.y);
            if (dist < 22.f + bullet.radius) {
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
    if (state_ != GameState::Playing) return;

    const sf::Vector2u windowSize = window_.getSize();
    player_.update(dt, windowSize, window_, modManager_);

    const sf::Vector2f mousePos = sf::Mouse::getPosition(window_);
    const sf::Vector2f aimDir = normalize({mousePos.x - player_.position.x, mousePos.y - player_.position.y});

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        player_.fire(bullets_, aimDir);
    }

    if (player_.health <= 0.f) {
        state_ = GameState::GameOver;
        player_.health = 100.f;
        score_ = 0;
        enemies_.clear();
        bullets_.clear();
    }

    spawnTimer_ += dt;
    if (spawnTimer_ > 0.75f) {
        spawnEnemy();
        spawnTimer_ = 0.f;
        difficulty_ += 0.15f;
    }

    for (auto& enemy : enemies_) {
        if (!enemy.alive) continue;

        enemy.fireCooldown = std::max(0.f, enemy.fireCooldown - dt);
        sf::Vector2f toPlayer = player_.position - enemy.position;
        const float distance = length(toPlayer);
        if (distance > 0.0001f) {
            sf::Vector2f dir = toPlayer / distance;
            if (distance > 150.f) {
                enemy.position += dir * enemy.speed * dt;
            } else if (distance < 120.f) {
                enemy.position -= dir * 55.f * dt;
            }

            if (distance < 420.f && enemy.fireCooldown <= 0.f) {
                const sf::Vector2f bulletVelocity = dir * 700.f;
                bullets_.emplace_back(enemy.position + bulletVelocity * 0.15f, bulletVelocity, static_cast<int>(modManager_.settings().enemyDamage), false);
                enemy.fireCooldown = 0.55f;
                audioManager_.playShoot();
            }
        }

        enemy.body.setPosition(enemy.position);
    }

    handleBulletLifetime(dt);
    handleCombat();

    hudText_.setString("Health: " + std::to_string(static_cast<int>(player_.health)) +
        "  |  Score: " + std::to_string(score_) +
        "  |  Weapon: " + std::string(player_.currentWeapon == 0 ? "Rifle" : player_.currentWeapon == 1 ? "SMG" : "Burst") +
        "  |  Scope: " + std::string(player_.scoped ? "OPEN" : "CLOSED") +
        "  |  Wave: " + std::to_string(static_cast<int>(difficulty_ * 10.f)) +
        "  |  Sprint: " + std::string(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) ? "ON" : "OFF"));

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
    if (!player_.scoped || state_ != GameState::Playing) return;

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

void Game::renderMenu() {
    window_.clear(sf::Color(9, 15, 25));
    sf::Text title("SHADOW STRIKE", font_, 54);
    title.setFillColor(sf::Color(255, 215, 120));
    title.setPosition(350.f, 140.f);

    sf::Text subtitle("WEAPON SWITCH MODE", font_, 26);
    subtitle.setFillColor(sf::Color(180, 180, 180));
    subtitle.setPosition(420.f, 220.f);

    window_.draw(title);
    window_.draw(subtitle);
    window_.draw(menuText_);
    window_.display();
}

void Game::renderPauseOverlay() {
    sf::RectangleShape overlay({static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y)});
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window_.draw(overlay);
    window_.draw(pauseText_);
}

void Game::render() {
    window_.clear(sf::Color(18, 21, 34));

    if (state_ == GameState::Menu) {
        renderMenu();
        return;
    }

    if (state_ == GameState::Paused) {
        sf::RectangleShape floor({static_cast<float>(window_.getSize().x), static_cast<float>(window_.getSize().y)});
        floor.setFillColor(sf::Color(58, 70, 85));
        window_.draw(floor);

        for (auto& bullet : bullets_) window_.draw(bullet.shape);
        for (auto& enemy : enemies_) if (enemy.alive) window_.draw(enemy.body);
        player_.draw(window_);

        const sf::Vector2f mousePosition = sf::Mouse::getPosition(window_);
        drawCrosshair(mousePosition);
        drawScopeOverlay(mousePosition);
        renderPauseOverlay();
        window_.draw(titleText_);
        window_.draw(hudText_);
        window_.draw(statusText_);
        window_.display();
        return;
    }

    if (state_ == GameState::GameOver) {
        sf::Text gameOver("MISSION FAILED", font_, 42);
        gameOver.setFillColor(sf::Color(255, 90, 90));
        gameOver.setPosition(400.f, 250.f);

        sf::Text restart("Press ENTER to restart", font_, 24);
        restart.setFillColor(sf::Color::White);
        restart.setPosition(430.f, 350.f);

        window_.draw(gameOver);
        window_.draw(restart);
        window_.display();
        return;
    }

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

    const sf::Vector2f mousePosition = sf::Mouse::getPosition(window_);
    drawCrosshair(mousePosition);
    drawScopeOverlay(mousePosition);

    window_.draw(titleText_);
    window_.draw(hudText_);
    window_.draw(statusText_);
    window_.display();
}

void Game::run() {
    loadingScreen_.show(window_);
    showMainMenu();

    sf::Clock frameClock;
    while (window_.isOpen()) {
        processEvents();
        if (state_ == GameState::GameOver && sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
            player_.health = 100.f;
            score_ = 0;
            enemies_.clear();
            bullets_.clear();
            difficulty_ = 1.f;
            state_ = GameState::Playing;
        }

        float dt = frameClock.restart().asSeconds();
        update(dt);
        render();
    }
}
