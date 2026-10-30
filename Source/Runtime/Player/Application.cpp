#include "Player/Application.h"

#include "Assets/AssetLibrary.h"
#include "Audio/AudioSystem.h"
#include "Game/CharacterSelectScene.h"
#include "Game/Controls.h"
#include "Game/FightScene.h"
#include "Game/GameData.h"
#include "Game/OptionsScene.h"
#include "Game/Services.h"
#include "Game/StageSelectScene.h"
#include "Game/TitleScene.h"
#include "Vfs/DirectoryMount.h"
#include "Vfs/ZipMount.h"

#include <algorithm>
#include <array>
#include <cstdio>
#include <stdexcept>

namespace iron {
namespace {

constexpr Uint64 kTickNs = SDL_NS_PER_SECOND / 60;
constexpr Uint64 kMaxFrameNs = SDL_NS_PER_SECOND / 4; // avoid a spiral of death after a stall

bool isArchiveName(const std::filesystem::path& p) {
    const auto ext = p.extension();
    return ext == ".assets" || ext == ".resource" || p.filename() == "globalgamemanagers";
}

} // namespace

Application::Application(const PlayerSettings& settings, PlayerPrefs prefs, const ProductInfo& info)
    : settings_(settings), prefs_(std::move(prefs)), screenshotFolder_(info.userFolder) {
    if (settings_.batchMode) {
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
        SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
    }
    // The id becomes the Wayland app_id / X11 class, matching the installed .desktop file.
    SDL_SetAppMetadata(info.product.c_str(), IR_VERSION_STRING, IR_APP_ID);
    sdlCheck(SDL_Init(SDL_INIT_VIDEO), "SDL_Init");

    mountData();
    const auto gameSettings = nlohmann::json::parse(vfs_.readText("ProjectSettings/GameSettings.json"));
    const auto logical = gameSettings.value("logicalResolution", std::array<int, 2>{640, 360});
    display_ = std::make_unique<Display>(settings_, info.product.c_str(), logical[0], logical[1]);
    SDL_Log("Display: %s", display_->describe().c_str());
    setWindowIcon();

    audio_ = std::make_unique<AudioSystem>(settings_.musicVolume, settings_.sfxVolume, settings_.voiceVolume);

    // "pixelart" keeps the 640x360 art crisp at any integer or letterbox scale.
    const auto filter = parseScaleMode(gameSettings.value("textureFilter", "linear"));
    assets_ = std::make_unique<AssetLibrary>(vfs_, display_->renderer(), filter);
    controls_ = std::make_unique<Controls>(assets_->json("ProjectSettings/InputManager.json"));
    controls_->loadBindings(prefs_); // remapped keys and per-pad layouts
    data_ = std::make_unique<GameData>(*assets_, *audio_);
    logicalWidth_ = logical[0];
    logicalHeight_ = logical[1];
    services_ = std::make_unique<Services>(Services{*assets_, *audio_, *display_, *controls_, settings_, prefs_, *data_,
                                                    scenes_, static_cast<float>(logical[0]),
                                                    static_cast<float>(logical[1]), [this] { resetRenderer(); }});
    openFirstScene();

    // "30:Return,45:Down": a one-frame tap at that frame; "30-90:D": held from
    // frame 30 to 90; "40:S+D": keys together.
    std::string_view script = settings_.keys;
    while (!script.empty()) {
        const auto comma = script.find(',');
        const auto item = script.substr(0, comma);
        script = comma == std::string_view::npos ? std::string_view{} : script.substr(comma + 1);
        const auto colon = item.find(':');
        const std::string frames(item.substr(0, colon));
        std::string_view keys = colon == std::string_view::npos ? std::string_view{} : item.substr(colon + 1);
        int first = -1, last = -1;
        try {
            const auto dash = frames.find('-');
            first = std::stoi(frames.substr(0, dash));
            last = dash == std::string::npos ? first : std::stoi(frames.substr(dash + 1));
        } catch (...) {
            first = -1;
        }
        bool ok = first >= 0 && last >= first && !keys.empty();
        std::vector<SDL_Scancode> codes;
        while (ok && !keys.empty()) {
            const auto plus = keys.find('+', 1); // "Keypad +" style names aren't needed here
            const std::string name(keys.substr(0, plus));
            keys = plus == std::string_view::npos ? std::string_view{} : keys.substr(plus + 1);
            const SDL_Scancode code = SDL_GetScancodeFromName(name.c_str());
            ok = code != SDL_SCANCODE_UNKNOWN;
            codes.push_back(code);
        }
        if (!ok) SDL_Log("-keys: ignored '%.*s'", static_cast<int>(item.size()), item.data());
        else
            for (const auto code : codes) keyScript_.push_back({first, last, code});
    }
}

void Application::playKeyScript(int frame) {
    for (const auto& k : keyScript_) {
        if (k.first != frame && k.last + 1 != frame) continue;
        SDL_Event e{};
        e.type = k.first == frame ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
        e.key.scancode = k.key;
        e.key.down = k.first == frame;
        e.key.windowID = SDL_GetWindowID(display_->window());
        SDL_PushEvent(&e);
    }
}

void Application::resetRenderer() {
    assets_->releaseTextures(); // they belong to the old renderer
    display_->recreateRenderer(settings_);
    assets_->recreateTextures(display_->renderer());
}

void Application::openFirstScene() {
    // The title is always at the bottom; -demo, -training and -stage go straight into a fight above it.
    scenes_.push(std::make_unique<TitleScene>(*services_), Transition::Cut);
    scenes_.flush(); // applies the push, so the next one lands on top
    if (settings_.demo || settings_.training || !settings_.stage.empty()) {
        FightSetup setup;
        setup.control = settings_.demo ? Control::Demo : Control::VsCpu;
        setup.training = settings_.training;
        setup.stage = settings_.stage;
        if (!settings_.dummy.empty()) {
            if (const auto dummy = sim::parseDummy(settings_.dummy)) setup.dummy = *dummy;
            else SDL_Log("Unknown -dummy '%s'; using STAND", settings_.dummy.c_str());
        }
        scenes_.push(std::make_unique<FightScene>(*services_, std::move(setup)), Transition::Cut);
        return;
    }
    const auto& scene = settings_.scene;
    if (scene.empty() || scene == "title") return;
    if (scene == "select") {
        scenes_.push(std::make_unique<CharacterSelectScene>(*services_, SelectMode::VsCpu), Transition::Cut);
    } else if (scene == "stage") {
        FightSetup setup;
        setup.fromSelect = true;
        scenes_.push(std::make_unique<StageSelectScene>(*services_, std::move(setup)), Transition::Cut);
    } else if (scene == "options") {
        scenes_.push(std::make_unique<OptionsScene>(*services_), Transition::Cut);
    } else {
        SDL_Log("Unknown -scene '%s' (title, select, stage, options)", scene.c_str());
    }
}

Application::~Application() {
    scenes_.popTo(0, Transition::Cut);
    scenes_.flush();
    services_.reset();
    data_.reset();
    controls_.reset();
    assets_.reset();
    audio_.reset();
    display_.reset();
    SDL_Quit();
}

void Application::setWindowIcon() {
    // Like a Unity player: <name>_Data/Resources/<engine>.png. Optional.
    const auto path = settings_.dataFolder / "Resources" / (std::string(IR_PLAYER_LIBRARY_NAME) + ".png");
    const auto u8 = path.u8string();
    SurfaceHandle icon(SDL_LoadPNG(reinterpret_cast<const char*>(u8.c_str())));
    if (icon) SDL_SetWindowIcon(display_->window(), icon.get());
}

void Application::mountData() {
    const auto& dataDir = settings_.dataFolder;
    if (!std::filesystem::is_directory(dataDir))
        throw std::runtime_error("Data folder not found: " + dataDir.string());

    std::vector<std::filesystem::path> archives;
    for (const auto& entry : std::filesystem::directory_iterator(dataDir))
        if (entry.is_regular_file() && isArchiveName(entry.path())) archives.push_back(entry.path());
    std::sort(archives.begin(), archives.end());

    for (const auto& archive : archives) vfs_.mount(std::make_unique<vfs::ZipMount>(archive));
    if (!settings_.assetsFolder.empty()) vfs_.mount(std::make_unique<vfs::DirectoryMount>(settings_.assetsFolder));
    for (const auto& m : vfs_.mounts()) SDL_Log("Mounted %s", m->describe().c_str());
}

int Application::run() {
    Uint64 previous = SDL_GetTicksNS();
    Uint64 accumulator = 0;
    Uint64 fpsWindowStart = previous;
    int fpsFrames = 0;
    int frame = 0;

    while (running_) {
        playKeyScript(frame);
        SDL_Event event;
        while (SDL_PollEvent(&event)) handleEvent(event);

        const Uint64 now = SDL_GetTicksNS();
        accumulator += std::min(now - previous, kMaxFrameNs);
        previous = now;
        if (settings_.batchMode) accumulator = kTickNs; // deterministic: one tick per frame

        while (accumulator >= kTickNs) {
            scenes_.fixedUpdate();
            controls_->endTick();
            accumulator -= kTickNs;
        }
        if (scenes_.quitting()) running_ = false;

        ++frame;
        const bool lastBatchFrame = settings_.batchMode && frame >= settings_.batchFrames;
        std::filesystem::path capture;
        if (lastBatchFrame) capture = settings_.screenshotPath;
        if (pendingCapture_) capture = screenshotFolder_ / ("Screenshot-" + std::to_string(SDL_GetTicks()) + ".png");
        pendingCapture_ = false;
        render(capture);
        if (++fpsFrames == 30) {
            fps_ = 30.0 * SDL_NS_PER_SECOND / static_cast<double>(std::max<Uint64>(now - fpsWindowStart, 1));
            fpsWindowStart = now;
            fpsFrames = 0;
        }
        if (lastBatchFrame) running_ = false;
    }
    SDL_Log("Player exiting after %d frames", frame);
    return 0;
}

void Application::handleEvent(const SDL_Event& event) {
    switch (event.type) {
    case SDL_EVENT_QUIT:
        running_ = false;
        return;
    case SDL_EVENT_WINDOW_HDR_STATE_CHANGED:
    case SDL_EVENT_WINDOW_DISPLAY_CHANGED:
        // HDR switched on in the system, or the window moved to an HDR display.
        if (display_->onHdrStateChanged(settings_.hdr)) resetRenderer();
        return;
    case SDL_EVENT_KEY_DOWN:
        if (event.key.repeat) break;
        if (event.key.scancode == SDL_SCANCODE_F1) overlay_ = !overlay_;
        if (event.key.scancode == SDL_SCANCODE_F12) pendingCapture_ = true;
        if (event.key.scancode == SDL_SCANCODE_F11 ||
            (event.key.scancode == SDL_SCANCODE_RETURN && (event.key.mod & SDL_KMOD_ALT)))
            display_->toggleFullscreen();
        break;
    default:
        break;
    }
    scenes_.handleEvent(event);
    if (const auto menu = controls_->onEvent(event)) {
        const unsigned before = scenes_.generation();
        scenes_.handleMenu(*menu);
        // A press that changed the screen must not also act on the new one (a jab on resume).
        if (scenes_.generation() != before) controls_->endTick();
    }
}

void Application::render(const std::filesystem::path& capturePath) {
    SDL_Renderer* r = display_->renderer();
    scenes_.render({r, display_->highlightBoost()}, static_cast<float>(logicalWidth_),
                   static_cast<float>(logicalHeight_));
    if (overlay_) drawOverlay();
    if (!capturePath.empty()) saveScreenshot(capturePath); // read back before present
    SDL_RenderPresent(r);
}

void Application::drawOverlay() {
    SDL_Renderer* r = display_->renderer();
    char fps[32];
    std::snprintf(fps, sizeof fps, "%.0f fps", fps_);
    const std::string lines[] = {
        std::string("Iron Ring ") + IR_VERSION_STRING + " | " + fps,
        display_->describe(),
        scenes_.status(),
        controls_->describe(),
    };
    float y = 60;
    for (const auto& line : lines) {
        SDL_SetRenderDrawColor(r, 9, 22, 39, 200);
        const SDL_FRect bg{8, y - 2, 8.0f * static_cast<float>(line.size()) + 4, 12};
        SDL_RenderFillRect(r, &bg);
        SDL_SetRenderDrawColor(r, 247, 228, 181, 255);
        SDL_RenderDebugText(r, 10, y, line.c_str());
        y += 12;
    }
}

void Application::saveScreenshot(const std::filesystem::path& path) const {
    SurfaceHandle pixels(SDL_RenderReadPixels(display_->renderer(), nullptr));
    sdlCheck(pixels != nullptr, "SDL_RenderReadPixels");
    SurfaceHandle rgba(SDL_ConvertSurface(pixels.get(), SDL_PIXELFORMAT_RGBA32));
    sdlCheck(rgba != nullptr, "SDL_ConvertSurface");
    const auto u8 = path.u8string();
    sdlCheck(SDL_SavePNG(rgba.get(), reinterpret_cast<const char*>(u8.c_str())), "SDL_SavePNG");
    SDL_Log("Screenshot saved: %s", reinterpret_cast<const char*>(u8.c_str()));
}

} // namespace iron
