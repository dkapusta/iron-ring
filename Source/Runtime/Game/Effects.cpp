#include "Game/Effects.h"

#include <algorithm>

namespace iron {
namespace {

const char* effectName(sim::Effect effect) {
    switch (effect) {
    case sim::Effect::LightHit:  return "light_hit";
    case sim::Effect::HeavyHit:  return "heavy_hit";
    case sim::Effect::Guard:     return "guard";
    case sim::Effect::Breakaway: return "breakaway";
    case sim::Effect::ThrowTech: return "tech";
    case sim::Effect::Slam:      return "slam";
    }
    return "light_hit";
}

} // namespace

Effects::Effects(AssetLibrary& assets, std::string_view atlasPath) : sheet_(assets, atlasPath) {}

void Effects::spawn(const sim::StepEvents::Event& event, int clock) {
    Active a;
    a.name = effectName(event.effect);
    while (sheet_.frame(a.name + "_" + std::to_string(a.frames))) ++a.frames;
    a.x = static_cast<float>(event.x) / sim::kFixed;
    a.y = static_cast<float>(sim::kFloorY) - static_cast<float>(event.y) / sim::kFixed;
    a.start = clock;
    active_.push_back(std::move(a));
}

void Effects::update(int clock) {
    std::erase_if(active_, [&](const Active& a) { return clock - a.start >= a.frames * kTicksPerFrame; });
}

void Effects::draw(const RenderContext& context, int clock) const {
    // On HDR output effect highlights go above SDR white; on SDR this is a no-op.
    SDL_SetRenderColorScale(context.renderer, context.highlightBoost);
    for (const auto& a : active_) {
        const int frame = std::clamp((clock - a.start) / kTicksPerFrame, 0, a.frames - 1);
        sheet_.draw(context.renderer, a.name + "_" + std::to_string(frame), a.x, a.y, false);
    }
    SDL_SetRenderColorScale(context.renderer, 1.0f);
}

} // namespace iron
