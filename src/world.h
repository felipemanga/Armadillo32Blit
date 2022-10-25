#pragma once

#include "../img-src/bgGaiaX34.h"
#include "../img-src/bgFreyzeX672.h"
#include "../img-src/bgMagmicZX41.h"

struct WorldFeature {
    Point2D position;
    const char* name;
    u8 image;
    u8 targetWorld;
    u8 scale;
};

struct World {
    const u8* tile;
    const u32 level;
    const WorldFeature* features;
    const u32 featureCount;
    void (*spawn)(u32 shipId, Ship&, const World&);
};

using MissionCheck = bool (*)();

struct Mission {
    const char* message;
    s16 x, y;
    u8 worldId, npcFaction;
    MissionCheck update, npcDied, npcInteract;
};

constexpr inline const WorldFeature spaceFeatures[] = {
    {{-14.0f, -13.0f}, "Seneus", 7, 0, 2},
    {{509, 392}, "Mazion", 45, 2, 1},
    {{-658.0f, 1000}, "Atus", 2, 3, 1},
    {{29, 1700}, "Baanella", 3, 4, 1},
    {{167, -2320.0f}, "La Tashtronia", 4, 5, 2},
    {{-2462.0f, 1317}, "Diaronia", 5, 6, 1},
    {{2054, 2755}, "Kaanuridian", 6, 7, 2},
    {{3628, -1517.0f}, "Omega Vivopia", 0, 8, 1},
    {{3616, 2781}, "Bireus Anaddon", 8, 9, 1},
    {{-155.0f, -5169.0f}, "Senillian Patalis", 9, 10, 1},
    {{-5653.0f, 1180}, "Biarius Depalis", 10, 11, 2},
    {{-5337.0f, 3260}, "Amonaldi II", 11, 12, 1},
    {{-5604.0f, 3950}, "Cairu", 12, 13, 2},
    {{-1457.0f, 7239}, "Khannor", 13, 14, 2},
    {{-764.0f, 7991}, "Portos", 14, 15, 1},
    {{8489, -273.0f}, "Sononine", 15, 16, 2},
    {{6777, 6222}, "Astaroros", 16, 17, 2},
    {{9274, -2797.0f}, "Papviz", 17, 18, 2},
    {{-7757.0f, 6851}, "Kepillian", 18, 19, 2},
    {{1815, -10705.0f}, "Prato", 19, 20, 1},
    {{553, -11441.0f}, "Vopchin", 20, 21, 2},
    {{10938, 5032}, "Mazlon", 21, 22, 1},
    {{-6054.0f, -10969.0f}, "Zerol", 22, 23, 2},
    {{12860, 3030}, "Churchil", 23, 24, 2},
    {{-867.0f, -13719.0f}, "Vadopia", 24, 25, 1},
    {{-13306.0f, -5118.0f}, "Sunacan", 25, 26, 1},
    {{-13202.0f, -6919.0f}, "Meraka", 26, 27, 2},
    {{-5593.0f, -14364.0f}, "Xeopia", 27, 28, 2},
    {{-8338.0f, -13624.0f}, "Ladburto", 28, 29, 2},
    {{14066, -8909.0f}, "Xelon V", 29, 30, 1},
    {{-12045.0f, -12147.0f}, "Ploum", 30, 31, 1},
    {{14154, -10751.0f}, "Cerminiar", 31, 32, 1},
    {{-5071.0f, 17580}, "Tafaka", 32, 33, 2},
    {{2950, -18660.0f}, "Ketankor", 33, 34, 1},
    {{-14608.0f, 12747}, "Sattaria", 34, 35, 1},
    {{2107, 19920}, "Sienrock", 35, 36, 2},
    {{-3297.0f, 20290}, "Cunicus", 36, 37, 1},
    {{-21045.0f, 1192}, "Fogthra", 37, 38, 1},
    {{-9146.0f, -19698.0f}, "Banenella", 38, 39, 2},
    {{5198, 21672}, "Kettose", 39, 40, 2},
    {{-22300.0f, -5347.0f}, "Broeaux", 40, 41, 1},
    {{8543, 21858}, "Dokoros", 41, 42, 1},
    {{24013, 1712}, "La Vopanbula", 42, 43, 1},
    {{20362, 13893}, "Ozarth", 43, 44, 1},
    {{-23142.0f, 9911}, "Tacza", 44, 45, 1},
    {{-22148.0f, -12955.0f}, "Satox", 1, 46, 1},
    {{7625, -25190.0f}, "Outonia", 46, 47, 1},
    {{24933, 10038}, "Khanebus", 47, 48, 1},
    {{-23650.0f, 14012}, "Kuornia", 48, 49, 2},
    {{-20031.0f, 19452}, "Bion", 49, 50, 2},
    {{-24422.0f, 14921}, "Carlas", 50, 51, 1},
    {{20442, -20714.0f}, "Pluangolia", 51, 52, 2},
    {{-20478.0f, 21578}, "Camperan", 52, 53, 2},
    {{-24387.0f, 18050}, "Cartronia", 53, 54, 2},
    {{-17856.0f, -25250.0f}, "Bopa", 54, 55, 1},
    {{15023, -27553.0f}, "Hubnor", 55, 56, 2}
};

