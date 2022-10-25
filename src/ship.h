#pragma once
#include "engine/api.hpp"
#include <Femto>

constexpr u32 shipCount = 8;

constexpr inline const char* specialCaption[] = {
    nullptr,
    nullptr,
    "Repair",
    "Pods",
    "Shop",
    "Target"
};

constexpr inline const char specialHUDSym[] = {
    0,
    0,
    '+',
    'o',
    's',
    '!'
};

extern const cAction_t specialAction[];

class Ship {
    static inline constexpr s32 maxDistance = screenWidth * 3;
    static inline constexpr s32 minDistance = screenWidth * 2;

    Point2D speed;
    Point2D target;
    const u8* mesh;
    f32 rotation;
    u32 ttnt = 0;
    const u32 uid;
    u32 lastShotTime = getTime();
    u32 fireRate = 0;
    f32 fireSpeed = f32(10.0f);
    f32 fireError = f32(0.3f);

    // sync::Entity entity;

public:
    static inline u8 factionColor[] = {0, 212};
    static inline Point2D missionTarget;

    enum class Special : uint8_t {
        Normal,
        Boss,
        Medic,
        PodGuy,
        Shop,
        Mission
    };

    u32 credits = 0;
    u32 exp = 0;
    f32 maxDmg;
    f32 HP, MP;
    f32 maxHP, maxMP;
    f32 regHP = 0, regMP = 0;
    u16 hitList = 0;
    u16 level = 0;

    static inline constexpr u32 inventorySize = 4;
    u8 inventory[inventorySize] = {0xFF, 0xFF, 0xFF, 0xFF};

    Special special;
    u8 faction;
    u8 maxPodCount;
    u8 podCount = 0;
    u16 shieldTTL = 0;
    u16 maxShieldTTL = 0;

    static inline char notice[32];
    static inline u32 noticeWidth = 0;
    static inline u8 showNotice = 0;

    static inline u8 showExpUp = 0;
    static inline u32 expInc = 0;
    static inline u8 playerLifeCount = 3;

    SERIALIZE("data/Armadillo.sav") {
        PROPERTY(exp);
        PROPERTY(inventory);
        PROPERTY(credits);
    }

    enum class State {
        AIHunt,
        AIFlee,
        Player
    } state;
    Point2D position;
    Ship* enemy = nullptr;

    static inline Ship* player = nullptr;
    static inline u32 nextUID = 0;

    void onHitEnemy(u32 id){
        hitList |= 1 << id;
    }

    bool pollShipsInView(bool sameFaction);

    void onGotHit(u32 dmg){
        if (!shieldTTL)
            HP -= dmg;
        if (isPlayer()) {
            shake += f32(1.0f);
            Audio::play<3>(hit, 128);
            if (HP > 0 && pollShipsInView(true)) {
                effectPriority = 3000;
                streamedEffect = "data/war_look_out.raw";
            }
        } else {
            Audio::play<3>(hit, 64);
            if (HP > 0 && faction == player->faction && (position - player->position).distanceCheck(screenWidth/2)) {
                u32 pick = frame & 1;
                effectPriority = 3000;
                if (pick == 0)
                    streamedEffect = "data/war_cover_me.raw";
                else if (pick == 1)
                    streamedEffect = "data/war_watch_my_back.raw";
            }
        }
        if (HP <= 0) {
            HP = 0;
            broadcast(&Ship::onShipDied, uid);
        } else if (state == State::AIFlee && ttnt > 30) {
            ttnt = 1;
        }
    }

    void onShipDied(u32 id);

    void calcStats(u32 exp, bool reset){
        this->exp = exp;
        u32 prevLevel = this->level;
        this->level = 1 + (exp ? s32(sqrt(f32(exp>>4))) : 0);
        if (level != prevLevel)
            reset = true;

        maxDmg = 5 + level * 5;

        maxMP = 10 + level * 3;
        regMP = f32(0.5f);// + f32(level) / 16;

        maxHP = 95 + level * 15;
        regHP = (level / 8) * 3;
        fireRate = 200;
        fireSpeed = 10;
        fireError = f32(0.3f);
        maxShieldTTL = 5 * 30;
        maxPodCount = 3;

        for (u32 i = 0; i < inventorySize; ++i) {
            if (inventory[i] >= itemCount)
                continue;
            auto& item = ::items[inventory[i]];
            maxHP += item.addHP;
            maxHP *= item.mulHP;
            regHP += item.regHP;

            maxDmg += item.addDmg;
            maxDmg *= item.mulDmg;

            fireError *= item.mulFireError;
            maxShieldTTL += item.addShield;
            maxShieldTTL *= item.mulShield;

            maxPodCount += item.addPod;
            fireRate += item.addFireRate;
        }

        if (!reset)
            return;

        HP = maxHP;
        MP = maxMP;
    }

