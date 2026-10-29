#include "Game/FightScene.h"

#include "Assets/AssetLibrary.h"
#include "Audio/AudioSystem.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

namespace iron {
namespace {

constexpr int kRoundOverDelayTicks = 3 * sim::kTickRate;

struct Rgba {
    Uint8 r, g, b, a;
};
constexpr Rgba kInk{9, 22, 39, 235};
constexpr Rgba kBrass{239, 186, 89, 255};
constexpr Rgba kCream{247, 228, 181, 255};
constexpr Rgba kTeal{46, 190, 185, 255};
constexpr Rgba kCoral{244, 119, 78, 255};
constexpr Rgba kDamage{66, 35, 36, 255};

void fill(SDL_Renderer* r, SDL_FRect rect, Rgba c) {
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderFillRect(r, &rect);
}

// Placeholder text with a 1px ink shadow so it reads over busy stage art.
void text(SDL_Renderer* r, float x, float y, const char* s, Rgba c = kCream) {
    SDL_SetRenderDrawColor(r, kInk.r, kInk.g, kInk.b, 255);
    SDL_RenderDebugText(r, x + 1, y + 1, s);
    SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
    SDL_RenderDebugText(r, x, y, s);
}

SDL_FRect rectFrom(const nlohmann::json& a) {
    return {a[0].get<float>(), a[1].get<float>(), a[2].get<float>(), a[3].get<float>()};
}

const char* soundFor(sim::Effect effect) {
    switch (effect) {
    case sim::Effect::LightHit:     return "light_hit";
    case sim::Effect::HeavyHit:     return "heavy_hit";
    case sim::Effect::Guard:        return "guard";
    case sim::Effect::Breakaway: return "breakaway";
    }
    return "";
}

// Maps simulation state to the kit's eight concept poses:
// 0 idle, 1 step, 2 guard, 3 light punch, 4 heavy punch, 5 kick, 6 hit, 7 victory.
int poseFor(const sim::Fighter& f, int tick, sim::Outcome outcome, int index) {
    const bool won = (outcome == sim::Outcome::Player1 && index == 0) || (outcome == sim::Outcome::Player2 && index == 1);
    if (won) return 7;
    if (f.hp == 0) return 6;
    if (f.stun > 0) return f.blockstun ? 2 : 6;
    if (f.move >= 0) return f.move == 0 ? 3 : (f.move == 1 ? 4 : 5);
    if (f.guarding || f.crouching) return 2;
    if (f.y > 0) return 0;
    return (tick / 12) % 2;
}

} // namespace

FightScene::FightScene(AssetLibrary& assets, AudioSystem& audio)
    : audio_(audio), input_(assets.json("ProjectSettings/InputManager.json")) {
    const auto settings = assets.json("ProjectSettings/GameSettings.json");

    const auto& roster = settings.at("fighters");
    for (std::size_t i = 0; i < fighters_.size(); ++i) {
        const auto& f = roster.at(i);
        fighters_[i].atlas = std::make_unique<SpriteAtlas>(assets, f.at("atlas").get<std::string>());
        fighters_[i].displayName = f.at("displayName").get<std::string>();
        fighters_[i].heightPixels = f.value("heightPixels", 140.0f);
    }
    loadStages(assets, settings.at("stages").get<std::string>(), settings.value("startStage", ""));
    effects_ = std::make_unique<SpriteAtlas>(assets, settings.at("effects").get<std::string>());
    hud_ = assets.json("Resources/UI/hud_layout.json");
    meterName_ = settings.value("meterName", "METER");

    for (const auto& [id, path] : settings.at("sounds").items()) audio_.load(id, assets.bytes(path.get<std::string>()));
    restartRound();
}

void FightScene::loadStages(AssetLibrary& assets, const std::string& stagesPath, const std::string& startStage) {
    const auto catalog = assets.json(stagesPath); // keep alive: range-for over a temporary's member dangles
    for (const auto& s : catalog.at("stages")) {
        if (s.at("id").get<std::string>() == startStage) stage_ = stages_.size();
        stages_.push_back({s.at("name").get<std::string>(), &assets.texture(s.at("image").get<std::string>())});
    }
}

void FightScene::restartRound() {
    match_.reset();
    roundOverTicks_ = 0;
    audio_.play("round_bell");
}

void FightScene::handleEvent(const SDL_Event& event) {
    if (event.type != SDL_EVENT_KEY_DOWN) return;
    input_.onKeyDown(event.key);
    if (event.key.repeat) return;
    switch (event.key.scancode) {
    case SDL_SCANCODE_TAB: cpu_ = !cpu_; break;
    case SDL_SCANCODE_F2: stage_ = (stage_ + 1) % stages_.size(); break;
    case SDL_SCANCODE_F5: restartRound(); break;
    default: break;
    }
}

void FightScene::fixedUpdate() {
    if (match_.outcome != sim::Outcome::Running) {
        if (++roundOverTicks_ >= kRoundOverDelayTicks) restartRound();
        return;
    }
    const std::array<sim::Input, 2> inputs{input_.sample(0), cpu_ ? sim::cpuInput(match_) : input_.sample(1)};
    input_.endTick();

    const auto events = match_.step(inputs);
    for (int i = 0; i < events.count; ++i) audio_.play(soundFor(events.items[static_cast<std::size_t>(i)].effect));
    if (match_.outcome != sim::Outcome::Running) audio_.play("round_bell");
}

void FightScene::render(const RenderContext& context) {
    SDL_Renderer* r = context.renderer;
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);

    const SDL_FRect full{0, 0, static_cast<float>(sim::kStageWidth), 360};
    SDL_RenderTexture(r, stages_[stage_].background->handle.get(), nullptr, &full);

