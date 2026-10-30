#include "Game/StageView.h"

#include <algorithm>
#include <cmath>

namespace iron {
namespace {

constexpr float kCalmPerTick = 0.992f; // excitement halves in about 1.5 s
constexpr float kCheerAbove = 0.45f;

float eventExcitement(sim::Effect e) {
    switch (e) {
    case sim::Effect::LightHit: return 0.06f;
    case sim::Effect::HeavyHit: return 0.22f;
    case sim::Effect::Guard: return 0.02f;
    case sim::Effect::Breakaway: return 0.25f;
    case sim::Effect::ThrowTech: return 0.15f;
    case sim::Effect::Slam: return 0.35f;
    }
    return 0;
}

} // namespace

StageView::StageView(const StageInfo& stage) : stage_(stage), phase_(stage.layers.size(), 0.0f) {}

void StageView::excite(float amount) { excitement_ = std::min(1.0f, excitement_ + amount); }

void StageView::onEvents(const sim::StepEvents& events) {
    for (int i = 0; i < events.count; ++i) excite(eventExcitement(events.items[static_cast<std::size_t>(i)].effect));
}

void StageView::update() {
    ++ticks_;
    excitement_ *= kCalmPerTick;
    const auto& layers = stage_.definition.layers;
    for (std::size_t i = 0; i < layers.size(); ++i) {
        const auto& l = layers[i];
        float speed = 1.0f / static_cast<float>(l.frameTicks);
        if (l.crowd) speed *= excitement_ > kCheerAbove ? 2.0f : 0.6f + excitement_; // restless, then jumping
        phase_[i] += speed;
    }
}

void StageView::drawLayer(SDL_Renderer* r, std::size_t index, float cameraX) const {
    const auto& l = stage_.definition.layers[index];
    const Texture& tex = *stage_.layers[index];
    const float frameH = tex.height / static_cast<float>(l.frames);
    int frame = 0;
    if (l.crowd) {
        const int half = l.frames / 2;
        frame = static_cast<int>(phase_[index]) % half + (excitement_ > kCheerAbove ? half : 0);
    } else if (l.frames > 1) {
        frame = static_cast<int>(phase_[index]) % l.frames;
    }
    const SDL_FRect src{0, frameH * static_cast<float>(frame), tex.width, frameH};
    float x = std::floor(static_cast<float>(l.x) - cameraX * l.parallax);
    if (l.drift != 0.0f) {
        // Drifting layers repeat: draw enough copies to cover the screen.
        const float shift = std::fmod(static_cast<float>(ticks_) * l.drift / 60.0f, tex.width);
        x = std::floor(x + shift);
        x = std::fmod(x, tex.width);
        if (x > 0) x -= tex.width;
        for (float at = x; at < 640.0f; at += tex.width) {
            const SDL_FRect dst{at, static_cast<float>(l.y), tex.width, frameH};
            SDL_RenderTexture(r, tex.handle.get(), &src, &dst);
        }
        return;
    }
    const SDL_FRect dst{x, static_cast<float>(l.y), tex.width, frameH};
    SDL_RenderTexture(r, tex.handle.get(), &src, &dst);
}

void StageView::drawBack(SDL_Renderer* r, float cameraX) const {
    if (!stage_.definition.layered()) {
        const SDL_FRect full{0, 0, 640, 360};
        SDL_RenderTexture(r, stage_.image->handle.get(), nullptr, &full);
        return;
    }
    for (std::size_t i = 0; i < stage_.layers.size(); ++i)
        if (!stage_.definition.layers[i].front) drawLayer(r, i, cameraX);
}

void StageView::drawFront(SDL_Renderer* r, float cameraX) const {
    for (std::size_t i = 0; i < stage_.layers.size(); ++i)
        if (stage_.definition.layers[i].front) drawLayer(r, i, cameraX);
}

} // namespace iron
