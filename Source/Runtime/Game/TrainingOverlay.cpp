#include "Game/TrainingOverlay.h"

#include <cstdio>

namespace iron {
namespace {

struct Colour {
    Uint8 r, g, b;
};
constexpr Colour kHurt{63, 196, 184};
constexpr Colour kHit{239, 106, 69};
constexpr Colour kGrab{242, 176, 78};
constexpr Colour kPush{207, 196, 168};

void outline(SDL_Renderer* r, const sim::WorldBox& b, Colour c, bool filled) {
    const float x0 = static_cast<float>(b.x0) / sim::kFixed;
    const float x1 = static_cast<float>(b.x1) / sim::kFixed;
    const float y0 = static_cast<float>(sim::kFloorY) - static_cast<float>(b.y1) / sim::kFixed;
    const float y1 = static_cast<float>(sim::kFloorY) - static_cast<float>(b.y0) / sim::kFixed;
    const SDL_FRect rect{x0, y0, x1 - x0, y1 - y0};
    if (filled) {
        SDL_SetRenderDrawColor(r, c.r, c.g, c.b, 70);
        SDL_RenderFillRect(r, &rect);
    }
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, 230);
    SDL_RenderRect(r, &rect);
}

bool actionable(const sim::Fighter& f) {
    return f.move < 0 && f.stun == 0 && !f.busy();
}

} // namespace

void TrainingOverlay::observe(const sim::Round& round, bool contactThisTick) {
    ++tick_;
    if (contactThisTick) {
        measuring_ = true;
        freeAt_ = {-1, -1};
    }
    if (!measuring_) return;
    // Advantage only makes sense for hits and blocks that leave both standing.
    for (const auto& f : round.fighters) {
        if (f.knockdown > 0 || f.heldTicks > 0) {
            advantage_ = "KNOCKDOWN";
            measuring_ = false;
            return;
        }
    }
    for (std::size_t i = 0; i < 2; ++i)
        if (freeAt_[i] < 0 && actionable(round.fighters[i])) freeAt_[i] = tick_;
    if (freeAt_[0] >= 0 && freeAt_[1] >= 0) {
        // Positive: player 1 can act first by that many ticks.
        const int adv = freeAt_[1] - freeAt_[0];
        char text[32];
        std::snprintf(text, sizeof text, "%+d", adv);
        advantage_ = adv == 0 ? "0" : text;
        measuring_ = false;
    }
}

void TrainingOverlay::draw(SDL_Renderer* r, const sim::Round& round,
                           const std::array<const FighterView*, 2>& views) const {
    if (!visible_) return;
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    for (std::size_t i = 0; i < 2; ++i) {
        const auto& f = round.fighters[i];
        const auto& spec = round.spec(f);
        if (!f.busy())
            for (const auto& box : round.hurtboxes(f))
                if (!box.empty()) outline(r, sim::place(box, f), kHurt, false);
        const sim::Box push{-spec.pushWidth / 2, 0, spec.pushWidth, 4};
        outline(r, sim::place(push, f), kPush, false);
        if (f.move >= 0) {
            const auto& m = round.currentMove(f);
            const bool active = f.age >= m.startup && f.age < m.startup + m.active;
            const Colour c = m.kind == sim::MoveKind::Throw ? kGrab : kHit;
            if (active) outline(r, sim::place(m.hitbox, f), c, true);
        }

        // Move name and frame data in the corner of each side.
        const auto& def = views[i]->definition();
        std::string line = "-";
        if (f.move >= 0) {
            const auto& m = round.currentMove(f);
            char data[48];
            std::snprintf(data, sizeof data, " %d/%d/%d", m.startup, m.active, m.recovery);
            line = def.moves[static_cast<std::size_t>(f.move)].name + data;
        } else if (f.throwTicks > 0) {
            line = def.moves[static_cast<std::size_t>(f.throwSlot)].name;
        }
        font_.draw(r, line, i == 0 ? 10.0f : 630.0f, 300.0f, {241, 234, 216, 255}, i == 0 ? Align::Left : Align::Right,
                   true);
    }
    font_.draw(r, "ADVANTAGE P1 " + advantage_, 320, 300, {242, 176, 78, 255}, Align::Centre, true);
    font_.draw(r, "F3 BOXES  HURT  HIT  GRAB  PUSH", 320, 60, {138, 147, 168, 255}, Align::Centre, true);
}

} // namespace iron
