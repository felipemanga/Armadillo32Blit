#pragma once

#include "constants.hpp"
#include <Femto>
#include <cstdio>

inline bool renderCutScene(u32 scene, u32 page) {
    constexpr const u32 pageSize = 220 * 176 * 2;
    File logo;
    char buff[100];
    snprintf(buff, sizeof(buff), "data/mission_%d.i16", scene);
    if (!logo.openRO(buff)){
        LOG("Could not open ", buff, "\n");
        return false;
    }
    u32 size = logo.size();
    if (page * pageSize >= size) {
        LOG("End of CutScene\n");
        return false;
    }
    LOG("Rendered scene ", scene, "-", page, "\n");
    logo.seek(page * pageSize);
    streamI16(logo, 220, 176, colorFromRGB(0x111122));
    return true;
}

inline u32 currentScenePage = 0, deadkey = 0;

inline void nextScenePage() {
    if (!renderCutScene(universe.missionId, currentScenePage++)) {
        currentScenePage = 0;
        targetBacklight = 0;
    }
    deadkey = getTime();
}

inline void updateEnterCutScene() {
    if (updateEnter(GameState::CutScene)) {
        gameRenderer->detach();
        nextScenePage();
    }
}

inline void updateCutScene() {
    delay(0);
    if (targetBacklight != 0) {
        if ((getTime() - deadkey) > 250 &&  isPressed(Button::A)) {
            nextScenePage();
        }
    } else {
        if (backlight == 0) {
            targetBacklight = 255;
            gameState = GameState::Space;
            gameRenderer->attach();
        }
    }
}
