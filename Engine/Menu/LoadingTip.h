#ifndef LOADINGTIP_H
#define LOADINGTIP_H

#include "../Graphics/PText.h"
#include "../Graphics/PSprite.h"
#include <SFML/Graphics.hpp>

class LoadingTip
{
public:
    bool pressAnyKey = false;
    bool tipFinished = false;

    sf::Font f_font;

    PText t_tipTitle;
    PText t_tipText;
    PText t_pressAnyKey;
    PText t_nowLoading;

    sf::RectangleShape box_1, box_2;

    std::string bg_key;
    std::string icon_key;

    float angle_1=0, angle_2=0;

    // loading tip mode
    // 0 - regular loading tip with random splash and text
    // 1 - very brief loading screen for some transitions, no splash and no text
    bool tipMode = 0;

    LoadingTip(int mode=0);
    void Draw();
};

#endif LOADINGTIP_H