    void addExp(u32 amount) {
        if (!amount) amount = 1;
        exp += amount;
        u32 prevLevel = level;
        calcStats(exp, false);
        if (state == State::Player) {
            if (level != prevLevel) {
                streamedEffect = "data/level_up.raw";
                showNotice = 30;
                snprintf(notice, sizeof(notice), "Lv:%d", level);
            } else {
                showExpUp = 30;
                expInc = amount;
            }
            // LOG(amount, " exp:", exp, " lvl:", level, "\n");
            save();
        }
    }

    f32 dmg(bool bomb) {
        // f32 critDmg = 0;
        // f32 r = maxDmg * s24q8ToF32(random(f32ToS24q8(f32(0.8) + critDmg), f32ToS24q8(f32(1.2) + critDmg)));
        // if (bomb) r *= f32(0.5);
        return maxDmg;
    }

    Ship() : speed{0,0}, uid{nextUID++} {
        // entity["position.x"] = position.x;
        // entity["position.y"] = position.y;
        // entity["rotation"]   = rotation;

        if (!player){
            podCount = maxPodCount;
            calcStats(1, true);
            state = State::Player;
            player = this;
            position = Point2D{0, 0};
            mesh = craft_racer;
            setFaction(1);
        } else {
            setFaction(2);
        }
    }

    bool isEquipped(u32 item) {
        for (u32 i = 0; i < inventorySize; ++i) {
            if (inventory[i] == item)
                return true;
        }
        return false;
    }

    bool isPlayer() {
        return this == player;
    }

    bool isBoss() {
        return special == Special::Boss || (special == Special::Mission && faction != player->faction);
    }

    s32 radius() {
        return 16 << isBoss();
    }

    void setFaction(u32 f){
        faction = f;
    }

    void stop() {
        speed.x = 0;
        speed.y = 0;
    }

    void spawn() {
        hitList = 0;
        HP = maxHP;
        ttnt = 0;
        state = State::AIFlee;
        podCount = maxPodCount;

        stop();

        mesh = shipMeshes[random(0, sizeof(shipMeshes) / sizeof(shipMeshes[0]))];

        do {
            position = Point2D{
                f32(player->position.x + random(-maxDistance, maxDistance)),
                f32(player->position.y + random(-maxDistance, maxDistance))
            };
        } while((player->position - position).lengthSquared() < minDistance*minDistance);

        u32 odds = 8;
        for (u32 i = 0; i < inventorySize; ++i) {
            if ((random() & 0xFF) > odds)
                inventory[i] = 0xFF;
            else {
                inventory[i] = random(0, itemCount);
                LOG("NPC ", uid, " spawned with ", items[inventory[i]].name, "\n");
            }
        }

        calcStats((int(position.length()) * random(80, 150) >> 9) * f32(isBoss() ? 50 : 1), true);
    }

    bool fire(){
        if (s32((lastShotTime + fireRate) - getTime()) > 0)
            return false;
        if (MP < 0)
            return false;
        MP -= regMP + 5;

        lastShotTime = getTime();
        f32 a = -rotation + s24q8ToF32(random(f32ToS24q8(-fireError), f32ToS24q8(fireError)));

        auto shape = Particles::Shape::Circle;
        if (state == State::Player)
            shape = Particles::Shape::Line;
        else if (isBoss())
            shape = Particles::Shape::BigCircle;

        shots.insert({
                position,
                // static_cast<s16>(speed.x * f32(0.1f) + static_cast<s16>(f32ToS24q8(sin(a) * fireSpeed))),
                // static_cast<s16>(speed.y * f32(0.1f) + static_cast<s16>(f32ToS24q8(cos(a) * fireSpeed))),
                static_cast<s16>(f32ToS24q8(sin(a) * fireSpeed)),
                static_cast<s16>(f32ToS24q8(cos(a) * fireSpeed)),
                u8(1 << faction), 0, u8(uid),
                shape
            }, 1);
        return true;
    }

    void moveToTarget() {
        constexpr auto accel = f32(1.8f);
        if (position.x < target.x) speed.x += accel;
        else if (position.x > target.x) speed.x -= accel;
        if (position.y < target.y) speed.y += accel;
        else if (position.y > target.y) speed.y -= accel;
        speed *= f32(0.97f);
        position += speed * f32(0.05f);
    }

