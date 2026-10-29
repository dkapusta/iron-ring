#pragma once

#include "Game/Scene.h"
#include "Render/SpriteSheet.h"
#include "Simulation/Round.h"

#include <string>
#include <vector>

namespace iron {

// Short-lived sprite effects spawned from simulation events (sparks, guard
// flashes, the Breakaway ring, slam dust). Purely presentational.
class Effects {
public:
    Effects(AssetLibrary& assets, std::string_view atlasPath);

    void spawn(const sim::StepEvents::Event& event, int clock);
    void update(int clock);
    void draw(const RenderContext& context, int clock) const;
    void clear() { active_.clear(); }

private:
    struct Active {
        std::string name;
        int frames = 1;
        float x = 0, y = 0;
        int start = 0;
    };
    static constexpr int kTicksPerFrame = 3;

    SpriteSheet sheet_;
    std::vector<Active> active_;
};

} // namespace iron
