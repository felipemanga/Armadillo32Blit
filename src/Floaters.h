#pragma once

class Floaters {
public:
    static inline constexpr u32 maxFloaters = 32;
    struct Floater : public Point2D {
        Point2D* target;
    } positions[maxFloaters];

    enum Type : u8 {
        None,
        Credits,
    } types[maxFloaters];

    void addFloater(Point2D position, Point2D *target, Type type) {
        Point2D ref = Graphics::camera + Point2D{f32(screenWidth/2), f32(screenHeight/2)};
        u32 longest = 0;
        u32 id = 0;
        for (u32 i = 0; i < maxFloaters; ++i) {
            u32 distance = (positions[i] - ref).lengthSquared();
            if (distance >= longest) {
                longest = distance;
                id = i;
            }
        }

        positions[id] = Floater{
            position,
            target
        };

        types[id] = type;
    }

    void draw(){
        for (u32 id = 0; id < maxFloaters; ++id) {
            switch(types[id]) {
            case None:
                break;
            case Credits: {
                auto delta = positions[id] - *positions[id].target;
                f32 magnitude = std::max(abs(delta.x), abs(delta.y));
                if (magnitude < 8) {
                    types[id] = None;
                    break;
                }

                positions[id] -= (delta * 4) / magnitude;
                Graphics::draw(BitmapFrame<8>(credits, (frame >> 1) & 3), positions[id]);
                break;
            }
            }
        }
    }

    void clear() {
        for (u32 i = 0; i < maxFloaters; ++i) {
            types[i] = None;
        }
    }
} floaters;