    // The fighter acting last is drawn on top, so attacks read clearly.
    const bool p1Front = match_.fighters[0].move >= 0 && match_.fighters[1].move < 0;
    for (int n = 0; n < 2; ++n) {
        const int i = p1Front ? 1 - n : n;
        drawFighter(r, match_.fighters[static_cast<std::size_t>(i)], fighters_[static_cast<std::size_t>(i)]);
    }
    drawEffect(context);
    drawHud(r);
}

void FightScene::drawFighter(SDL_Renderer* r, const sim::Fighter& f, const FighterView& view) const {
    const int index = &f == &match_.fighters[0] ? 0 : 1;
    const SpriteFrame& frame = view.atlas->frame(poseFor(f, match_.tick, match_.outcome, index));
    const float scale = view.heightPixels / view.atlas->cellHeight();
    const float x = static_cast<float>(f.x) / sim::kFixed;
    const float y = static_cast<float>(sim::kFloorY) - static_cast<float>(f.y) / sim::kFixed;

    if (f.y == 0) { // contact shadow
        for (int row = -3; row <= 3; ++row) {
            const float w = static_cast<float>(38 - std::abs(row) * 5);
            fill(r, {x - w / 2, static_cast<float>(sim::kFloorY + row), w, 1}, {5, 15, 25, 100});
        }
    }
    const float pivotX = f.facing < 0 ? frame.source.w - frame.pivotX : frame.pivotX;
    const SDL_FRect dst{x - scale * pivotX, y - scale * frame.pivotY, scale * frame.source.w, scale * frame.source.h};
    SDL_RenderTextureRotated(r, view.atlas->texture().handle.get(), &frame.source, &dst, 0, nullptr,
                             f.facing < 0 ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
}

void FightScene::drawEffect(const RenderContext& context) const {
    if (match_.effectTicks <= 0) return;
    const SpriteFrame& frame = effects_->frame(static_cast<int>(match_.effect));
    const float size = match_.effect == sim::Effect::Breakaway ? 72.0f : 44.0f;
    const float x = static_cast<float>(match_.effectX) / sim::kFixed;
    const float y = static_cast<float>(sim::kFloorY) - static_cast<float>(match_.effectY) / sim::kFixed;
    const SDL_FRect dst{x - size / 2, y - size / 2, size, size};

    SDL_Texture* texture = effects_->texture().handle.get();
    SDL_SetTextureAlphaModFloat(texture, std::min(1.0f, static_cast<float>(match_.effectTicks) / 6.0f));
    // On HDR output the spark core exceeds SDR white; on SDR this is a no-op.
    SDL_SetRenderColorScale(context.renderer, context.highlightBoost);
    SDL_RenderTexture(context.renderer, texture, &frame.source, &dst);
    SDL_SetRenderColorScale(context.renderer, 1.0f);
    SDL_SetTextureAlphaModFloat(texture, 1.0f);
}

void FightScene::drawHud(SDL_Renderer* r) const {
    for (std::size_t i = 0; i < 2; ++i) {
        const auto& layout = hud_.at(i == 0 ? "p1" : "p2");
        const auto& f = match_.fighters[i];

        const SDL_FRect portrait = rectFrom(layout.at("portrait"));
        fill(r, portrait, kBrass);
        fill(r, {portrait.x + 2, portrait.y + 2, portrait.w - 4, portrait.h - 4}, i == 0 ? Rgba{21, 56, 68, 255} : Rgba{85, 43, 38, 255});

        const SDL_FRect bar = rectFrom(layout.at("health"));
        fill(r, bar, kBrass);
        fill(r, {bar.x + 2, bar.y + 2, bar.w - 4, bar.h - 4}, kDamage);
        const float health = (bar.w - 4) * static_cast<float>(f.hp) / 100.0f;
        fill(r, {i == 0 ? bar.x + 2 : bar.x + bar.w - 2 - health, bar.y + 2, health, bar.h - 4}, kCream);
        const float nameX = i == 0 ? bar.x : bar.x + bar.w - 8.0f * static_cast<float>(fighters_[i].displayName.size());
        text(r, nameX, bar.y + bar.h + 4, fighters_[i].displayName.c_str());

        const SDL_FRect meter = rectFrom(layout.at("meter"));
        fill(r, meter, kBrass);
        fill(r, {meter.x + 1, meter.y + 1, meter.w - 2, meter.h - 2}, kInk);
        fill(r, {meter.x + 1, meter.y + 1, (meter.w - 2) * static_cast<float>(f.meter) / sim::kMeterMax, meter.h - 2},
             f.meter >= sim::kBreakCost ? kTeal : Rgba{23, 107, 118, 255});
        text(r, meter.x, meter.y + meter.h + 2, meterName_.c_str());
    }

    const SDL_FRect timer = rectFrom(hud_.at("timer"));
    fill(r, timer, kBrass);
    fill(r, {timer.x + 2, timer.y + 2, timer.w - 4, timer.h - 4}, kInk);
    char clock[16];
    std::snprintf(clock, sizeof clock, "%02d", std::max(0, (match_.remaining + sim::kTickRate - 1) / sim::kTickRate));
    text(r, timer.x + timer.w / 2 - 8, timer.y + 8, clock);

    if (match_.outcome != sim::Outcome::Running) {
        const char* banner = match_.outcome == sim::Outcome::Draw ? "DRAW" : "K.O.";
        text(r, 320 - 16, 150, banner, kCoral);
    }
}

std::string FightScene::status() const {
    char line[256];
    std::snprintf(line, sizeof line, "%s | %s | tick %d | Tab: CPU/2P  F2: stage  F5: reset",
                  stages_[stage_].name.c_str(), cpu_ ? "vs CPU" : "2P local", match_.tick);
    return line;
}

} // namespace iron
