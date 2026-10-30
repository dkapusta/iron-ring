#pragma once

#include "Core/Menu.h"
#include "Game/Scene.h"
#include "Game/Services.h"

namespace iron {

// Options: sound levels and display settings. Changes apply at once (HDR on
// the next launch, since it needs a new renderer) and are saved to prefs.cfg
// when the screen closes.
class OptionsScene final : public Scene {
public:
    explicit OptionsScene(Services& services);

    void handleMenu(const MenuEvent& event) override;
    void fixedUpdate() override { ++blink_; }
    void render(const RenderContext& context) override;
    [[nodiscard]] std::string status() const override;

private:
    void apply(const MenuItem& item);
    void close();

    Services& services_;
    Menu menu_;
    bool changed_ = false;
    int blink_ = 0;
};

} // namespace iron
