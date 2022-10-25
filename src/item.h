#pragma once

struct Item {
    const char* name;
    s32 price = 1000;
    s32 addHP = 0;
    f32 mulHP = 1;
    f32 regHP = 0;
    f32 addDmg = 0;
    f32 mulDmg = 1;
    f32 mulFireError = 1;
    f32 addShield = 0;
    f32 mulShield = 1;
    s32 addPod = 0;
    s32 addFireRate = 0;
};

inline constexpr Item items[] = {
    {
        .name = "Health Booster",
        .price = 500,
        .addHP = 50
    },
    {
        .name = "Health Booster+",
        .price = 750,
        .addHP = 70
    },
    {
        .name = "Health Regen",
        .price = 1000,
        .regHP = f32(1.0f/30.0f)
    },
    {
        .name = "Cannon Scope",
        .price = 800,
        .mulFireError = f32(0.8)
    },
    {
        .name = "Cannon Scope+",
        .price = 1200,
        .mulFireError = f32(0.7)
    },
    {
        .name = "Rapid Fire",
        .price = 400,
        .addFireRate = -20
    },
    {
        .name = "Rapid Fire+",
        .price = 800,
        .addFireRate = -50
    },
    {
        .name = "Shield Booster",
        .price = 250,
        .addShield = 50
    },
    {
        .name = "Shield Booster+",
        .price = 450,
        .addShield = 100
    },
    {
        .name = "Shield Ex",
        .price = 1000,
        .mulShield = 2
    },
    {
        .name = "Pod slot",
        .price = 2000,
        .addPod = 1
    },
    {
        .name = "Pod slot 2",
        .price = 3000,
        .addPod = 2
    },
    {
        .name = "Pod slot 4",
        .price = 4000,
        .addPod = 4
    }
};

inline constexpr u32 itemCount = ARRAY_LENGTH(items);
