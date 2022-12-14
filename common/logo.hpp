#pragma once

#if !defined(__EMSCRIPTEN__) && !defined(TARGET_32BLIT_HW)

WEAK void showLogo(){
    File logo;
    if (!logo.openRO("data/logo.i16")){
        LOG("Could not find logo\n");
        setBacklight(1);
        return;
    }

    setBacklight(0);

    u32 size = logo.size();
    constexpr u32 frameSize = 220 * 176 * 2;
    u32 frameCount = size / frameSize;
    logo.seek(random(0, frameCount) * frameSize);
    streamI16(logo, 220, 176, 0xFFFF);
    for (f32 lum = 0; lum < 1; lum += f32(0.02)) {
        setBacklight(lum);
        delay(10);
    }

    if (Audio::internal::sinkInstance) {
        logo.openRO("data/logo.raw");
        Audio::setVolume(32);
    } else {
        logo.close();
    }

    if (!logo){
        delay(2000);
    } else {
        auto& src = Audio::play<0>(logo);
        src.setLoop(false);
        while (!src.ended() && !isPressed(Button::C)) {
            delay(30);
        }
        Audio::stop<0>();
    }
}

#else

WEAK void showLogo(){
    void game_update();
    static File logo;
    static auto backup = updateHandler;

    if (!logo.openRO("data/logo.i16")){
        LOG("Could not find logo\n");
        setBacklight(1);
        return;
    }

    setBacklight(0);

    u32 size = logo.size();
    constexpr u32 frameSize = screenWidth * screenHeight * 2;
    u32 frameCount = size / frameSize;
    logo.seek(random(0, frameCount) * frameSize);
    streamI16(logo, 220, 176, 0xFFFF);
    static f32 startTime = getTime();
    static Audio::RAWFileSource *src = nullptr;
    static bool startSound = false;
    if (logo.openRO("data/logo.raw") && Audio::internal::sinkInstance){
        startSound = true;
    }

    updateHandler = +[](){
        static f32 lum;
        if (lum < 1){
            setBacklight(lum);
            lum += f32(0.02);
            return;
        }

        if (startSound) {
            startSound = false;
            src = &Audio::play<0>(logo);
            src->setLoop(false);
            Audio::setVolume(1<<8);
        }

        bool ended = (src && src->ended())
            || (!src && (getTime() - startTime > 2000))
            || isPressed(Button::C);

        if (ended) {
            Audio::stop<0>();
            updateHandler = game_update;
        }
    };
}

#endif