bool passMission(){return true;}
template<int> bool checkKillCount();

constexpr inline const Mission missions[] = {
    { // Intro mission, Seneus
        .update = passMission
    },
    { // Talk to Captain, Seneus
        .npcFaction = 1,
        .npcInteract = passMission
    },
    { // Defeat boss, Space (near Mazion)
        .x = 500, .y = 200,
        .worldId = 1,
        .npcFaction = 2,
        .npcDied = passMission
    },
    { // Liberate Moon, Mazion
        .message = "Fight: Mazion",
        .worldId = 2,
        .update = checkKillCount<10>
    },
    { // Liberate Moon part 2, Mazion
        .message = "Part 2: Mazion",
        .worldId = 2,
        .update = checkKillCount<20>
    },
    {
        .worldId = 3,
        .npcFaction = 1,
        .npcInteract = passMission
    },
    {.message = "End"}
};

constexpr inline const u32 missionCount = sizeof(missions) / sizeof(missions[0]);

u32 worldKillCount();

inline void spawnAlienWorld(u32 id, Ship& ship, const World& world) {
    ship.special = id == 1 ? Ship::Special::Boss : Ship::Special::Normal;

    u32 enemyCount = (world.level + 10) * 30;
    if (worldKillCount() < enemyCount)
        ship.setFaction(1 + (id & 1));
    else
        ship.setFaction(1);

    ship.spawn();
    ship.state = Ship::State::AIFlee;
}

