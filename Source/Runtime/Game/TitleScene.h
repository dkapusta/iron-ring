#pragma once

#include "Core/Menu.h"
#include "Game/Scene.h"
#include "Game/Services.h"

namespace iron {

// The title: the logo over a venue, "PRESS START", then the main menu. Left
// alone, it runs a CPU-vs-CPU demo fight (the attract mode), as arcade
// cabinets do.
class TitleScene final : public Scene {
public:
    explicit TitleScene(Services& services);

    void handleMenu(const MenuEvent& event) override;
    void fixedUpdate() override;
    void render(const RenderContext& context) override;
    [[nodiscard]] std::string status() const override;
    void onEnter() override;

private:
    static constexpr int kAttractTicks = 60 * 20; // idle time before the demo starts

    void choose(const std::string& id);
    void startAttract();

    Services& services_;
    Menu menu_;
    bool showMenu_ = false;
    bool fromAttract_ = false;
    int idle_ = 0;
    int clock_ = 0;
    std::size_t venue_ = 0; // backdrop and the next demo's stage
};

} // namespace iron
