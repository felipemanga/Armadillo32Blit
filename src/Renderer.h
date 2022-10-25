#pragma once

#include <variant>
#include <MonoText.hpp>

inline Particles shots;
using PlanetsLayer = Graphics::layer::FastDrawList<10, 5>;
using ShipsLayer = Graphics::layer::FastDrawList<20, 5>;
using HUDLayer = Graphics::layer::DrawList<80, fontTiny>;

using GameRenderer = Graphics::Renderer<
    Background,
    PlanetsLayer,
    Particles,
    ShipsLayer,
    HUDLayer
    >;

using ShopRenderer = Graphics::Renderer<
    Graphics::layer::SolidColor<colorFromRGB(0x777788)>,
    Graphics::layer::MonoText<fontTiny>
    >;

inline GameRenderer* gameRenderer;
inline ShopRenderer* shopRenderer;

std::variant<GameRenderer, ShopRenderer> renderer;
