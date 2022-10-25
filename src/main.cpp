#include <Femto>
#include <SFXVolumeSource.hpp>
#include "Background.h"
#include "Particles.h"
#include "Serialize.hpp"
#include "drawMesh.h"
#include "Renderer.h"
#include "HUD.h"
#include "miloslav.h"
#include "../img-src/lifeCountUI.h"
#include "../img-src/podCountUI.h"
#include "../img-src/credits.h"
#include "../img-src/logo.h"
#include "../sound/hit.h"
#include "../sound/engine.h"
#include "../sound/shoot.h"
#include "../sound/explosion.h"

f32 shake;
void enterWorld(u32 worldId);
const c8* streamedEffect = nullptr;
u32 effectPriority = 0;
char missionText[100];

using cAction_t = void (*)(uptr);
bool ignoreAction = false;
void cActionNOP(uptr) {}
cAction_t cAction = cActionNOP;
uptr cActionArg;
const c8* featureCaption;

enum class GameState {
    Start,
    Logo,
    Space,
    EnterShop,
    Shop,
    EnterCutScene,
    CutScene
} gameState = GameState::Start;

s32 backlight = 0, targetBacklight = 255;
void fadeOut() {
    for (s32 i = backlight; i >= 0; i -= 11) {
        setBacklight(s24q8ToF32(i));
        delay(3);
    }
    backlight = 0;
    setBacklight(backlight);
}

bool updateEnter(GameState state) {
    targetBacklight = 0;
    if (backlight != 0)
        return false;
    targetBacklight = 255;
    gameState = state;
    return true;
}

void respawnNPC(u32 id);
void addKill();
void addItem(u32);

void missionNPCInteract();
void missionNPCDied();
void nextMission();

#include "meshes.h"
#include "Floaters.h"
#include "item.h"
#include "ship.h"
#include "world.h"
#include "shop.h"
#include "cutscene.h"

Audio::Sink<7, 10000> audio;
Point2D refCamera;

void updateCamera(f32 speed) {
    auto target = Ship::player->position - Point2D{screenWidth/2, screenHeight/2};
    refCamera -= (refCamera - target) * speed;
    Graphics::camera = refCamera + (Point2D{shake, 0}).rotateXY(frame * f32(2.5f));
}

void respawnNPC(u32 id) {
    auto& w = worlds[world];
    if (ships[id].isPlayer())
        return;
    auto& mission = missions[universe.missionId];
    if (id == 1 && world == mission.worldId && mission.npcFaction != 0) {
        Point2D point{
            f32(mission.x),
            f32(mission.y)
        };
        if ((Ship::player->position - point).distanceCheck(screenWidth)) {
            auto& ship = ships[id];
            ship.setFaction(mission.npcFaction);
            ship.spawn();
            ship.state = Ship::State::AIFlee;
            ship.special = Ship::Special::Mission;
            return;
        }
    }
    (w.spawn ?: spawnAlienWorld)(id, ships[id], w);
}

void updateMissionText() {
    auto& mission = missions[universe.missionId];
    // snprintf(missionText, sizeof(missionText), "%d, %d", int(mission.worldId), int(world));
    // return;

    if (mission.worldId == world) {
        Ship::missionTarget = Point2D{f32(mission.x), f32(mission.y)};
        snprintf(missionText, sizeof(missionText), mission.message ?: "%d, %d", int(mission.x), int(mission.y));
        return;
    }
    auto& w = worlds[world];
    for (u32 i = 0; i < w.featureCount; ++i) {
        auto& feature = w.features[i];
        if (feature.targetWorld == mission.worldId) {
            Ship::missionTarget = feature.position;
            snprintf(missionText, sizeof(missionText), "%s (%d, %d)", feature.name, int(feature.position.x), int(feature.position.y));
            return;
        }
    }
    snprintf(missionText, sizeof(missionText), "Space");
}

