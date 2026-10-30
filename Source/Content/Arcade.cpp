#include "Content/Arcade.h"

#include "Content/Fighter.h" // ContentError

#include <algorithm>

namespace iron::content {

ArcadeLadder parseArcade(const nlohmann::json& j, std::string_view path) {
    ArcadeLadder ladder;
    for (const auto& b : j.at("bouts")) {
        ArcadeBout bout;
        bout.stage = b.at("stage").get<std::string>();
        bout.opponent = b.value("opponent", std::string("rival"));
        bout.level = b.value("level", 1);
        bout.final = b.value("final", false);
        if (bout.level < 0 || bout.level > 4)
            throw ContentError(std::string(path) + ": bout " + std::to_string(ladder.bouts.size() + 1) +
                               ": level must be 0..4");
        ladder.bouts.push_back(std::move(bout));
    }
    if (ladder.bouts.empty()) throw ContentError(std::string(path) + ": no bouts");
    return ladder;
}

std::string arcadeOpponent(const ArcadeBout& bout, const std::string& player, const std::vector<std::string>& playable) {
    if (bout.opponent == "mirror") return player;
    if (bout.opponent == "rival") {
        for (const auto& id : playable)
            if (id != player) return id;
        return player; // a roster of one fights itself
    }
    return std::find(playable.begin(), playable.end(), bout.opponent) != playable.end() ? bout.opponent : player;
}

int arcadeLevel(const ArcadeBout& bout, int difficulty) { return std::clamp(bout.level + difficulty - 1, 0, 4); }

void ArcadeRun::finished(bool playerWon, std::size_t bouts) {
    if (!playerWon) {
        step = Step::Continue;
        return;
    }
    if (++bout >= bouts) {
        bout = bouts - 1;
        step = Step::Ending;
        return;
    }
    step = Step::Versus;
}

} // namespace iron::content
