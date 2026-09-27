#pragma once

#include <SFML/Audio.hpp>
#include <cmath>
#include <vector>

class AudioManager {
public:
    AudioManager();
    void playShoot();
    void playHit();
    void playImpact();
    void playMusic();

private:
    sf::SoundBuffer shootBuffer_;
    sf::SoundBuffer hitBuffer_;
    sf::SoundBuffer impactBuffer_;
    sf::SoundBuffer musicBuffer_;
    sf::Sound shootSound_;
    sf::Sound hitSound_;
    sf::Sound impactSound_;
    sf::Sound music_;

    static sf::SoundBuffer generateTone(float frequency, float durationSeconds, float volume, int sampleRate = 44100);
};
