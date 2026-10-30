#include "Simulation/Training.h"

namespace iron::sim {

std::string_view dummyName(Dummy mode) {
    switch (mode) {
    case Dummy::Stand: return "STAND";
    case Dummy::Crouch: return "CROUCH";
    case Dummy::Jump: return "JUMP";
    case Dummy::Guard: return "GUARD";
    case Dummy::GuardAll: return "GUARD ALL";
    case Dummy::Cpu: return "CPU";
    }
    return "";
}

std::optional<Dummy> parseDummy(std::string_view name) {
    for (const Dummy mode : kDummyModes) {
        const auto canonical = dummyName(mode);
        if (canonical.size() != name.size()) continue;
        bool same = true;
        for (std::size_t i = 0; i < name.size() && same; ++i) {
            const char a = name[i] == '-' ? ' ' : name[i];
            const char b = canonical[i];
            same = (a == b) || (a >= 'a' && a <= 'z' && static_cast<char>(a - 'a' + 'A') == b);
        }
        if (same) return mode;
    }
    return std::nullopt;
}

Dummy nextDummy(Dummy mode) {
    return kDummyModes[(static_cast<std::size_t>(mode) + 1) % kDummyModes.size()];
}

Input dummyInput(const Round& r, int side, Dummy mode) {
    if (mode == Dummy::Cpu) return cpuInput(r, side);
    const auto& self = r.fighters[static_cast<std::size_t>(side)];
    const auto& other = r.fighters[static_cast<std::size_t>(1 - side)];
    if (self.heldTicks > 0) return (r.tick / 7) % 2 == 0 ? Input(LightPunch | LightKick) : Input(0);

    switch (mode) {
    case Dummy::Stand: return 0;
    case Dummy::Crouch: return Input(Down);
    case Dummy::Jump: return self.y == 0 && r.tick % 40 == 0 ? Input(Up) : Input(0);
    case Dummy::Guard: return Input(Guard);
    case Dummy::GuardAll: {
        const bool low = other.move >= 0 && r.currentMove(other).low;
        return low ? Input(Guard | Down) : Input(Guard);
    }
    case Dummy::Cpu: break;
    }
    return 0;
}

} // namespace iron::sim