void nextMission() {
    if (universe.missionId >= missionCount) {
        return;
    }
    LOG("Mission ", universe.missionId, "\n");
    universe.missionId++;
    universe.save();
    gameState = GameState::EnterCutScene;
    updateMissionText();
    auto& mission = missions[universe.missionId];
    if (mission.worldId == world && mission.npcFaction) {
        ships[1].special = Ship::Special::Mission;
        if (ships[1].faction != mission.npcFaction)
            ships[1].HP = 0;
    } else {
        ships[1].special = Ship::Special::Normal;
    }
    Ship::player->addExp(universe.missionId * universe.missionId * 8);
}

void enterWorld(u32 worldId) {
    if (worldId == world || worldId >= worldCount)
        return;

    fadeOut();
    shots.purge();
    gameRenderer->get<Particles>().purge();

    floaters.clear();
    auto backup = prevPosition;
    prevPosition = Ship::player->position;
    Ship::player->position = backup;
    Ship::player->stop();
    world = worldId;
    for (u32 i = 0; i < shipCount; ++i)
        respawnNPC(i);

    updateCamera(f32(1.0f));
    updateMissionText();
}

void game_init(){
    Graphics::textMode = Graphics::TextMode::Clip;
    Graphics::palette = miloslav;
    // setMaxFPS(30);
    Graphics::primaryColor = colorFromRGB(0xFFFFFF);
    world = 1;
    Serialize::init();
    gameRenderer = &std::get<GameRenderer>(renderer);
    activateFeature(0);
    LOG("Free RAM: ", getFreeRAM(), "\n");
}

void activateFeature(uptr id) {
    if (id != ~uptr{}) {
        const auto& feature = worlds[world].features[id];
        enterWorld(feature.targetWorld);
    } else {
        enterWorld(1);
    }
}

u32 loadedFeature = ~u32{};
u8 featureData[64*64];

void renderWorld(const World& world) {
    if (&world == &spaceWorld) {
        cAction = cActionNOP;
        featureCaption = nullptr;
    } else {
        cAction = activateFeature;
        cActionArg = ~uptr{};
        featureCaption = "Go to space";
    }

    Graphics::palette = Graphics::generalPalette;

    for (u32 i = 0; i < world.featureCount; ++i) {
        using namespace Graphics;
        auto& feature = world.features[i];
        f32 radius = 32 * feature.scale;
        auto position = feature.position - Point2D{radius, radius};
        if ((feature.position - Ship::player->position).distanceCheck(radius)) {
            cAction = activateFeature;
            cActionArg = i;
            featureCaption = feature.name;
        }
        bool drawn = false;
        if (feature.scale > 1)
            drawn = draw<true, true>(BitmapFrame<8>{featureData - 2, 64, 64}, position);
        else
            drawn = draw<true, false>(BitmapFrame<8>{featureData - 2, 64, 64}, position);

        if (!drawn)
            continue;

        if (loadedFeature != feature.image) {
            loadedFeature = feature.image;
            File file;
            char buff[100];
            snprintf(buff, sizeof(buff), "data/p%d.i8", loadedFeature);
            LOG("Loading ", buff, "\n");
            if (!file.openRO(buff)) {
                LOG("FAILED\n");
            }
            LOG("Read: ", file.read(featureData), "\n");
        }

        break;
    }

    Graphics::palette = miloslav;

    floaters.draw();

    if (backlight < 255 || Ship::player->isDead()) {
        cAction = cActionNOP;
        featureCaption = nullptr;
    }
}

