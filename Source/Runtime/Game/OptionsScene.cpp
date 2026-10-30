#include "Game/OptionsScene.h"

#include "Game/ControlsScene.h"
#include "Game/MenuView.h"
#include "Render/Ui.h"

namespace iron {
namespace {

constexpr int kSteps = 10;

} // namespace

OptionsScene::OptionsScene(Services& services) : services_(services) {
    const auto& s = services_.settings;
    menu_.add(MenuItem::level("music", "MUSIC", kSteps, levelIndex(s.musicVolume, kSteps)));
    menu_.add(MenuItem::level("sfx", "EFFECTS", kSteps, levelIndex(s.sfxVolume, kSteps)));
    menu_.add(MenuItem::level("voice", "ANNOUNCER", kSteps, levelIndex(s.voiceVolume, kSteps)));
    menu_.add(MenuItem::setting("window", "DISPLAY", {"FULLSCREEN", "WINDOW"},
                                services_.display.fullscreen() ? 0 : 1));
    auto& scaling = menu_.add(MenuItem::setting("scaling", "SCALING", {"PIXEL PERFECT", "FILL"},
                                                s.presentation == Presentation::IntegerScale ? 0 : 1));
    scaling.note = scaling.choice == 0 ? "WHOLE-PIXEL STEPS, BORDERS AROUND" : "AS LARGE AS FITS";
    menu_.add(MenuItem::setting("vsync", "VSYNC", {"OFF", "ON"}, s.vsync ? 1 : 0));
    auto& hdr = menu_.add(MenuItem::setting("hdr", "HDR", {"AUTO", "ON", "OFF"}, static_cast<int>(s.hdr)));
    hdr.note = "BRIGHTER HIGHLIGHTS ON HDR DISPLAYS";
    menu_.add(MenuItem::action("controls", "CONTROLS"));
    menu_.add(MenuItem::action("back", "BACK"));
}

void OptionsScene::apply(const MenuItem& item) {
    auto& s = services_.settings;
    changed_ = true;
    if (item.id == "music" || item.id == "sfx" || item.id == "voice") {
        const float level = levelValue(item.choice, kSteps);
        (item.id == "music" ? s.musicVolume : item.id == "sfx" ? s.sfxVolume : s.voiceVolume) = level;
        services_.audio.setVolumes(s.musicVolume, s.sfxVolume, s.voiceVolume);
        if (item.id == "voice") services_.audio.playVoice("fight"); // hear the new level
    } else if (item.id == "window") {
        s.windowMode = item.choice == 0 ? WindowMode::Fullscreen : WindowMode::Windowed;
        services_.display.setFullscreen(item.choice == 0);
    } else if (item.id == "scaling") {
        s.presentation = item.choice == 0 ? Presentation::IntegerScale : Presentation::Letterbox;
        services_.display.setPresentation(s.presentation);
        menu_.find("scaling")->note = item.choice == 0 ? "WHOLE-PIXEL STEPS, BORDERS AROUND" : "AS LARGE AS FITS";
    } else if (item.id == "vsync") {
        s.vsync = item.choice == 1;
        services_.display.setVsync(s.vsync);
    } else if (item.id == "hdr") {
        s.hdr = static_cast<HdrMode>(item.choice);
        services_.resetRenderer(); // scRGB or sRGB output: a renderer of its own
    }
}

void OptionsScene::close() {
    if (changed_) {
        services_.prefs.storeSettings(services_.settings);
        if (!services_.settings.batchMode && !services_.prefs.save())
            SDL_Log("Options: cannot save %s", services_.prefs.file().string().c_str());
    }
    services_.scenes.pop();
}

void OptionsScene::handleMenu(const MenuEvent& event) {
    switch (navigate(menu_, event, services_.audio)) {
    case MenuResult::Changed: apply(menu_.focused()); break;
    case MenuResult::Chosen:
        if (menu_.focused().id == "back") close();
        else if (menu_.focused().id == "controls") services_.scenes.push(std::make_unique<ControlsScene>(services_));
        break;
    case MenuResult::Back: close(); break;
    default: break;
    }
}

void OptionsScene::render(const RenderContext& context) {
    SDL_Renderer* r = context.renderer;
    const auto& data = services_.data;
    const SDL_FRect all{0, 0, services_.width, services_.height};
    SDL_RenderTexture(r, data.backdrop().handle.get(), nullptr, &all);
    data.large().draw(r, "OPTIONS", 320, 34, ui::kWhite, Align::Centre);
    const SDL_FRect panel{170, 82, 300, 156}; // nine lines; the focused note goes below it
    ui::frame(r, panel);
    ui::fill(r, panel.x, panel.y, panel.w, panel.h, ui::kNight);
    MenuStyle style;
    style.y = 98;
    style.width = 250;
    style.spacing = 16;
    style.noteGap = 10;
    drawMenu(r, menu_, data.small(), style, blink_);
    if (!services_.prefs.file().empty())
        data.small().draw(r, "CHANGES ARE SAVED", 320, 322, ui::kFog, Align::Centre, true);
}

std::string OptionsScene::status() const { return "options: " + menu_.focused().id; }

} // namespace iron