    bool isDead() {
        return HP <= 0;
    }

    bool dead() {
        if (HP > 0)
            return false;
        HP -= 1;
        if (HP < -60) {
            HP = maxHP;
            stop();
            if (state != State::Player) {
                respawnNPC(uid);
            } else {
                shieldTTL = 60;
                if (!playerLifeCount--) {
                    position.set(0, 0);
                    enterWorld(0);
                    playerLifeCount = 3;
                    podCount = maxPodCount;
                }
            }
            return false;
        }
        return true;
    }

    Ship* findTarget();

    void hunt() {
        if (dead())
            return;

        bool speak = false;
        if (ttnt == 0) {
            ttnt = random(200, 500);
            enemy = nullptr;
            speak = faction == player->faction;
        }

        if (!enemy)
            enemy = findTarget();

        if (!enemy || !--ttnt) {
            state = State::AIFlee;
            ttnt = 0;
            return;
        }

        target  = enemy->position;
        moveToTarget();
        rotation += angleDelta(rotation, atan2(position.x - target.x, target.y - position.y)) * f32(0.1f);

        if ( (position - player->position).distanceCheck(screenWidth/2) && fire()) {
            if (speak) {
                effectPriority = 4000;
                streamedEffect = "data/war_target_engaged.raw";
            }

            auto s = (position - player->position).length();
            if (s < 1) s = 1;
            auto volume = f32ToS24q8(f32(screenWidth/4) / s);
            if (!isBoss()) volume >>= 1;
            volume = std::clamp(int(volume), 0, 255);
            if (volume > 100) {
                if (isBoss())
                    Audio::play<5>(shoot, volume, 4);
                else
                    Audio::play<5>(shoot, volume, 2);
            }
        }
    }

    void setTarget(Point2D target) {
        this->target = target;
        this->ttnt = random(60, 300);
    }

    void flee() {
        if (dead())
            return;

        if (ttnt == 0 || (position - target).lengthSquared() < 64) {
            Point2D offset{
                f32(random(-screenWidth, screenWidth)),
                f32(random(-screenWidth, screenWidth))
            };
            if (special == Special::Mission) {
                setTarget(missionTarget + offset);
            } else {
                setTarget(position + offset);
            }
        }

        if (--ttnt <= 0) {
            enemy = findTarget();
            if (enemy && (position - enemy->position).distanceCheck(screenWidth)) {
                ttnt = 0;
                state = State::AIHunt;
            }
        }
        if (!(position - player->position).distanceCheck(maxDistance))
            respawnNPC(uid);

        if (special > Special::Boss)
            updateSpecial();

        moveToTarget();
        rotation += angleDelta(rotation, (PI + atan2(speed.x, -speed.y))) * f32(0.1f);
    }

    void updateSpecial() {
        if (special != Special::Mission) {
            speed *= f32(0.8f);
        }
        if ((position - player->position).distanceCheck(64)) {
            specialInRange();
        }
    }

    void specialInRange() {
        cActionArg = reinterpret_cast<uptr>(this);
        featureCaption = specialCaption[(int)special] ?: "";
        cAction = specialAction[(int)special];
    }

    void repair(bool quiet = false) {
        HP = maxHP;
        if (state == State::Player) {
            playerLifeCount = std::min<u8>(3, playerLifeCount);
            if (!quiet) {
                effectPriority = 1000;
                streamedEffect = "data/ready.raw";
            }
        }
    }

    void podRecharge(bool quiet = false) {
        podCount = maxPodCount;
        if (state == State::Player && !quiet) {
            effectPriority = 1000;
            streamedEffect = "data/power_up.raw";
        }
    }

    static constexpr inline f32 agility = f32(0.2f);
    static constexpr inline f32 acceleration = f32(0.05f);
    static constexpr inline f32 boostAcceleration = f32(0.1f);
    static constexpr inline f32 regularSpeed = f32(40);
    static constexpr inline f32 boostSpeed = f32(70);

