#pragma once

// The arcade ladder (Assets/Data/arcade.json): a run of bouts against the
// CPU, venue by venue, ending at The Crown.
//
//   { "bouts": [ { "stage": "pier_nine", "opponent": "rival", "level": 1 }, ...,
//                { "stage": "the_crown", "opponent": "rival", "level": 3, "final": true } ] }
//
// opponent: a fighter id, "rival" (a playable fighter who isn't the player),
// "mirror" (the player's own fighter; only as the first entry), or a list of
// these: the first that is playable, isn't the player and hasn't been met
// earlier in the run. So one
// ladder serves every fighter and a growing roster: nobody fights themselves
// by accident or meets the same opponent twice while others are left. level:
// the CPU level (0 ROOKIE .. 4 CHAMPION) at NORMAL difficulty; EASY plays one
// lower, HARD one higher. final: the title bout. boss: a bout after it against
// a fighter nobody can pick (the Broker), named by the first opponent entry.

#include <nlohmann/json.hpp>

#include <string>
#include <string_view>
#include <vector>

namespace iron::content {

struct ArcadeBout {
    std::string stage;
    std::vector<std::string> opponents{"rival"}; // candidates in order: fighter ids, "rival", "mirror"
    int level = 1;
    bool final = false; // the title bout ("FINAL BOUT")
    bool boss = false;  // after the title: the first opponent listed, whoever the player is
};

struct ArcadeLadder {
    std::vector<ArcadeBout> bouts;
};

[[nodiscard]] ArcadeLadder parseArcade(const nlohmann::json& json, std::string_view path);

// Every bout's opponent for a run as `player`, in ladder order. `playable`
// lists the playable fighter ids in roster order.
[[nodiscard]] std::vector<std::string> arcadeOpponents(const ArcadeLadder& ladder, const std::string& player,
                                                       const std::vector<std::string>& playable);
// The CPU level for a bout at a difficulty (0 EASY, 1 NORMAL, 2 HARD), within 0..4.
[[nodiscard]] int arcadeLevel(const ArcadeBout& bout, int difficulty);

// Where a run stands. Pure state, so the rules are unit-tested.
struct ArcadeRun {
    enum class Step { Story, Versus, Fight, Continue, GameOver, Ending };
    std::size_t bout = 0;
    Step step = Step::Story; // the player's intro first
    int continuesUsed = 0;

    void begin() { step = Step::Versus; }
    void startFight() { step = Step::Fight; }
    // The fight's result: a win moves to the next bout (or the ending), a loss offers a continue.
    void finished(bool playerWon, std::size_t bouts);
    void continueRun() { ++continuesUsed, step = Step::Versus; } // same bout again
    void giveUp() { step = Step::GameOver; }
};

} // namespace iron::content
