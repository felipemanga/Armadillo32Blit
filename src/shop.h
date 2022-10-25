#pragma once

#include "item.h"
class Shop {
public:
    enum class Mode {
        topLevel,
        buy,
        owned
    } mode;

    u32 selection;
    bool dirty;
    u32 redrawTime;

    void init() {
        mode = Mode::topLevel;
        selection = 0;
        dirty = true;
        redrawTime = getTime();
    }

    void tick() {
        u32 now = getTime();
        if (dirty || now - redrawTime < 200)
            return;
        int move = isPressed(Button::Down) - isPressed(Button::Up);
        selection += move;
        dirty |= move != 0;
        dirty |= isPressed(Button::A) || isPressed(Button::B) || isPressed(Button::C);
    }

    void redraw() {
        if (isPressed(Button::C)) {
            if (mode == Mode::topLevel) {
                targetBacklight = 0;
                return;
            } else {
                setMode(Mode::topLevel);
            }
        }
        redrawTime = getTime();
        dirty = false;
        Graphics::clearText();
        Graphics::setCursor(20, 8);

        switch (mode) {
        case Mode::topLevel: redrawTopLevel(); break;
        case Mode::buy: redrawBuy(); break;
        case Mode::owned: redrawOwned(); break;
        }
    }

    void setMode(Mode mode) {
        if (this->mode == mode) {
            return;
        }
        this->mode = mode;
        dirty = true;
        selection = 0;
    }

    void redrawTopLevel() {
        using namespace Graphics;
        print("Menu");
        setCursor(5, (0 + 3) * 8);
        print(selection == 0 ? '>' : ' ', "Buy items");
        setCursor(5, (1 + 3) * 8);
        print(selection == 1 ? '>' : ' ', "View owned items");
        if (selection >= 2) {
            if (s32(selection) < 0) selection = 1;
            else selection = 0;
            dirty = true;
        }
        if (isPressed(Button::A)) {
            switch (selection) {
            case 0: setMode(Mode::buy); break;
            case 1: setMode(Mode::owned); break;
            }
        }
    }

    void redrawBuy() {
        constexpr const u32 maxShopItems = 5;

        using namespace Graphics;
        print("Item Shop");

        u32 line = 0;
        u32 key = 0;
        key += getTime() / 0x5460000;
        key += world * 1664525;
        key += 1013904223;

        u32 itemIds[maxShopItems];
        for (u32 i = 0; i < maxShopItems; ++i) {
            key ^= key << 17;
            key ^= key >> 13;
            key ^= key << 5;
            line = key % itemCount;
            for (u32 j = 0; j < i; ++j) {
                if (itemIds[j] == line) {
                    line = (line + 1) % itemCount;
                    j = 0;
                }
            }
            itemIds[i] = line;
        }

        line = 0;
        for (u32 i = 0; i < maxShopItems; ++i) {
            u32 item = itemIds[i];
            char pref = selection == line ? '>' : ' ';
            setCursor(5, (line + 3) * 8);
            print(pref, items[item].name);
            setCursor(38 * 5, (line + 3) * 8);
            print(sellPrice(items[item].price), "c\n");
            ++line;
        }
        if (selection >= line) {
            if (s32(selection) < 0) selection = line;
            else selection = 0;
            dirty = true;
        }
    }

    s32 buyPrice(s32 basePrice) {
        auto i = f32(universe.worldKillCount[world]) / f32(255);
        return basePrice - (1 - i) * (basePrice * f32(0.5));
    }

    s32 sellPrice(s32 basePrice) {
        auto i = f32(universe.worldKillCount[world]) / f32(255);
        return basePrice + (1 - i) * (basePrice * f32(0.5));
    }

    void redrawOwned() {
        using namespace Graphics;
        print("Inventory");
        u32 line = 0;
        for (u32 item = 0; item < itemCount; ++item) {
            u32 count = universe.ownedItems[item];
            if (!count) continue;
            bool equipped = Ship::player->isEquipped(item);
            char pref = selection == line ? '>' : ' ';

            setCursor(5, (line + 3) * 8);
            print(pref, items[item].name);
            if (equipped)
                print(" [e]");
            setCursor(30 * 5, (line + 3) * 8);
            print(" x", count);
            setCursor(38 * 5, (line + 3) * 8);
            print(items[item].price, "c\n");
            ++line;
        }
        if (selection >= line) {
            if (s32(selection) < 0) selection = line;
            else selection = 0;
            dirty = true;
        }
    }
} shop;

void updateEnterShop() {
    if (updateEnter(GameState::Shop)) {
        shopRenderer = &renderer.emplace<ShopRenderer>();
        shop.init();
    }
}

void updateShop() {
    using namespace Graphics;
    clear();
    if (targetBacklight == 255) {
        shop.tick();
        if (shop.dirty)
            shop.redraw();
    } else {
        if (backlight == 0) {
            gameState = GameState::Space;
            gameRenderer = &renderer.emplace<GameRenderer>();
            targetBacklight = 255;
        }
    }
}