constexpr inline World worlds[] = {
//Seneus
    {
        .tile=bgGaiaX34,
        .level=0,
        .spawn=[](u32 id, Ship& ship, const World& world){
            ship.special = Ship::Special::Normal;
            if (id == 1) ship.special = Ship::Special::Boss;
            else if (id == 2) ship.special = Ship::Special::Medic;
            else if (id == 3) ship.special = Ship::Special::PodGuy;
            else if (id == 4) ship.special = Ship::Special::Shop;

            if (worldKillCount() < 10) {
                ship.setFaction((((getTime() >> 5) & 7) == 0) ? 1 + (ship.special == Ship::Special::Normal) : 1);
            } else {
                ship.setFaction(1);
            }

            ship.spawn();
            ship.state = Ship::State::AIFlee;
        }
    },

// space
    {
        .tile=bg,
        .level=0,
        .features=spaceFeatures,
        .featureCount=ARRAY_LENGTH(spaceFeatures),
        .spawn=[](u32 id, Ship& ship, const World& world){
            if (id == 1) ship.special = Ship::Special::Boss;
            else if (id == 2 && (random() & 7) == 0) ship.special = Ship::Special::Medic;
            else if (id == 2 && (random() & 7) == 0) ship.special = Ship::Special::PodGuy;
            else ship.special = Ship::Special::Normal;
            ship.setFaction(1 + (id != 2));
            ship.spawn();
            ship.state = Ship::State::AIFlee;
        }
    },

// Mazion
    {.tile=bgFreyzeX672, .level=0},

// "Atus"
    {.tile=bgMagmicZX41, .level=2},

// "Baanella"
    {.tile=bgMagmicZX41, .level=2},

// "La Tashtronia"
    {.tile=bgMagmicZX41, .level=2},

// "Dikaronia 280"
    {.tile=bgMagmicZX41, .level=2},

// "Kaanuridian 9"
    {.tile=bgMagmicZX41, .level=2},

// "Omega Vivopia"
    {.tile=bgMagmicZX41, .level=2},

// "Bireus Anaddon"
    {.tile=bgMagmicZX41, .level=2},

// "Senillian Patalis"
    {.tile=bgMagmicZX41, .level=2},

// "Biarius Depalis"
    {.tile=bgMagmicZX41, .level=2},

// "Ciurn Amonaldi II"
    {.tile=bgMagmicZX41, .level=2},

// "Ceti Cairu 35"
    {.tile=bgMagmicZX41, .level=2},

// "Khannor"
    {.tile=bgMagmicZX41, .level=2},

// "Portos 305"
    {.tile=bgMagmicZX41, .level=2},

// "Sononine Sigma"
    {.tile=bgMagmicZX41, .level=2},

// "Astaroros e4"
    {.tile=bgMagmicZX41, .level=2},

// "Papviz Ariaelialia"
    {.tile=bgMagmicZX41, .level=2},

// "Kepillian 150"
    {.tile=bgMagmicZX41, .level=2},

// "Prato Bandania"
    {.tile=bgMagmicZX41, .level=2},

// "Vopchin Vadopia"
    {.tile=bgMagmicZX41, .level=2},

// "Mazlon"
    {.tile=bgMagmicZX41, .level=2},

// "Zerol Elax"
    {.tile=bgMagmicZX41, .level=2},

// "Churchil"
    {.tile=bgMagmicZX41, .level=2},

// "Alpha Vadopia"
    {.tile=bgMagmicZX41, .level=2},

// "Sunacan Poolaka"
    {.tile=bgMagmicZX41, .level=2},

// "Meraka"
    {.tile=bgMagmicZX41, .level=2},

// "Theta Xeopia"
    {.tile=bgMagmicZX41, .level=2},

// "New Ladburto"
    {.tile=bgMagmicZX41, .level=2},

// "Xelon V"
    {.tile=bgMagmicZX41, .level=2},

// "Ploum Gamma"
    {.tile=bgMagmicZX41, .level=2},

// "Cerminiar Kep‘am"
    {.tile=bgMagmicZX41, .level=2},

// "Tafaka 293"
    {.tile=bgMagmicZX41, .level=2},

// "Ketankor Moon"
    {.tile=bgMagmicZX41, .level=2},

// "Sattaria"
    {.tile=bgMagmicZX41, .level=2},

// "Alpha Sizenrock"
    {.tile=bgMagmicZX41, .level=2},

// "Epsilon Cunicus"
    {.tile=bgMagmicZX41, .level=2},

// "Fogthra"
    {.tile=bgMagmicZX41, .level=2},

// "Tau Banenella"
    {.tile=bgMagmicZX41, .level=2},

// "Kettose"
    {.tile=bgMagmicZX41, .level=2},

// "Broeaux"
    {.tile=bgMagmicZX41, .level=2},

// "Than Dokoros Planetara"
    {.tile=bgMagmicZX41, .level=2},

// "La Vopanbula"
    {.tile=bgMagmicZX41, .level=2},

// "Ozarth 6"
    {.tile=bgMagmicZX41, .level=2},

// "Tacza 378"
    {.tile=bgMagmicZX41, .level=2},

// "Satox"
    {.tile=bgMagmicZX41, .level=2},

// "Winaroid Outonia 323"
    {.tile=bgMagmicZX41, .level=2},

// "Banongolia Khanebus"
    {.tile=bgMagmicZX41, .level=2},

// "Kuornia"
    {.tile=bgMagmicZX41, .level=2},

// "Bion"
    {.tile=bgMagmicZX41, .level=2},

// "Ciebus Carlas"
    {.tile=bgMagmicZX41, .level=2},

// "Pluangolia"
    {.tile=bgMagmicZX41, .level=2},

// "Camperan"
    {.tile=bgMagmicZX41, .level=2},

// "Sulchin Cartronia"
    {.tile=bgMagmicZX41, .level=2},

// "B’opa 256"
    {.tile=bgMagmicZX41, .level=2},

// "Hubnor"
    {.tile=bgMagmicZX41, .level=2}

};

constexpr inline uint32_t worldCount = sizeof(worlds) / sizeof(worlds[0]);
inline u32 world;

class Universe {
public:

    u8 ownedItems[itemCount] = {};
    u8 worldKillCount[worldCount] = {};
    u32 missionId = 0;
    u32 shotsFired = 0;
    u32 shotsHit = 0;

    SERIALIZE("data/ArmadilloWorld.sav") {
        PROPERTY(ownedItems);
        PROPERTY(worldKillCount);
        PROPERTY(missionId);
        PROPERTY(shotsFired);
        PROPERTY(shotsHit);
    }

} inline universe;

inline constexpr const World& spaceWorld = worlds[1];
inline Point2D prevPosition;

void activateFeature(uptr id);
void renderWorld(const World& world);

inline u32 worldKillCount() {
    return universe.worldKillCount[world];
}

inline void addKill() {
    if (universe.worldKillCount[world] == 0xFF)
        return;
    universe.worldKillCount[world]++;
    universe.save();
}

inline void addItem(u32 item) {
    if (item >= itemCount || universe.ownedItems[item] == 0xFF)
        return;
    universe.ownedItems[item]++;
}

inline void missionNPCInteract() {
    if (auto cb = missions[universe.missionId].npcInteract; cb && cb()) {
        nextMission();
    }
}

inline void missionNPCDied() {
    if (auto cb = missions[universe.missionId].npcDied; cb && cb())
        nextMission();
}

template<int amount>
bool checkKillCount() {
    return universe.worldKillCount[missions[universe.missionId].worldId] >= amount;
}
