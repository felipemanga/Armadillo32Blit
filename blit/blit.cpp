#include "../common/common_internal.hpp"
#include <chrono>
#include "32blit.hpp"
#include "engine/api_private.hpp"
#include "engine/api.hpp"
#include "engine/engine.hpp"
#include "engine/input.hpp"

#ifndef PIXEL_SIZE
#define PIXEL_SIZE 1
#endif

template<Button button> bool buttonState;
static s32 scanlineY;
static u16 *frameBuffer;
static void *nextHook;

u32 getTimeMicro() {
    auto timePoint = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::microseconds>(timePoint.time_since_epoch()).count();
}

inline bool menuButton(){
    return blit::buttons.state & blit::Button::MENU;
}

bool isPressed(Button button){
    switch(button){
    case Button::A: return buttonState<Button::A>;
    case Button::B: return buttonState<Button::B>;
    case Button::C: return buttonState<Button::C>;
    case Button::D: return buttonState<Button::D>;
    case Button::Up: return buttonState<Button::Up>;
    case Button::Down: return buttonState<Button::Down>;
    case Button::Left: return buttonState<Button::Left>;
    case Button::Right: return buttonState<Button::Right>;
    default: break;
    }
    return false;
}

static f32 backlight;

void setBacklight(f32 value) {
    value = (value < 0) ? f32(0) : value;
    value = (value >= 1) ? s24q8ToF32(255) : value;
    backlight = value;
}

u32 getFreeRAM() {
    return 0;
}

static u32 timeMS;

u32 getTime() {
    return timeMS;
}

// static void* updateEvents(bool isFrame) {
//     SDL_Event e;
//     while (SDL_PollEvent(&e)) {
//         if( e.type == SDL_WINDOWEVENT ){
//             if(e.window.event == SDL_WINDOWEVENT_RESIZED){
//                 screen = SDL_GetWindowSurface( window );
//             }
//             continue;
//         }
// #ifndef __EMSCRIPTEN__
// 	if( e.type == SDL_QUIT )
//             exit(0);
// #endif

// 	if( e.type == SDL_KEYUP || e.type == SDL_KEYDOWN ){
//             bool isDown = e.type == SDL_KEYDOWN;
// 	    switch( e.key.keysym.sym ){
//             case SDLK_LCTRL:
//             case SDLK_a:
//             case SDLK_z:
//                 buttonState<Button::A> = isDown;
//                 break;

//             case SDLK_LALT:
//             case SDLK_b:
//             case SDLK_s:
//             case SDLK_x:
//                 buttonState<Button::B> = isDown;
//                 break;

//             case SDLK_c:
//             case SDLK_d:
//                 buttonState<Button::C> = isDown;
//                 break;
// #ifdef VERTICAL_SCREEN
//             case SDLK_i:
//             case SDLK_UP:
//                 buttonState<Button::Left> = isDown;
//                 break;

//             case SDLK_k:
//             case SDLK_DOWN:
//                 buttonState<Button::Right> = isDown;
//                 break;

//             case SDLK_j:
//             case SDLK_LEFT:
//                 buttonState<Button::Down> = isDown;
//                 break;

//             case SDLK_l:
//             case SDLK_RIGHT:
//                 buttonState<Button::Up> = isDown;
//                 break;
// #else
//             case SDLK_i:
//             case SDLK_UP:
//                 buttonState<Button::Up> = isDown;
//                 break;

//             case SDLK_k:
//             case SDLK_DOWN:
//                 buttonState<Button::Down> = isDown;
//                 break;

//             case SDLK_j:
//             case SDLK_LEFT:
//                 buttonState<Button::Left> = isDown;
//                 break;

//             case SDLK_l:
//             case SDLK_RIGHT:
//                 buttonState<Button::Right> = isDown;
//                 break;
// #endif

// #ifndef __EMSCRIPTEN__
//             case SDLK_ESCAPE:
//                 exit(0);
//                 break;
// #endif
//             }
//         }
//     }
//     return nextHook;
// }

extern "C" void flushLine16(u16 *line) {
    auto frameLine = frameBuffer + scanlineY * (blit::screen.row_stride >> 1);
    for(u32 x=0; x<screenWidth; ++x){
        frameLine[x] = line[x];
    }

    if (u32(++scanlineY) >= screenHeight) {
        scanlineY = 0;
    }
}

void delay(u32 milli) {
}


void init() {
    void registerAssets();
    void game_update();
    void game_init();

    registerAssets();

    blit::set_screen_mode(blit::ScreenMode::hires, blit::PixelFormat::RGB565);
    frameBuffer = reinterpret_cast<u16*>(blit::screen.data);
    if (!frameBuffer) {
        LOG("No framebuffer\n");
        updateHandler = nullptr;
    } else {
        updateHandler = game_update;
        game_init();
        // showLogo();
    }
}

void update(uint32_t) {
    buttonState<Button::A> = blit::buttons & blit::Button::A;
    buttonState<Button::B> = blit::buttons & blit::Button::B;
    buttonState<Button::C> = blit::buttons & blit::Button::X;
    buttonState<Button::D> = blit::buttons & blit::Button::Y;
    buttonState<Button::Up> = blit::buttons & blit::Button::DPAD_UP;
    buttonState<Button::Down> = blit::buttons & blit::Button::DPAD_DOWN;
    buttonState<Button::Left> = blit::buttons & blit::Button::DPAD_LEFT;
    buttonState<Button::Right> = blit::buttons & blit::Button::DPAD_RIGHT;
}

