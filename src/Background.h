#pragma once

#include <Femto>
#include <layers/Tiles.hpp>

#include "../img-src/bg.h"
/*
class Background : public Graphics::layer::Tiles<64, 64> {
public:
    Background() :
        Graphics::layer::Tiles<64, 64>(Graphics::layer::TileSource{this})
        {}

    using Tiles<64, 64>::operator();

    s32 X, Y, oldY, oldX;

    Graphics::layer::TileCopy operator () (s32 x, s32 y) {
        if (y != Y) {
            X = oldX = x;
            Y = oldY = (y & 0x3F) * 64;
        }
        return {this};
    }

    void operator () (u16* line, s32 x, s32 y, u32 width) {
        auto bmp = reinterpret_cast<const u16*>(bg + 2);
        if (y != oldY) {
            oldY = y;
            X = oldX;
        }
        auto overlay = (bmp[Y + (X++ & 0x3F)] & 0xF7DF) >> 1; // ~0x8410;
        bmp += y * width + x + (width - x);
        line += width - x;
        for( s32 i = -width + x; i < 0; ++i ){
            line[i] = ((bmp[i] & 0xF7DF) >> 1) + overlay;
        }
    }
};
*/

class Background {
public:
    s32 gy, gx;
    const u16* tile = reinterpret_cast<const u16*>(bg + 2);
    void init(const u8* tile){
        gy = round(-Graphics::camera.y);
        gx = round(Graphics::camera.x);
        this->tile = reinterpret_cast<const u16*>(tile + 2);
    }

    void operator () (u16* line, s32 y) {
        y -= gy;
        auto overlay = &tile[y & (63 << 6)];
        auto bmp = &tile[(y & 63) * 64];
        line += screenWidth;
        s32 x = gx;
        for (s32 i = -screenWidth; i < 0;) {
            line[i++] = bmp[x&63] + overlay[(x >> 6)&63]; ++x;
            line[i++] = bmp[x&63] + overlay[(x >> 6)&63]; ++x;
            line[i++] = bmp[x&63] + overlay[(x >> 6)&63]; ++x;
            line[i++] = bmp[x&63] + overlay[(x >> 6)&63]; ++x;
        }
    }
};