    void local() {
        static bool prevRight = false;
        static bool prevDown = false;
        static bool prevLeft = false;
        static bool prevUp = false;
        static bool prevB = false;
        static u32 timeRight = 0;
        static u32 timeDown = 0;
        static u32 timeLeft = 0;
        static u32 timeUp = 0;
        static bool boost = false;

        u32 deltaRight = 500;
        u32 deltaDown = 500;
        u32 deltaLeft = 500;
        u32 deltaUp = 500;

        if (dead())
            return;

        if (isPressed(Button::A)) {
            if (fire())
                Audio::play<2>(shoot, 128).slowdown = 3;
        }

        bool right = isPressed(Button::Right);
        bool down = isPressed(Button::Down);
        bool left = isPressed(Button::Left);
        bool up = isPressed(Button::Up);
        bool B = isPressed(Button::B);

        if (!boost) {
            if (!prevRight && right) { deltaRight = getTime() - timeRight; timeRight = getTime(); }
            if (!prevDown && down) { deltaDown = getTime() - timeDown; timeDown = getTime(); }
            if (!prevLeft && left) { deltaLeft = getTime() - timeLeft; timeLeft = getTime(); }
            if (!prevUp && up) { deltaUp = getTime() - timeUp; timeUp = getTime(); }
            boost = std::min({deltaRight, deltaDown, deltaLeft, deltaUp}) < 500;
            boost = boost || isPressed(Button::D);
        } else if(right || down || left || up) {}
        else boost = false;

        if ((B && !prevB) && podCount && !shieldTTL) {
            podCount--;
            shield();
        }

        s32 s = boost ? boostSpeed : regularSpeed;
        f32 a = boost ? boostAcceleration : acceleration;

        prevRight = right;
        prevDown = down;
        prevLeft = left;
        prevUp = up;
        prevB = B;

        f32 dx = right == 0 && left == 0 ? blit::joystick.x : right - left;
        f32 dy = up == 0 && down == 0 ? blit::joystick.y : down - up;

        target.x -= (target.x - s * dx) * agility;
        speed.x -= (speed.x - target.x) * a;

        target.y -= (target.y - s * dy) * agility;
        speed.y -= (speed.y - target.y) * a;

        if (dx || dy) {
            Audio::play<1>(engine,
                           std::clamp((int) round(40 * (abs(speed.x) + abs(speed.y))), 0, 255),
                           boost ? 1 : 2);
            if (boost && MP > 0)
                MP -= regMP + f32(0.3f);
        }

        // rotation = PI + atan2(target.x, -target.y);
        rotation += angleDelta(rotation - PI, atan2(target.x, -target.y)) * f32(0.2f);
        position += speed * f32(0.06);

        if (podCount && pollShipsInView(false)) {
            featureCaption = "EMP Bomb";
            cActionArg = reinterpret_cast<uptr>(this);
            cAction = +[](uptr ptr){
                reinterpret_cast<Ship*>(ptr)->bomb();
            };
        }
    }

    void shield() {
        shieldTTL = maxShieldTTL;
    }

    void bomb() {
        podCount--;
        shots.insert({
                {position},
                s16(0), s16(0),
                u8(1 << faction), u8(0), u8(uid),
                Particles::Shape::Bomb
            }, radius());
    }

    void update(u32){
        s32 x = f32ToS24q8(position.x);
        s32 y = f32ToS24q8(position.y);

        MP += regMP;
        if (MP > maxMP)
            MP = maxMP;

        switch (state) {
        case State::AIHunt: hunt(); break;
        case State::AIFlee: flee(); break;
        case State::Player: local(); break;
        }

        if (HP > 0) {
            x -= f32ToS24q8(position.x);
            y -= f32ToS24q8(position.y);
            x = x * random(1, 5) >> 3;
            y = y * random(1, 5) >> 3;
        } else {
            s32 r = (1 - HP) * 10;
            x = random(-r, r);
            y = random(-r, r);
        }

        gameRenderer->get<Particles>().insert({
                {position},
                s16(x), s16(y),
                u8(0), u8(0), u8(0),
                isBoss() ? Particles::Shape::BigSmoke : Particles::Shape::Smoke
            }, radius());
    }

    void postUpdate(u32);

    void checkHit(Ship& other) {
        if (other.HP <= 0)
            return;

        auto delta = position - other.position;
        auto r = (radius() + other.radius()) >> 1;
        auto len = delta.lengthSquared();
        if (len > r*r)
            return;

        if (faction != other.faction) {
            f32 impact = (speed + other.speed).length() / 4;
            if (faction != player->faction) {
                other.onHitEnemy(uid);
                onGotHit(impact);
            }
            if (other.faction != player->faction) {
                onHitEnemy(other.uid);
                other.onGotHit(impact);
            }
        }

        len = delta.length();
        f32 overlap = r - len;
        if (len) {
            delta.x /= len;
            delta.y /= len;
        }
        speed += delta * 20;
        other.speed -= delta * 20;
        delta *= overlap;
        position += delta;
        other.position -= delta;

    }

