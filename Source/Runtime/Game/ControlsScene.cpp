#include "Game/ControlsScene.h"

#include "Game/MenuView.h"
#include "Render/Ui.h"

namespace iron {
namespace {

std::string actionLabel(std::string_view taken) {
    // "p1.heavyKick" -> "PLAYER 1 HEAVY KICK"
    const int action = actionIndex(taken.substr(3));
    const std::string who = taken[1] == '1' ? "PLAYER 1 " : "PLAYER 2 ";
    return who + (action >= 0 ? std::string(kActionLabels[static_cast<std::size_t>(action)]) : std::string(taken));
}

} // namespace

ControlsScene::ControlsScene(Services& services) : services_(services) {
    menu_.add(MenuItem::setting("player", "PLAYER", {"1", "2"}, 0));
    for (int a = 0; a < kActionCount; ++a)
        menu_.add(MenuItem::action("a" + std::to_string(a), std::string(kActionLabels[static_cast<std::size_t>(a)])));
    menu_.add(MenuItem::action("resetKeys", "RESET KEYS"));
    menu_.add(MenuItem::action("resetPad", "RESET PAD"));
    menu_.add(MenuItem::action("back", "BACK"));
}

void ControlsScene::finishCapture(std::string message) {
    capturing_ = -1;
    swallow_ = true;
    changed_ = true;
    message_ = std::move(message);
    messageTicks_ = 180;
}

void ControlsScene::handleEvent(const SDL_Event& event) {
    if (capturing_ < 0) return;
    auto& controls = services_.controls;
    const std::string label(kActionLabels[static_cast<std::size_t>(capturing_)]);
    if (event.type == SDL_EVENT_KEY_DOWN && !event.key.repeat) {
        if (event.key.scancode == SDL_SCANCODE_ESCAPE) {
            capturing_ = -1;
            swallow_ = true;
            services_.audio.play("menu_back");
            return;
        }
        if (!Controls::bindableKey(event.key.scancode)) return;
        const auto taken = controls.bindKey(player(), capturing_, event.key.scancode);
        services_.audio.play("menu_confirm");
        std::string message = label + " = " + controls.keyLabel(player(), capturing_);
        if (!taken.empty()) message += "  (" + actionLabel(taken.front()) + " IS NOW UNBOUND)";
        finishCapture(message);
        return;
    }
    if (capturing_ < kFirstButtonAction) return; // pads always move with the d-pad and stick
    if (const auto pad = controls.padInputFor(event); pad && pad->first == player()) {
        controls.bindPadButton(player(), capturing_, pad->second);
        services_.audio.play("menu_confirm");
        finishCapture(label + " = PAD " + controls.padLabel(player(), capturing_));
    }
}

void ControlsScene::handleMenu(const MenuEvent& event) {
    if (capturing_ >= 0 || swallow_) return;
    switch (navigate(menu_, event, services_.audio)) {
    case MenuResult::Chosen: {
        const auto& id = menu_.focused().id;
        if (id == "back") {
            close();
        } else if (id == "resetKeys") {
            services_.controls.resetKeys(player());
            changed_ = true;
            message_ = "PLAYER " + std::to_string(player() + 1) + " KEYS RESET";
            messageTicks_ = 120;
        } else if (id == "resetPad") {
            services_.controls.resetPad(player());
            changed_ = true;
            message_ = "PAD LAYOUT RESET";
            messageTicks_ = 120;
        } else if (id[0] == 'a') {
            capturing_ = std::stoi(id.substr(1));
            captureLeft_ = kCaptureTicks;
        }
        break;
    }
    case MenuResult::Back: close(); break;
    default: break;
    }
}

void ControlsScene::fixedUpdate() {
    ++clock_;
    swallow_ = false; // events come before ticks: anything left from the capturing press is gone
    if (capturing_ >= 0 && --captureLeft_ <= 0) capturing_ = -1;
    if (messageTicks_ > 0 && --messageTicks_ == 0) message_.clear();
    menu_.find("resetPad")->enabled = services_.controls.hasPad(player());
}

void ControlsScene::close() {
    if (changed_) {
        services_.controls.storeBindings(services_.prefs);
        if (!services_.settings.batchMode && !services_.prefs.save())
            SDL_Log("Controls: cannot save %s", services_.prefs.file().string().c_str());
    }
    services_.scenes.pop();
}

void ControlsScene::render(const RenderContext& context) {
    SDL_Renderer* r = context.renderer;
    const auto& data = services_.data;
    const auto& font = data.small();
    const auto& controls = services_.controls;
    const float w = services_.width;
    const SDL_FRect all{0, 0, w, services_.height};
    SDL_RenderTexture(r, data.backdrop().handle.get(), nullptr, &all);
    data.large().draw(r, "CONTROLS", w / 2, 14, ui::kWhite, Align::Centre);

    const int p = player();
    const SDL_FRect panel{150, 66, 340, 236};
    ui::frame(r, panel);
    ui::fill(r, panel.x, panel.y, panel.w, panel.h, ui::kNight);
    const float left = panel.x + 10, keyX = panel.x + 230, padX = panel.x + panel.w - 10;
    const std::string pad = controls.hasPad(p) ? ui::upper(controls.padName(p)) : "NO PAD";
    font.draw(r, "PAD: " + pad.substr(0, 40), w / 2, 50, ui::kFog, Align::Centre, true);

    float y = panel.y + 8;
    for (int i = 0; i < static_cast<int>(menu_.items().size()); ++i) {
        const auto& item = menu_.items()[static_cast<std::size_t>(i)];
        const bool focused = i == menu_.focus();
        if (focused) {
            ui::fill(r, panel.x + 3, y - 2, panel.w - 6, 11, ui::kSteel);
            ui::fill(r, panel.x + 3, y - 2, 2, 11, ui::kBrassLight);
        }
        const SDL_Color colour = !item.enabled ? ui::kSteel : focused ? ui::kBrassLight : ui::kChalk;
        if (item.id == "player") {
            const bool arrows = focused && (clock_ / 16) % 2 == 0;
            font.draw(r, arrows ? "< PLAYER " + std::to_string(p + 1) + " >" : "  PLAYER " + std::to_string(p + 1) + "  ",
                      w / 2, y, ui::kSide[p], Align::Centre, true);
            y += 6;
            font.draw(r, "KEY", keyX, y + 8, ui::kFog, Align::Centre, true);
            font.draw(r, "PAD", padX, y + 8, ui::kFog, Align::Right, true);
            y += 8;
        } else if (item.id[0] == 'a') {
            const int a = std::stoi(item.id.substr(1));
            font.draw(r, item.label, left, y, colour, Align::Left, true);
            const bool waiting = capturing_ == a;
            const std::string key = waiting ? ((clock_ / 10) % 2 ? "?" : "") : controls.keyLabel(p, a);
            font.draw(r, key, keyX, y, waiting ? ui::kWhite : ui::kChalk, Align::Centre, true);
            const std::string padLabel = !controls.hasPad(p) ? "" : waiting && a >= kFirstButtonAction
                                                                    ? ((clock_ / 10) % 2 ? "?" : "")
                                                                    : controls.padLabel(p, a);
            font.draw(r, padLabel, padX, y, a < kFirstButtonAction ? ui::kSteel : ui::kChalk, Align::Right, true);
        } else {
            if (item.id == "resetKeys") y += 6;
            font.draw(r, item.label, w / 2, y, colour, Align::Centre, true);
        }
        y += 14;
    }

    std::string hint;
    if (capturing_ >= 0)
        hint = "PRESS A KEY" + std::string(controls.hasPad(p) && capturing_ >= kFirstButtonAction ? " OR PAD BUTTON" : "") +
               " FOR " + std::string(kActionLabels[static_cast<std::size_t>(capturing_)]) + " - ESC CANCELS";
    else if (!message_.empty())
        hint = message_;
    else
        hint = "LEFT/RIGHT ON PLAYER SWITCHES - CONFIRM AN ACTION TO REBIND";
    font.draw(r, hint, w / 2, 312, capturing_ >= 0 ? ui::kWhite : ui::kTealLight, Align::Centre, true);
    font.draw(r, "MENUS FOLLOW LIGHT PUNCH (CONFIRM) AND LIGHT KICK (BACK)", w / 2, 340, ui::kFog, Align::Centre, true);
}

std::string ControlsScene::status() const {
    return "controls: player " + std::to_string(player() + 1) + (capturing_ >= 0 ? " (waiting for a press)" : "");
}

} // namespace iron
