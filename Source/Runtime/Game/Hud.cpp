#include "Game/Hud.h"

#include "Render/Ui.h"

#include <algorithm>
#include <string>

namespace iron {
using namespace ui;
namespace {

SDL_FRect rectOf(const nlohmann::json& a) {
    return {a[0].get<float>(), a[1].get<float>(), a[2].get<float>(), a[3].get<float>()};
}

} // namespace

Hud::Hud(nlohmann::json layout, const BitmapFont& small, const BitmapFont& large, std::string meterName)
    : layout_(std::move(layout)), small_(small), large_(large), meterName_(std::move(meterName)) {}

void Hud::onHit(int attacker, int combo, bool counter) {
    if (attacker < 0) return;
    auto& c = callouts_[static_cast<std::size_t>(attacker)];
    if (combo >= 2) {
        c.combo = combo;
        c.comboTicks = 70;
    }
    if (counter) {
        c.text = "COUNTER";
        c.textTicks = 50;
    }
}

void Hud::onBreakaway(int side) {
    if (side < 0) return;
    auto& c = callouts_[static_cast<std::size_t>(side)];
    c.text = "BREAKAWAY";
    c.textTicks = 50;
}

void Hud::clearCallouts() { callouts_ = {}; }

void Hud::drawCallouts(SDL_Renderer* r, int side) const {
    const auto& c = callouts_[static_cast<std::size_t>(side)];
    const bool right = side == 1;
    const float x = right ? 628.0f : 12.0f;
    const Align align = right ? Align::Right : Align::Left;
    if (c.comboTicks > 0) {
        large_.draw(r, std::to_string(c.combo) + " HITS", x, 48, kWhite, align);
    }
    if (c.textTicks > 0 && (c.textTicks > 30 || (blink_ / 4) % 2 == 0))
        small_.draw(r, c.text, x, 76, c.text == "COUNTER" ? kCoralLight : kTealLight, align, true);
}

void Hud::update(const sim::Match& match) {
    ++blink_;
    for (auto& c : callouts_) {
        if (c.comboTicks > 0) --c.comboTicks;
        if (c.textTicks > 0) --c.textTicks;
    }
    for (std::size_t i = 0; i < 2; ++i) {
        const auto hp = static_cast<float>(match.round.fighters[i].hp);
        // The damage bar waits a moment, then drains to the real value.
        if (ghostHealth_[i] < hp || match.phase == sim::Phase::Intro) ghostHealth_[i] = hp;
        else if (match.round.hitstop == 0) ghostHealth_[i] = std::max(hp, ghostHealth_[i] - 0.6f);
    }
}

void Hud::drawPlayer(SDL_Renderer* r, const sim::Match& match, const FighterView& view, int side) const {
    const auto& l = layout_.at(side == 0 ? "p1" : "p2");
    const auto& f = match.round.fighters[static_cast<std::size_t>(side)];
    const bool right = side == 1;

    // Portrait (player 2 mirrored to face the centre).
    const SDL_FRect p = rectOf(l.at("portrait"));
    frame(r, p);
    SDL_RenderTextureRotated(r, view.portrait().handle.get(), nullptr, &p, 0, nullptr,
                             right ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);

    // Health: anchored at the outer edge, damage eats in from the centre.
    const SDL_FRect bar = rectOf(l.at("health"));
    frame(r, bar);
    const float hp = static_cast<float>(f.hp) / 100.0f;
    const float ghost = ghostHealth_[static_cast<std::size_t>(side)] / 100.0f;
    const auto span = [&](float fraction, SDL_Color c, float inset = 0) {
        const float w = bar.w * fraction;
        fill(r, right ? bar.x + bar.w - w : bar.x, bar.y + inset, w, bar.h - inset * 2, c);
    };
    span(ghost, kCoral);
    const bool danger = f.hp <= 25;
    span(hp, danger && (blink_ / 8) % 2 ? kCoralLight : kBrassLight);
    span(hp, danger ? kCoralLight : kChalk, 2);
    span(hp, kBrassLight, bar.h - 3);

    const auto name = l.at("name");
    small_.draw(r, view.definition().displayName, name[0].get<float>(), name[1].get<float>(), kChalk,
                right ? Align::Right : Align::Left, true);

    // Round win markers.
    const auto wins = l.at("wins");
    for (int i = 0; i < sim::kRoundsToWin; ++i) {
        const float x = wins[0].get<float>() + static_cast<float>(right ? -(i * 9) - 6 : i * 9);
        const float y = wins[1].get<float>();
        const bool lit = match.wins[static_cast<std::size_t>(side)] > i;
        fill(r, x, y, 6, 6, kInk);
        fill(r, x + 1, y + 1, 4, 4, lit ? kBrassLight : kSteel);
        if (lit) fill(r, x + 1, y + 1, 2, 1, kChalk);
    }

    // Heat meter: two segments, one Breakaway each.
    const SDL_FRect m = rectOf(l.at("meter"));
    frame(r, m);
    const float heat = static_cast<float>(f.meter) / sim::kMeterMax;
    const float w = m.w * heat;
    const bool ready = f.meter >= sim::kBreakCost;
    fill(r, right ? m.x + m.w - w : m.x, m.y, w, m.h, ready ? kTeal : kTealDark);
    if (ready && (blink_ / 10) % 2) fill(r, right ? m.x + m.w - w : m.x, m.y, w, 2, kTealLight);
    fill(r, m.x + m.w / 2, m.y, 1, m.h, kInk);
    const auto label = l.at("meterLabel");
    small_.draw(r, meterName_, label[0].get<float>(), label[1].get<float>(), kTealLight,
                right ? Align::Right : Align::Left, true);
}

void Hud::drawBanner(SDL_Renderer* r, const sim::Match& match,
                     const std::array<const FighterView*, 2>& fighters) const {
    const auto at = layout_.at("banner");
    const float x = at[0].get<float>(), y = at[1].get<float>();
    std::string text;
    switch (match.phase) {
    case sim::Phase::Intro: {
        if (match.phaseTicks < sim::kFightCallTick)
            text = sim::isFinalRound(match) ? "FINAL ROUND" : "ROUND " + std::to_string(match.roundNumber);
        else
            text = "FIGHT!";
        break;
    }
    case sim::Phase::RoundOver: {
        const auto& f = match.round.fighters;
        const bool ko = f[0].hp == 0 || f[1].hp == 0;
        if (match.phaseTicks < 70) text = ko ? (f[0].hp == 0 && f[1].hp == 0 ? "DOUBLE K.O." : "K.O.") : "TIME";
        else if (match.lastRound == sim::Outcome::Draw) text = "DRAW";
        else text = fighters[match.lastRound == sim::Outcome::Player1 ? 0 : 1]->definition().displayName + " WINS";
        break;
    }
    case sim::Phase::MatchOver:
        if (match.winner == sim::Outcome::Draw) text = "DRAW GAME";
        else text = fighters[match.winner == sim::Outcome::Player1 ? 0 : 1]->definition().displayName;
        large_.draw(r, text, x, y, {255, 255, 255, 255}, Align::Centre);
        if ((blink_ / 20) % 2 == 0 && match.winner != sim::Outcome::Draw)
            small_.draw(r, "TAKES THE RING", x, y + 30, kChalk, Align::Centre, true);
        if (match.winner != sim::Outcome::Draw) {
            // A win quote, the same one for the same match: picked by the loser and the rounds it took.
            const int w = match.winner == sim::Outcome::Player1 ? 0 : 1;
            const auto& quotes = fighters[static_cast<std::size_t>(w)]->definition().story.winQuotes;
            if (!quotes.empty()) {
                std::size_t pick = static_cast<std::size_t>(match.roundNumber);
                for (const char ch : fighters[static_cast<std::size_t>(1 - w)]->definition().id)
                    pick += static_cast<unsigned char>(ch);
                small_.draw(r, "\"" + ui::upper(quotes[pick % quotes.size()]) + "\"", x, y + 48, kChalk, Align::Centre,
                            true);
            }
        }
        return;
    case sim::Phase::Fight:
        return;
    }
    large_.draw(r, text, x, y, {255, 255, 255, 255}, Align::Centre);
}

void Hud::draw(SDL_Renderer* r, const sim::Match& match, const std::array<const FighterView*, 2>& fighters,
               const std::string& status) const {
    for (int side = 0; side < 2; ++side) {
        drawPlayer(r, match, *fighters[static_cast<std::size_t>(side)], side);
        drawCallouts(r, side);
    }

    const SDL_FRect t = rectOf(layout_.at("timer"));
    frame(r, t);
    fill(r, t.x, t.y, t.w, t.h, kNight);
    // Arcade clocks show two digits.
    const int seconds = std::clamp((match.round.remaining + sim::kTickRate - 1) / sim::kTickRate, 0, 99);
    const std::string clock = (seconds < 10 ? "0" : "") + std::to_string(seconds);
    large_.draw(r, clock, t.x + t.w / 2, t.y + 4, {255, 255, 255, 255}, Align::Centre);

    const auto round = layout_.at("roundLabel");
    small_.draw(r, "ROUND " + std::to_string(match.roundNumber), round[0].get<float>(), round[1].get<float>(),
                kBrassLight, Align::Centre, true);
    const auto bottom = layout_.at("status");
    small_.draw(r, status, bottom[0].get<float>(), bottom[1].get<float>(), {138, 147, 168, 255}, Align::Centre,
                true);
    drawBanner(r, match, fighters);
}

} // namespace iron