    bool isVisible() {
        s32 r = radius();
        return
            (HP > 0) &&
            (position.x - Graphics::camera.x) > -r &&
            (position.y - Graphics::camera.y) > -r &&
            (position.x - Graphics::camera.x) < (screenWidth + r) &&
            (position.y - Graphics::camera.y) < (screenHeight + r);
    }

    void draw(bool odd){
        if (shieldTTL) {
            shieldTTL--;
            if (!odd) {
                shots.draw({
                        {position},
                        s16(0), s16(0),
                        u8(0), u8(0), u8(uid),
                        Particles::Shape::Shield
                    }, radius());
            }
        }

        if (auto bitmap = drawMesh(mesh, 1.5, rotation, factionColor[faction - 1])) {
            s32 r = radius();
            gameRenderer->bind<ShipsLayer>();
            if (isBoss()) {
                Graphics::draw<true, true>(*bitmap, position - r);
            } else {
                Graphics::draw<true, false>(*bitmap, position - r);
            }
        }

        gameRenderer->get<HUDLayer>().bind();
        if (state == State::Player) {
            Graphics::primaryColor = colorFromRGB(0x33EEAA);
            Graphics::setCursor(0, HUDSize);
            Graphics::print("\n >", missionText);
            Graphics::print(
                "\n X: ", (int) round(position.x),
                "\n Y: ", (int) round(position.y));
            Graphics::draw(BitmapFrame<8>(lifeCountUI, 11, 7, playerLifeCount), position + Point2D{f32(16), f32(20)});

            for (u32 i = 0; i < maxPodCount; ++i) {
                Graphics::draw(BitmapFrame<8>(podCountUI, 3, 7, i < podCount), position + Point2D{f32(16 + i * 4), f32(30)});
            }
        }

        if (state == State::Player || /*(HP <= (maxHP / 2) &&*/ odd) {
            if (state == State::Player && showNotice) {
                showNotice--;
                if (odd) {
                    bool big = noticeWidth < 10;
                    f32 x = -f32(noticeWidth * (big ? 5 : 3)) / 2;
                    Graphics::setCursor(position + Point2D{x, f32(-7 - radius() - 30 + showNotice * 2)});
                    Graphics::primaryColor = colorFromRGB(0x77EEFF);
                    Graphics::doubleFontSize = big;
                    Graphics::print(notice);
                }
            } else if (state == State::Player && showExpUp) {
                showExpUp--;
                Graphics::setCursor(position + Point2D{f32(-5), f32(-7 - radius())});
                Graphics::primaryColor = colorFromRGB(0x33FF33);
                Graphics::doubleFontSize = false;
                Graphics::print("Exp +", expInc);
            } else if (state != State::Player && HP == maxHP && faction != player->faction) {
                Graphics::setCursor(position + Point2D{f32(-5), f32(-7 - radius())});
                Graphics::primaryColor = colorFromRGB(0xFFFFFF);
                Graphics::doubleFontSize = false;
                Graphics::print("Lv:", level);
            } else if (state != State::Player && faction == player->faction) {
                if (const char* txt = specialCaption[int(special)]; txt) {
                    Graphics::setCursor(position + Point2D{f32(-5), f32(-7 - radius())});
                    Graphics::primaryColor = cActionArg == reinterpret_cast<uptr>(this) ?
                        colorFromRGB(0xFFFFFF) :
                        colorFromRGB(0x0000FF);
                    Graphics::doubleFontSize = false;
                    Graphics::print(txt);
                }
            } else {
                // Graphics::setCursor(position + Point2D{f32(-5), f32(-6 - radius())});
                // Graphics::doubleFontSize = isBoss();
                // Graphics::print(s32(HP));
            }

            Graphics::doubleFontSize = false;
        }

        if (HP != maxHP) {
            Graphics::primaryColor = HP > (maxHP / 4) ? colorFromRGB(0x00FF00) : colorFromRGB(0xFF3300);
            auto lineStart = position - radius();
            auto lineMid = lineStart + Point2D{f32(HP * 2 * radius()) / maxHP, 0};
            auto lineEnd = lineStart + Point2D{f32(2 * radius()), 0};
            Graphics::line(lineStart, lineMid);
            Graphics::line(lineMid, lineEnd, 0x770000);
        }
    }

