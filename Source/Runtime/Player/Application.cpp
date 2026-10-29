#include "Player/Application.h"

#include "Assets/AssetLibrary.h"
#include "Audio/AudioSystem.h"
#include "Game/FightScene.h"
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

Application::Application(const PlayerSettings& settings, const ProductInfo& info)
    : settings_(settings), screenshotFolder_(info.userFolder) {
    if (settings_.batchMode) {
        SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "offscreen");
        SDL_SetHint(SDL_HINT_AUDIO_DRIVER, "dummy");
    }
    SDL_SetAppMetadata(info.product.c_str(), IR_VERSION_STRING, nullptr);
    sdlCheck(SDL_Init(SDL_INIT_VIDEO), "SDL_Init");

    mountData();
    const auto gameSettings = nlohmann::json::parse(vfs_.readText("ProjectSettings/GameSettings.json"));
    const auto logical = gameSettings.value("logicalResolution", std::array<int, 2>{640, 360});
    display_ = std::make_unique<Display>(settings_, info.product.c_str(), logical[0], logical[1]);
    SDL_Log("Display: %s", display_->describe().c_str());
    setWindowIcon();

    audio_ = std::make_unique<AudioSystem>();

    // "pixelart" keeps the 640x360 art crisp at any integer or letterbox scale.
    const auto filter = parseScaleMode(gameSettings.value("textureFilter", "linear"));
    assets_ = std::make_unique<AssetLibrary>(vfs_, display_->renderer(), filter);
    FightOptions options;
    options.control = settings_.demo ? Control::Demo : Control::VsCpu;
    options.training = settings_.training;
    options.stage = settings_.stage;
    scene_ = std::make_unique<FightScene>(*assets_, *audio_, options);
}

Application::~Application() {
    scene_.reset();
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
        SDL_Event event;
        while (SDL_PollEvent(&event)) handleEvent(event);

        const Uint64 now = SDL_GetTicksNS();
        accumulator += std::min(now - previous, kMaxFrameNs);
        previous = now;
        if (settings_.batchMode) accumulator = kTickNs; // deterministic: one tick per frame

        while (accumulator >= kTickNs) {
            scene_->fixedUpdate();
            accumulator -= kTickNs;
        }

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
        display_->onHdrStateChanged();
        return;
    case SDL_EVENT_KEY_DOWN:
        if (event.key.repeat) break;
        if (event.key.scancode == SDL_SCANCODE_ESCAPE) running_ = false;
        if (event.key.scancode == SDL_SCANCODE_F1) overlay_ = !overlay_;
        if (event.key.scancode == SDL_SCANCODE_F12) pendingCapture_ = true;
        if (event.key.scancode == SDL_SCANCODE_F11 ||
            (event.key.scancode == SDL_SCANCODE_RETURN && (event.key.mod & SDL_KMOD_ALT)))
            display_->toggleFullscreen();
        break;
    default:
        break;
    }
    scene_->handleEvent(event);
}

void Application::render(const std::filesystem::path& capturePath) {
    SDL_Renderer* r = display_->renderer();
    scene_->render({r, display_->highlightBoost()});
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
        scene_->status(),
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