void render(uint32_t time) {
    timeMS = time;
    if (updateHandler)
        updateLoop();
}


#include "assets.hpp"
void registerAssets() {
    blit::File::add_buffer_file("data/mission_1.i16", mission_1, mission_1_length);
    blit::File::add_buffer_file("data/mission_2.i16", mission_2, mission_2_length);
    blit::File::add_buffer_file("data/mission_3.i16", mission_3, mission_3_length);
    blit::File::add_buffer_file("data/mission_4.i16", mission_4, mission_4_length);
    blit::File::add_buffer_file("data/mission_5.i16", mission_5, mission_5_length);
    blit::File::add_buffer_file("data/mission_6.i16", mission_6, mission_6_length);
    blit::File::add_buffer_file("data/mission_7.i16", mission_7, mission_7_length);
    blit::File::add_buffer_file("data/p0.i8", p0, p0_length);
    blit::File::add_buffer_file("data/p10.i8", p10, p10_length);
    blit::File::add_buffer_file("data/p11.i8", p11, p11_length);
    blit::File::add_buffer_file("data/p12.i8", p12, p12_length);
    blit::File::add_buffer_file("data/p13.i8", p13, p13_length);
    blit::File::add_buffer_file("data/p14.i8", p14, p14_length);
    blit::File::add_buffer_file("data/p15.i8", p15, p15_length);
    blit::File::add_buffer_file("data/p16.i8", p16, p16_length);
    blit::File::add_buffer_file("data/p17.i8", p17, p17_length);
    blit::File::add_buffer_file("data/p18.i8", p18, p18_length);
    blit::File::add_buffer_file("data/p19.i8", p19, p19_length);
    blit::File::add_buffer_file("data/p1.i8", p1, p1_length);
    blit::File::add_buffer_file("data/p20.i8", p20, p20_length);
    blit::File::add_buffer_file("data/p21.i8", p21, p21_length);
    blit::File::add_buffer_file("data/p22.i8", p22, p22_length);
    blit::File::add_buffer_file("data/p23.i8", p23, p23_length);
    blit::File::add_buffer_file("data/p24.i8", p24, p24_length);
    blit::File::add_buffer_file("data/p25.i8", p25, p25_length);
    blit::File::add_buffer_file("data/p26.i8", p26, p26_length);
    blit::File::add_buffer_file("data/p27.i8", p27, p27_length);
    blit::File::add_buffer_file("data/p28.i8", p28, p28_length);
    blit::File::add_buffer_file("data/p29.i8", p29, p29_length);
    blit::File::add_buffer_file("data/p2.i8", p2, p2_length);
    blit::File::add_buffer_file("data/p30.i8", p30, p30_length);
    blit::File::add_buffer_file("data/p31.i8", p31, p31_length);
    blit::File::add_buffer_file("data/p32.i8", p32, p32_length);
    blit::File::add_buffer_file("data/p33.i8", p33, p33_length);
    blit::File::add_buffer_file("data/p34.i8", p34, p34_length);
    blit::File::add_buffer_file("data/p35.i8", p35, p35_length);
    blit::File::add_buffer_file("data/p36.i8", p36, p36_length);
    blit::File::add_buffer_file("data/p37.i8", p37, p37_length);
    blit::File::add_buffer_file("data/p38.i8", p38, p38_length);
    blit::File::add_buffer_file("data/p39.i8", p39, p39_length);
    blit::File::add_buffer_file("data/p3.i8", p3, p3_length);
    blit::File::add_buffer_file("data/p40.i8", p40, p40_length);
    blit::File::add_buffer_file("data/p41.i8", p41, p41_length);
    blit::File::add_buffer_file("data/p42.i8", p42, p42_length);
    blit::File::add_buffer_file("data/p43.i8", p43, p43_length);
    blit::File::add_buffer_file("data/p44.i8", p44, p44_length);
    blit::File::add_buffer_file("data/p45.i8", p45, p45_length);
    blit::File::add_buffer_file("data/p46.i8", p46, p46_length);
    blit::File::add_buffer_file("data/p47.i8", p47, p47_length);
    blit::File::add_buffer_file("data/p48.i8", p48, p48_length);
    blit::File::add_buffer_file("data/p49.i8", p49, p49_length);
    blit::File::add_buffer_file("data/p4.i8", p4, p4_length);
    blit::File::add_buffer_file("data/p50.i8", p50, p50_length);
    blit::File::add_buffer_file("data/p51.i8", p51, p51_length);
    blit::File::add_buffer_file("data/p52.i8", p52, p52_length);
    blit::File::add_buffer_file("data/p53.i8", p53, p53_length);
    blit::File::add_buffer_file("data/p54.i8", p54, p54_length);
    blit::File::add_buffer_file("data/p55.i8", p55, p55_length);
    blit::File::add_buffer_file("data/p5.i8", p5, p5_length);
    blit::File::add_buffer_file("data/p6.i8", p6, p6_length);
    blit::File::add_buffer_file("data/p7.i8", p7, p7_length);
    blit::File::add_buffer_file("data/p8.i8", p8, p8_length);
    blit::File::add_buffer_file("data/p9.i8", p9, p9_length);
}
