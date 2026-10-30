#include "Content/Fighter.h"

#include "Content/Paths.h"
#include "Simulation/Round.h"

#include <algorithm>

namespace iron::content {
namespace {

using nlohmann::json;

[[noreturn]] void fail(std::string_view path, const std::string& what) {
    throw ContentError(std::string(path) + ": " + what);
}

sim::Box parseBox(const json& j, std::string_view path, const std::string& field) {
    if (!j.is_array() || j.size() != 4) fail(path, field + " must be [x, y, w, h]");
    return {j[0].get<int>(), j[1].get<int>(), j[2].get<int>(), j[3].get<int>()};
}

template <typename T>
void read(const json& object, const char* key, T& out) {
    if (const auto it = object.find(key); it != object.end()) out = it->get<T>();
}

sim::MoveSpec parseMove(const json& j, std::string_view path, const std::string& key, MoveInfo& info) {
    sim::MoveSpec m;
    const std::string kind = j.value("kind", "strike");
    if (kind == "throw") m.kind = sim::MoveKind::Throw;
    else if (kind != "strike") fail(path, "moves." + key + ".kind must be \"strike\" or \"throw\"");

    read(j, "startup", m.startup);
    read(j, "active", m.active);
    read(j, "recovery", m.recovery);
    if (m.startup < 1 || m.active < 1 || m.recovery < 0) fail(path, "moves." + key + ": bad frame data");
    m.hitbox = parseBox(j.at("hitbox"), path, "moves." + key + ".hitbox");
    read(j, "damage", m.damage);
    read(j, "hitstun", m.hitstun);
    read(j, "blockstun", m.blockstun);
    read(j, "knockback", m.knockback);
    read(j, "blockPush", m.blockPush);
    read(j, "hitstopOnHit", m.hitstopOnHit);
    read(j, "hitstopOnBlock", m.hitstopOnBlock);
    read(j, "low", m.low);
    read(j, "heavy", m.heavy);
    read(j, "knockdown", m.knockdown);
    read(j, "advance", m.advance);
    read(j, "throwDuration", m.throwDuration);
    read(j, "throwDistance", m.throwDistance);
    read(j, "techable", m.techable);
    read(j, "cancel", m.cancel);
    if (j.contains("hurt")) {
        constexpr std::array<std::string_view, sim::kPhaseCount> phases{"startup", "active", "recovery"};
        const auto& hurt = j.at("hurt");
        for (const auto& [phase, boxes] : hurt.items())
            if (std::find(phases.begin(), phases.end(), phase) == phases.end())
                fail(path, "moves." + key + ".hurt." + phase + ": expected startup, active or recovery");
        for (std::size_t p = 0; p < phases.size(); ++p) {
            const std::string name(phases[p]);
            if (!hurt.contains(name)) continue;
            const auto& boxes = hurt.at(name);
            if (!boxes.is_array() || boxes.empty() || boxes.size() > static_cast<std::size_t>(sim::kMaxHurtBoxes))
                fail(path, "moves." + key + ".hurt." + name + ": 1 or 2 boxes");
            for (std::size_t b = 0; b < boxes.size(); ++b)
                m.hurt[p][b] = parseBox(boxes[b], path, "moves." + key + ".hurt." + name);
        }
    }

    info.key = key;
    info.name = j.value("name", key);
    info.animation = j.at("animation").get<std::string>();
    info.holdAnimation = j.value("holdAnimation", "");
    info.input = j.value("input", "");
    if (!info.input.empty()) {
        try {
            parseMotion(info.input, m);
        } catch (const ContentError& e) {
            fail(path, "moves." + key + ".input: " + e.what());
        }
    }
    if (m.kind == sim::MoveKind::Throw && info.holdAnimation.empty())
        fail(path, "moves." + key + ": throws need a holdAnimation");
    return m;
}

Animation parseAnimation(const json& j, std::string_view path, const std::string& name) {
    Animation a;
    a.loop = j.value("loop", false);
    for (const auto& f : j.at("frames")) {
        if (!f.is_array() || f.size() < 2 || f.size() > 3)
            fail(path, "animations." + name + ": frames are [name, ticks] or [name, ticks, partnerFrame]");
        AnimationFrame frame{f[0].get<std::string>(), f[1].get<int>(), f.size() == 3 ? f[2].get<std::string>() : ""};
        if (frame.ticks < 1) fail(path, "animations." + name + ": frame ticks must be >= 1");
        a.frames.push_back(std::move(frame));
    }
    if (a.frames.empty()) fail(path, "animations." + name + " has no frames");
    return a;
}

} // namespace

int Animation::totalTicks() const {
    int total = 0;
    for (const auto& f : frames) total += f.ticks;
    return total;
}

const AnimationFrame& Animation::at(int tick) const {
    const int total = totalTicks();
    if (loop && total > 0) tick %= total;
    tick = std::max(tick, 0);
    for (const auto& f : frames) {
        if (tick < f.ticks) return f;
        tick -= f.ticks;
    }
    return frames.back();
}

const Animation& FighterDefinition::animation(std::string_view animationName) const {
    const auto it = animations.find(animationName);
    if (it == animations.end()) throw ContentError(id + ": no animation '" + std::string(animationName) + "'");
    return it->second;
}

bool FighterDefinition::hasAnimation(std::string_view animationName) const {
    return animations.find(animationName) != animations.end();
}

void parseMotion(std::string_view notation, sim::MoveSpec& move) {
    std::size_t i = 0;
    move.motionLength = 0;
    while (i < notation.size() && notation[i] >= '1' && notation[i] <= '9') {
        if (move.motionLength == sim::kMaxMotionLength) throw ContentError("motion longer than 6 directions");
        move.motion[static_cast<std::size_t>(move.motionLength++)] = static_cast<std::uint8_t>(notation[i++] - '0');
    }
    const std::string_view button = notation.substr(i);
    if (move.motionLength == 0) throw ContentError("'" + std::string(notation) + "' has no directions");
    if (button == "P") move.motionButtons = sim::LightPunch | sim::HeavyPunch;
    else if (button == "K") move.motionButtons = sim::LightKick | sim::HeavyKick;
    else if (button == "LP") move.motionButtons = sim::LightPunch;
    else if (button == "HP") move.motionButtons = sim::HeavyPunch;
    else if (button == "LK") move.motionButtons = sim::LightKick;
    else if (button == "HK") move.motionButtons = sim::HeavyKick;
    else throw ContentError("'" + std::string(notation) + "': button must be P, K, LP, HP, LK or HK");
}

FighterDefinition parseFighter(const json& j, std::string_view path) {
    try {
        FighterDefinition d;
        d.id = j.at("id").get<std::string>();
        d.name = j.at("name").get<std::string>();
        d.displayName = j.value("displayName", d.name);
        d.atlas = siblingPath(path, j.at("atlas").get<std::string>());
        d.portrait = siblingPath(path, j.at("portrait").get<std::string>());
        if (j.contains("selectPortrait")) d.selectPortrait = siblingPath(path, j.at("selectPortrait").get<std::string>());
        if (j.contains("ending")) d.ending = siblingPath(path, j.at("ending").get<std::string>());

        auto& s = d.spec;
        const auto& physics = j.at("physics");
        read(physics, "walkForward", s.walkForward);
        read(physics, "walkBack", s.walkBack);
        read(physics, "jumpVelocity", s.jumpVelocity);
        read(physics, "gravity", s.gravity);
        const auto& body = j.at("body");
        s.standHurt = parseBox(body.at("standHurt"), path, "body.standHurt");
        s.crouchHurt = parseBox(body.at("crouchHurt"), path, "body.crouchHurt");
        read(body, "pushWidth", s.pushWidth);
        read(body, "knockdownTicks", s.knockdownTicks);

        const auto& moves = j.at("moves");
        for (std::size_t slot = 0; slot < kMoveKeys.size(); ++slot) {
            const std::string key(kMoveKeys[slot]);
            const auto it = moves.find(key);
            if (it == moves.end()) {
                if (slot < sim::ThrowSlot) fail(path, "moves." + key + " is required");
                s.moves[slot].enabled = false;
                continue;
            }
            s.moves[slot] = parseMove(*it, path, key, d.moves[slot]);
        }
        if (s.moves[sim::SpecialSlot].enabled && s.moves[sim::SpecialSlot].motionLength == 0)
            fail(path, "moves.special needs an input such as \"236P\"");

        for (const auto& [name, animation] : j.at("animations").items())
            d.animations.emplace(name, parseAnimation(animation, path, name));
        return d;
    } catch (const json::exception& e) {
        fail(path, e.what());
    }
}

} // namespace iron::content
