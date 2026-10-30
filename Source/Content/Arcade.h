#pragma once

// The arcade ladder (Assets/Data/arcade.json): a run of bouts against the
// CPU, venue by venue, ending at The Crown.
//
//   { "bouts": [ { "stage": "pier_nine", "opponent": "rival", "level": 1 }, ...,
//                { "stage": "the_crown", "opponent": "rival", "level": 3, "final": true } ] }
//
// opponent: "rival" (the first playable fighter who isn't the player),
// "mirror" (the player's own fighter), or a fighter id. level: the CPU level
// (0 ROOKIE .. 4 CHAMPION) at NORMAL difficulty; EASY plays one lower, HARD
// one higher.

#include <nlohmann/json.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace iron::content {

struct ArcadeBout {
    std::string stage;
    std::string opponent; // "rival", "mirror" or a fighter id
    int level = 1;
    bool final = false;
};

struct ArcadeLadder {
    std::vector<ArcadeBout> bouts;
};

[[nodiscard]] ArcadeLadder parseArcade(const nlohmann::json& json, std::string_view path);

// The opponent's fighter id for a bout. `playable` lists the playable fighter
// ids in roster order.
[[nodiscard]] std::string arcadeOpponent(const ArcadeBout& bout, const std::string& player,
                                         const std::vector<std::string>& playable);
// The CPU level for a bout at a difficulty (0 EASY, 1 NORMAL, 2 HARD), within 0..4.
[[nodiscard]] int arcadeLevel(const ArcadeBout& bout, int difficulty);

// Where a run stands. Pure state, so the rules are unit-tested.
struct ArcadeRun {
    enum class Step { Versus, Fight, Continue, GameOver, Ending };
    std::size_t bout = 0;
    Step step = Step::Versus;
    int continuesUsed = 0;

    void startFight() { step = Step::Fight; }
    // The fight's result: a win moves to the next bout (or the ending), a loss offers a continue.
    void finished(bool playerWon, std::size_t bouts);
    void continueRun() { ++continuesUsed, step = Step::Versus; } // same bout again
    void giveUp() { step = Step::GameOver; }
};

} // namespace iron::content