    static void broadcast(void (Ship::*method)(u32), u32);

} ships[shipCount];

inline void Ship::onShipDied(u32 id){
    auto& dead = ships[id];
    if (&dead == this) {
        hitList = 0;
        if (isPlayer()) {
            shake += f32(10.0f);
            Audio::play<3>(explosion);
        } else {
            Audio::play<4>(explosion);
        }
        if (special == Special::Mission) {
            LOG("Mission NPC Died\n");
            missionNPCDied();
        }
    } else if (hitList & (1 << id)) {
        hitList ^= 1 << id;

        if (isPlayer() && dead.faction != player->faction)
            addKill();

        if (!dead.isPlayer()) {
            for (u32 i = 0; i < inventorySize; ++i) {
                u32 item = dead.inventory[i];
                if (item > itemCount)
                    continue;

                if (isPlayer())
                    addItem(item);

                for (u32 j = 0; j < inventorySize; ++j) {
                    if (inventory[j] < itemCount)
                        continue;
                    if (items[item].name == nullptr) {
                        LOG(item, " is null\n");
                        break;
                    }
                    inventory[j] = item;
                    dead.inventory[i] = itemCount;

                    if (isPlayer()) {
                        showNotice = 30;
                        noticeWidth = snprintf(notice, sizeof(notice), "Got %s", items[item].name);
                        LOG("Player got ", items[item].name, "\n");
                    }

                    i = inventorySize;
                    j = inventorySize;
                }
            }
        }

        credits += random(dead.level * 8, dead.level * 12);

        addExp((dead.level + 1) * 8);
        floaters.addFloater(ships[id].position, &position, Floaters::Credits);
        if (enemy == &ships[id])
            enemy = nullptr;
        if (!isPlayer() && faction == player->faction) {
            effectPriority = 5000;
            streamedEffect = "data/war_target_destroyed.raw";
        }
    }
}

inline void Ship::postUpdate(u32 frame) {
    if (HP > 0)
        addToHUD(position - player->position,
                 isPlayer() ? 0xFFFF : (faction == 2 ? colorFromRGB(0xFF0000) : colorFromRGB(0x00FF00)),
                 special);

    if (!isVisible())
        return;

    if (auto particle = shots.find(position, ~(1 << faction), radius())) {
        auto& shooter = ships[particle->data];
        if (particle->shape != Particles::Shape::Bomb) {
            auto normal = (*particle - position).normalize() * 4;
            particle->shape = Particles::Shape::Dot;
            particle->mask = 0;
            particle->vx = f32ToS24q8(normal.x) + random(-200, 200);
            particle->vy = f32ToS24q8(normal.y) + random(-200, 200);
            particle->ttl = 10;
        }
        shooter.onHitEnemy(uid);
        onGotHit(shooter.dmg(particle->shape == Particles::Shape::Bomb));
        if (HP <= 0) {
            return;
        }
    }

    for (u32 j = uid + 1; j < shipCount; ++j)
        checkHit(ships[j]);

    draw((frame + uid) & 1);
}

inline void Ship::broadcast(void (Ship::*method)(u32), u32 data) {
    for (u32 i = 0; i < shipCount; ++i) {
        (ships[i].*method)(data);
    }
}


inline Ship* Ship::findTarget() {
    Ship* other = nullptr;
    u32 otherDistance = 0;
    for (u32 i = 0; i < shipCount; ++i) {
        auto& candidate = ships[i];
        if (candidate.faction == faction || candidate.HP <= 0)
            continue;
        u32 distance = (candidate.position - position).lengthSquared();
        if (!other || otherDistance > distance) {
            other = &candidate;
            otherDistance = distance;
        }
    }
    return other;
}

bool Ship::pollShipsInView(bool sameFaction) {
    for (u32 i = 0; i < shipCount; ++i) {
        auto& other = ships[i];
        if (&other == this)
            continue;
        if (sameFaction && other.faction != faction)
            continue;
        if (!sameFaction && other.faction == faction)
            continue;
        if ((other.position - position).distanceCheck(screenHeight/2))
            return true;
    }
    return false;
}

constexpr inline const cAction_t specialAction[] = {
    nullptr, // normal
    nullptr, // boss
    +[](uptr ptr){ Ship::player->repair(); }, //medic
    +[](uptr ptr){ Ship::player->podRecharge(); }, // pod guy
    +[](uptr ptr){ gameState = GameState::EnterShop; }, // shop
    +[](uptr ptr){ missionNPCInteract(); } // Mission
};
