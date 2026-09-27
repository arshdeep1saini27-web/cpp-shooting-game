#include "LoadingScreen.h"

#include <string>
#include <vector>

namespace {
bool loadLoadingFont(sf::Font& font) {
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

void LoadingScreen::show(sf::RenderWindow& window) {
    sf::Font font;
    if (!loadLoadingFont(font)) {
        return;
    }

    sf::Text title("LOADING", font, 40);
    title.setFillColor(sf::Color::White);
    title.setPosition(window.getSize().x / 2.f - 110.f, window.getSize().y / 2.f - 110.f);

    sf::Text subtitle("INITIALIZING WEAPONS, AI, AND ENVIRONMENT", font, 18);
    subtitle.setFillColor(sf::Color(180, 180, 180));
    subtitle.setPosition(window.getSize().x / 2.f - 250.f, window.getSize().y / 2.f - 55.f);

    sf::RectangleShape loader({600.f, 24.f});
    loader.setFillColor(sf::Color(40, 40, 40));
    loader.setOutlineThickness(2.f);
    loader.setOutlineColor(sf::Color::White);
    loader.setPosition(window.getSize().x / 2.f - 300.f, window.getSize().y / 2.f + 20.f);

    sf::RectangleShape progress({0.f, 24.f});
    progress.setFillColor(sf::Color(90, 200, 255));
    progress.setPosition(loader.getPosition());

    // Short preview: 10 quick frames instead of a long artificial loading delay.
    for (int step = 0; step <= 10; ++step) {
        progress.setSize({static_cast<float>(step) * 60.f, 24.f});
        window.clear(sf::Color(8, 12, 18));
        window.draw(title);
        window.draw(subtitle);
        window.draw(loader);
        window.draw(progress);
        window.display();
        sf::sleep(sf::milliseconds(5));
    }
}