void updateSpace() {
    using namespace Graphics;
    usedBufferCount = 0;

    {
        auto hp = Ship::player->HP / Ship::player->maxHP;
        drawHUD(
            hp,
            hp > f32(0.5f) ? colorFromRGB(0xFFFFFF) : colorFromRGB(0xFF0000),
            Ship::player->MP / Ship::player->maxMP,
            colorFromRGB(0x007F00),
            Ship::playerLifeCount
            );
    }

    gameRenderer->bind<PlanetsLayer>();
    clear();
    renderWorld(worlds[world]);

    gameRenderer->bind<ShipsLayer>();
    clear();

    if (targetBacklight == 255) {
        Ship::broadcast(&Ship::update, 0);
        shots.update();

        if (auto update = missions[universe.missionId].update; update && update())
            nextMission();

        if (isPressed(Button::C)) {
            if (!ignoreAction)
                cAction(cActionArg);
            ignoreAction = true;
        } else
            ignoreAction = false;
    }

    Ship::broadcast(&Ship::postUpdate, frame);

    if (featureCaption) {
        gameRenderer->bind<HUDLayer>();
        Graphics::primaryColor = colorFromRGB(0xFFFFFF);
        Graphics::setCursor(5, screenHeight - 10);
        Graphics::print("Y= ", featureCaption);
    }

    auto& mission = missions[universe.missionId];
    if ((mission.worldId == world && ships[1].special == Ship::Special::Mission) || world == 1) {
        bool hasTargetShip = mission.worldId == world && mission.npcFaction && ships[1].special == Ship::Special::Mission;
        auto delta = (hasTargetShip ? ships[1].position : Ship::missionTarget) - Ship::player->position;
        if (delta.lengthSquared() > f32(s32(screenWidth/2) * s32(screenWidth/2))) {
            auto angle = atan2(delta.y, delta.x);
            Graphics::primaryColor = colorFromRGB(0x00FF00);
            Point2D position = Ship::player->position + Point2D{cos(angle) * f32(20), sin(angle) * f32(20)};
            Graphics::setCursor(position);
            Graphics::print("!");
        }
    }

    shake *= f32(0.93f);
    updateCamera(f32(0.1));
    gameRenderer->get<Background>().init(worlds[world].tile);
}

void updateStart(){
    // Audio::setVolume(0);
    if (auto music = Audio::play("music/bensound-birthofahero.raw")) {
        music->setLoop(true);
        LOG("Music started\n");
    } else LOG("Music not found\n");
    gameState = GameState::Logo;
    backlight = 0;
    targetBacklight = 255;
    universe.load();
    // universe.missionId = 5;
    updateMissionText();
    if (Ship::player->load()) {
        LOG("Loaded with exp ", Ship::player->exp, "\n");
    } else {
        LOG("No save file\n");
    }
    Ship::player->calcStats(Ship::player->exp, true);
    Ship::player->podRecharge(true);
}

void updateLogo(){
    using namespace Graphics;
    gameRenderer->bind<HUDLayer>();
    BitmapFrame<8> bmp{logo};
    draw(bmp, screenWidth / 2 - bmp.width() / 2, screenHeight / 2 - bmp.height() / 2, std::max(0, int(frame) - 30) * f32(0.01f));
    camera.y = -f32(frame);
    gameRenderer->get<Background>().init(bg);

    if (targetBacklight == 255) {
        if (isPressed(Button::A) || isPressed(Button::B) || isPressed(Button::C))
            targetBacklight = 0;
    } else {
        if (backlight == 0) {
            gameState = GameState::Space;
            targetBacklight = 255;
        }
    }
}

void game_update(){
    using namespace Graphics;

    frame++;
    if (frame%60 == 0){
        LOG(getFPS(), "\n");
    }
    Particles::clear();
    gameRenderer->bind<HUDLayer>();
    clear();

    if (backlight != targetBacklight) {
        if (backlight > targetBacklight) {
            backlight -= 11;
            if (backlight < targetBacklight)
                backlight = targetBacklight;
        } else {
            backlight += 11;
            if (backlight > targetBacklight)
                backlight = targetBacklight;
        }

        if (backlight > 255) backlight = 255;
        setBacklight(s24q8ToF32(backlight));
    }

    if (streamedEffect) {
        static Audio::RAWFileSource* source = nullptr;
        static u32 lastSoundTime = 0;
        auto now = getTime();
        if ((!source || source->ended()) && (now - lastSoundTime > effectPriority)) {
            source = Audio::play<6>(streamedEffect);
            if (source) source->setLoop(false);
            lastSoundTime = now;
            effectPriority = ~u32{};
        }
        streamedEffect = nullptr;
    }
    switch (gameState) {
    case GameState::Start: updateStart(); break;
    case GameState::Logo: updateLogo(); break;
    case GameState::Space: updateSpace(); break;
    case GameState::EnterShop: updateEnterShop(); break;
    case GameState::Shop: updateShop(); break;
    case GameState::EnterCutScene: updateEnterCutScene(); break;
    case GameState::CutScene: updateCutScene(); break;
    }
}
