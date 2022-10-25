#pragma once

#include <Femto>

inline u32 frame;

class Particles {
public:
    static inline constexpr u32 maxParticles = 50;
    static inline constexpr u32 indexWidth = screenWidth / 32 + 1;
    static inline constexpr u16 color = 0XECA6; // colorFromRGB(0x774611);

    enum class Shape : uint8_t {
        Smoke,
        BigSmoke,
        Dot,
        BigCircle,
        Circle,
        Line,
        Bomb,
        Shield
    } shape;

    struct Particle : public Point2D {
        s16 vx, vy;
        u8 mask, ttl, data;
        Shape shape;
    } buffer[maxParticles];

    static inline u32 index[screenHeight][indexWidth];

    u32 ringHead = 0, ringTail = 0;

    u32 inc(u32& i){
        u32 old = i++;
        if (i == maxParticles) i = 0;
        return old;
    }

    void purge() {
        ringHead = ringTail = 0;
    }

    void insert(const Particle& p, s32 offset = 0) {
        auto& b = buffer[inc(ringHead)];
        b = p;
        b.x += s24q8ToF32(p.vx) * offset;
        b.y += s24q8ToF32(p.vy) * offset;
        if (ringHead == ringTail) {
            inc(ringTail);
        }
    }

    static void clear() {
        for (u32 y = 0; y < screenHeight; ++y) {
            for(u32 x = 0; x < indexWidth; ++x) {
                index[y][x] = 0;
            }
        }
    }

    Particle* find(const Point2D& p, u32 mask, f32 radius) {
        u32 age = 0;
        for( u32 current = ringTail; current != ringHead; inc(current)) {
            auto &c = buffer[current];
            age++;
            if (!(c.mask & mask))
                continue;

            auto sum = radius;
            switch(c.shape){
            case Shape::Dot: sum += 1; break;
            case Shape::Circle: sum += 3; break;
            case Shape::BigCircle: sum += 6; break;
            case Shape::Shield: sum += 20; break;
            case Shape::Bomb:
                if (c.ttl < 60) {
                    sum += c.ttl;
                }
                break;
            case Shape::Smoke: sum += (age >> 4) + 1; break;
            case Shape::BigSmoke: sum += (age >> 3) + 1; break;
            case Shape::Line: sum += 1; break;
            }

            if (sum != radius && (c - p).distanceCheck(sum))
                return &c;
        }
        return nullptr;
    }

    void draw(Particle&& p, u32 age) {
        s32 prevY = round(p.y - Graphics::camera.y);
        s32 prevX = round(p.x - Graphics::camera.x);

        p.x += s24q8ToF32(p.vx);
        p.y += s24q8ToF32(p.vy);
        s32 y = round(p.y - Graphics::camera.y);
        s32 x = round(p.x - Graphics::camera.x);

        if (u32(x) >= screenWidth || u32(y) >= screenHeight)
            return;

        auto shape = p.shape;

        u32 radius = 0;
        u32 stride = 1;

        switch(shape){
        case Shape::Dot: break;
        case Shape::Circle: radius = 3; break;
        case Shape::BigCircle: radius = 6; shape = Shape::Circle; break;
        case Shape::Shield: stride = 2; radius = 20; shape = Shape::Circle; break;
        case Shape::Bomb:
            if (p.ttl < 60) {
                radius = p.ttl += 7;
                shape = Shape::Circle;
            }
            break;
        case Shape::Smoke: radius = (age >> 4) + 1; shape = Shape::Circle; break;
        case Shape::BigSmoke: radius = (age >> 3) + 1; shape = Shape::Circle; break;
        case Shape::Line: radius = 1; break;
        }

        s32 sy = std::max<s32>(y - radius, 0);
        s32 ey = std::min<s32>(screenHeight, sy + radius*2 + 1);
        s32 sx = std::max<s32>(x - radius, 0);
        s32 ex = std::min<s32>(screenWidth, sx + radius*2 + 1);

        if (shape == Shape::Dot && !p.ttl) {
        } else if (shape == Shape::Circle) {
            radius = radius * radius;
            for (s32 i = sy; i < ey; i += stride) {
                for (s32 j = sx; j < ex; j += stride) {
                    u32 d = (y - i) * (y - i) + (x - j) * (x - j);
                    if (d <= radius) {
                        s32 w = j >> 5, b = j & 0x1F;
                        index[i][w] |= 1 << b;
                    }
                }
            }
        } else if (shape == Shape::Line || shape == Shape::Dot) {
            if (shape == Shape::Dot) {
                p.ttl--;
            } else {
                for (s32 i = sy; i < ey; ++i) {
                    for (s32 j = sx; j < ex; ++j) {
                        if (u32(j) < screenWidth && u32(i) < screenHeight) {
                            s32 w = j >> 5, b = j & 0x1F;
                            index[i][w] |= 1 << b;
                        }
                    }
                }
            }

            if (y > prevY) {
                s32 t = prevY;
                prevY = y;
                y = t;

                t = prevX;
                prevX = x;
                x = t;
            }

            s32 dy = prevY - y;
            s32 dx = prevX - x;
            s32 step = x < prevX ? 1 : -1;

            if (dy > abs(dx)) {
                s32 error = std::abs(dx << 15) / dy;
                for (s32 i = y; i < prevY; ++i) {
                    if (u32(x) < screenWidth && u32(i) < screenHeight) {
                        s32 w = x >> 5, b = x & 0x1F;
                        index[i][w] |= 1 << b;
                    }
                    if ((((i - 1 - prevY) * u32(error) + (1<<14)) >> 15) != (((i - prevY) * u32(error) + (1<<14)) >> 15)) {
                        x += step;
                    }
                }
            } else if (dx) {
                s32 error = (dy << 15) / std::abs(dx);
                s32 total = std::abs(dx);
                for (s32 i = y; i <= prevY; ++i) {
                    s32 acc = error * total;
                    s32 end = (acc + (1<<14)) >> 15;
                    for (; total && ((acc + (1<<14)) >> 15) == end; acc -= error){
                        total--;
                        s32 cx = x;
                        x += step;
                        if (u32(cx) < screenWidth && u32(i) < screenHeight) {
                            s32 w = cx >> 5, b = cx & 0x1F;
                            index[i][w] |= 1 << b;
                        };
                    }
                }
            }
        }
    }

    void update(){
        u32 age = 0;
        for( u32 current = ringTail; current != ringHead; inc(current)) {
            draw(std::move(buffer[current]), ++age);
        }
    }

    void operator () (u16* line, u32 y) {
        if (y == 0) update();
        for (u32 w = 0; w < indexWidth; ++w) {
            u32 c = index[y][w];
            if (!c) continue;
            u32 x = w << 5;
            for (u32 b = 0; b <= 0x1F; ++b, ++x) {
                if (c & (1 << b))
                    line[x] += color;
            }
        }
    }
};
