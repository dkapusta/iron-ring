#include "Content/Arcade.h"

#include "Content/Fighter.h" // ContentError

#include <algorithm>

namespace iron::content {

ArcadeLadder parseArcade(const nlohmann::json& j, std::string_view path) {
    ArcadeLadder ladder;
    for (const auto& b : j.at("bouts")) {
        ArcadeBout bout;
        bout.stage = b.at("stage").get<std::string>();
        if (const auto it = b.find("opponent"); it != b.end()) {
            bout.opponents.clear();
            if (it->is_array())
                for (const auto& id : *it) bout.opponents.push_back(id.get<std::string>());
            else
                bout.opponents.push_back(it->get<std::string>());
            if (bout.opponents.empty())
                throw ContentError(std::string(path) + ": bout " + std::to_string(ladder.bouts.size() + 1) +
                                   ": opponent list is empty");
        }
        bout.level = b.value("level", 1);
        bout.final = b.value("final", false);
        bout.boss = b.value("boss", false);
        if (bout.level < 0 || bout.level > 4)
            throw ContentError(std::string(path) + ": bout " + std::to_string(ladder.bouts.size() + 1) +
                               ": level must be 0..4");
        ladder.bouts.push_back(std::move(bout));
    }
    if (ladder.bouts.empty()) throw ContentError(std::string(path) + ": no bouts");
    return ladder;
}

std::vector<std::string> arcadeOpponents(const ArcadeLadder& ladder, const std::string& player,
                                         const std::vector<std::string>& playable) {
    std::vector<std::string> met;
    const auto lastMet = [&](const std::string& id) {
        const auto it = std::find(met.rbegin(), met.rend(), id);
        return it == met.rend() ? -1 : static_cast<int>(met.rend() - it) - 1;
    };
    const auto eligible = [&](const std::string& id) {
        return id != player && std::find(playable.begin(), playable.end(), id) != playable.end();
    };
    for (const auto& bout : ladder.bouts) {
        if (bout.boss) { // not a roster fighter: always the one named
            met.push_back(bout.opponents.front());
            continue;
        }
        std::string pick;
        const auto listed = [&](bool includeMet) {
            for (const auto& c : bout.opponents)
                if (c != "rival" && c != "mirror" && eligible(c) && (includeMet || lastMet(c) < 0)) return c;
            return std::string();
        };
        if (bout.opponents.front() == "mirror") pick = player;
        // 1. a listed fighter not met yet; 2. anyone not met yet; 3. a listed fighter again;
        // 4. whoever was met longest ago; 5. a roster of one fights itself.
        if (pick.empty()) pick = listed(false);
        if (pick.empty())
            for (const auto& id : playable)
                if (eligible(id) && lastMet(id) < 0) {
                    pick = id;
                    break;
                }
        if (pick.empty()) pick = listed(true);
        if (pick.empty())
            for (const auto& id : playable)
                if (eligible(id) && (pick.empty() || lastMet(id) < lastMet(pick))) pick = id;
        if (pick.empty()) pick = player;
        met.push_back(pick);
    }
    return met;
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
