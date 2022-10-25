#pragma once

#include "../meshes/Craft_racer.h"
#include "../meshes/Craft_speederA.h"
#include "../meshes/Craft_speederB.h"
#include "../meshes/Craft_speederC.h"
#include "../meshes/Craft_speederD.h"
#include "../meshes/Craft_miner.h"
#include "../meshes/Craft_cargoA.h"
#include "../meshes/Craft_cargoB.h"

constexpr const u8* const shipMeshes[] = {
    craft_miner,
    craft_racer,
    craft_cargoA,
    craft_cargoB,
    craft_speederA,
    craft_speederB,
    craft_speederC,
    craft_speederD
};

constexpr const u32 shipMeshCount = sizeof(shipMeshes) / sizeof(shipMeshes[0]);
