#include "Game/StageSelectScene.h"

#include "Render/Ui.h"

#include <cmath>

namespace iron {

StageSelectScene::StageSelectScene(Services& services, FightSetup setup)
    : services_(services), setup_(std::move(setup)) {
    stage_ = services_.data.stageIndex(services_.prefs.get("last-stage").value_or(""));
}

void StageSelectScene::handleMenu(const MenuEvent& event) {
    const auto count = services_.data.stages().size();
    switch (event.action) {
    case MenuAction::Left:
    case MenuAction::Right:
    case MenuAction::Up:
    case MenuAction::Down: {
        const bool back = event.action == MenuAction::Left || event.action == MenuAction::Up;
        stage_ = (stage_ + (back ? count - 1 : 1)) % count;
        slide_ = back ? -1 : 1;
        slideTicks_ = clock_;
        services_.audio.play("menu_move");
        break;
    }
    case MenuAction::Confirm: {
        services_.audio.play("menu_confirm");
        const auto& id = services_.data.stages()[stage_].id;
        services_.prefs.set("last-stage", id);
        if (!services_.settings.batchMode) (void)services_.prefs.save();
        setup_.stage = id;
        // The fight takes this screen's place: leaving the fight returns to the character select.
        services_.scenes.replaceTop(std::make_unique<FightScene>(services_, setup_));
        break;
    }
    case MenuAction::Back:
    case MenuAction::Pause:
        services_.audio.play("menu_back");
        services_.scenes.pop();
        break;
    case MenuAction::Info: break;
    }
}

void StageSelectScene::render(const RenderContext& context) {
    SDL_Renderer* r = context.renderer;
    const auto& data = services_.data;
    const float w = services_.width;
    const SDL_FRect all{0, 0, w, services_.height};
    SDL_RenderTexture(r, data.backdrop().handle.get(), nullptr, &all);
    data.large().draw(r, "SELECT STAGE", w / 2, 14, ui::kWhite, Align::Centre);

    const auto& stage = data.stages()[stage_];
    const SDL_FRect preview{std::floor((w - 320) / 2), 50, 320, 180}; // the venue at half size
    ui::frame(r, preview);
    SDL_RenderTexture(r, stage.image->handle.get(), nullptr, &preview);

    // Arrows either side; the one just pressed lights up.
    const bool lit = clock_ - slideTicks_ < 8;
    const bool pulse = (clock_ / 20) % 2 == 0;
    data.large().draw(r, "<", preview.x - 26, preview.y + 78, lit && slide_ < 0 ? ui::kWhite : pulse ? ui::kBrassLight : ui::kBrass);
    data.large().draw(r, ">", preview.x + preview.w + 8, preview.y + 78,
                      lit && slide_ > 0 ? ui::kWhite : pulse ? ui::kBrassLight : ui::kBrass);

    // Which venue of how many.
    const auto count = data.stages().size();
    const float dotsX = w / 2 - static_cast<float>(count) * 5 + 1;
    for (std::size_t i = 0; i < count; ++i)
        ui::fill(r, dotsX + static_cast<float>(i) * 10, preview.y + preview.h + 8, 6, 6, i == stage_ ? ui::kBrassLight : ui::kSteel);

    data.large().draw(r, ui::upper(stage.name), w / 2, 252, ui::kWhite, Align::Centre);
    if (!stage.musicTitle.empty())
        data.small().draw(r, "MUSIC - " + ui::upper(stage.musicTitle), w / 2, 280, ui::kTealLight, Align::Centre, true);
    float y = 296;
    for (const auto& line : ui::wrap(data.small(), ui::upper(stage.description), 400)) {
        data.small().draw(r, line, w / 2, y, ui::kFog, Align::Centre, true);
        y += 10;
    }
    data.small().draw(r, "PUNCH TO FIGHT - KICK TO GO BACK", w / 2, 346, ui::kFog, Align::Centre, true);
}

std::string StageSelectScene::status() const { return "stage select: " + services_.data.stages()[stage_].id; }

} // namespace iron
