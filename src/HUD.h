#pragma once

#include <Femto>

extern const char specialHUDSym[];
inline constexpr s32 HUDSize = 75;
inline constexpr s32 MaxHUD = (HUDSize/2)*(HUDSize/2);

inline void drawHUD(f32 bar1, u32 bar1Color, f32 bar2, u32 bar2Color, u32 life) {
    using namespace Graphics;
    using namespace _drawListInternal;

    static f32 pbar1, pbar2;

    if (bar1 > 1)
        bar1 = 1;
    else if (bar1 < 0)
        bar1 = 0;

    if (bar2 > 1)
        bar2 = 1;
    else if (bar2 < 0)
        bar2 = 0;

    pbar1 -= (pbar1 - bar1) * f32(0.2);
    pbar2 -= (pbar2 - bar2) * f32(0.2);

    draw_t f = [](u16 *line, Cmd &s, u32 y){
        y++;
        u32 Y = (y-HUDSize/2) * (y-HUDSize/2);
        line += HUDSize + s.x;
        s32 i = -HUDSize;
        u16 c;

        for(; i < 0; ++i){
            s32 d = (i+HUDSize/2)*(i+HUDSize/2)+Y;
            if (d < MaxHUD)
                break;
        }

        c = ((HUDSize - y) < s.b1) ? reinterpret_cast<uptr>(s.data) : 0;
        line[i - 2] = c;
        line[i - 1] = c;

        u16 bgc = ((y - 1) & 7) == 0 ? colorFromRGB(0x001F00) : colorFromRGB(0x001100);

        for(; i < 0; ++i){
            s32 d = (i+HUDSize/2)*(i+HUDSize/2)+Y;
            if (d >= MaxHUD) break;
            line[i] = ((line[i] & 0xF7DF) >> 1) | bgc;
            if (((i + 2) & 7) == 0) {
                line[i] |= colorFromRGB(0x001F00);
            }
        }

        c = ((HUDSize - y) < s.b2) ? s.s : 0;
        line[i++] = c;
        line[i] = c;
    };

    gameRenderer->bind<HUDLayer>();

    Cmd cmd = {
        .data = reinterpret_cast<void*>((uptr)bar1Color),
        .draw = f,
        .x = 3,
        .y = 3,
        .maxY = HUDSize - 2,
        .b1 = decl_cast(Cmd::b1, round(pbar1 * HUDSize)),
        .b2 = decl_cast(Cmd::b2, round(pbar2 * HUDSize))
    };

    cmd.s = decl_cast(Cmd::s, bar2Color);

    add(cmd);

    setCursor(0, 0);
    primaryColor = 0xFFFF;
    print(getFPS());
}

template<typename Type>
inline void addToHUD(Point2D p, u32 color, Type type) {
    gameRenderer->bind<HUDLayer>();
    p *= f32(1.0f / 16.0f);
    p += HUDSize/2;
    if ((p - Point2D{HUDSize/2, HUDSize/2}).lengthSquared() >= MaxHUD)
        return;

    p += Graphics::camera;

    Graphics::setCursor(p);
    Graphics::primaryColor = color;

    auto sym = specialHUDSym[(int)type];
    if (!sym) {
        f32 size = type == Type::Boss ? 2 : 1;
        p += f32(3);
        Graphics::fillRect(p - size, p + size, color);
    } else {
        Graphics::print(sym);
    }
}
