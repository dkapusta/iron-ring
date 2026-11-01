#pragma once

// The online front end (docs/design/online.md §2): ONLINE (name, server,
// invites), the server lobby, and the room. The character select, stage
// select and fight have online modes of their own.

#include "Core/Menu.h"
#include "Core/TextInput.h"
#include "Game/Scene.h"
#include "Game/Services.h"

#include <optional>
#include <string>
#include <vector>

namespace iron {

// Edits one TextInput in place: typing, Backspace, Ctrl+V and Enter/Escape on
// a keyboard; Up/Down/Left/Right/Confirm/Back on a pad. While editing, the
// menu action a typed key would also produce (W is Up for player 1) is
// swallowed.
class FieldEditor {
  public:
    enum class Result { None, Done, Cancelled };

    void start(TextInput& field, SDL_Window* window);
    [[nodiscard]] bool editing() const { return field_ != nullptr; }
    [[nodiscard]] const TextInput* field() const { return field_; }
    Result onEvent(const SDL_Event& event);
    Result onMenu(const MenuEvent& event);

  private:
    void stop();
    TextInput* field_ = nullptr;
    SDL_Window* window_ = nullptr;
    bool swallow_ = false;
};

// Puts a fight on the stack if the server started one (from the room or the
// select). True if it did.
bool followMatchStart(Services& services, bool replaceTop);

class OnlineScene final : public Scene {
  public:
    explicit OnlineScene(Services& services);

    void handleEvent(const SDL_Event& event) override;
    void handleMenu(const MenuEvent& event) override;
    void fixedUpdate() override;
    void render(const RenderContext& context) override;
    [[nodiscard]] std::string status() const override;
    void onEnter() override;

  private:
    void connect(const std::string& address);
    void paste();
    void finishEdit(FieldEditor::Result result);
    void refreshLabels();

    Services& services_;
    Menu menu_;
    FieldEditor editor_;
    TextInput name_, server_;
    std::vector<std::string> history_;
    int historyIndex_ = -1;
    std::string message_;
    bool lobbyShown_ = false;
    bool autoConnect_ = false;
    int blink_ = 0;
};

class LobbyScene final : public Scene {
  public:
    explicit LobbyScene(Services& services);

    void handleEvent(const SDL_Event& event) override;
    void handleMenu(const MenuEvent& event) override;
    void fixedUpdate() override;
    void render(const RenderContext& context) override;
    [[nodiscard]] std::string status() const override;
    void onEnter() override;

  private:
    void refreshLabels();
    void showRooms();

    Services& services_;
    Menu menu_;
    Menu rooms_; // OPEN ROOMS
    bool listing_ = false;
    FieldEditor editor_;
    TextInput code_;
    std::string message_;
    bool roomShown_ = false;
    int autoplayStep_ = 0;
    int blink_ = 0;
};

class RoomScene final : public Scene {
  public:
    explicit RoomScene(Services& services);

    void handleMenu(const MenuEvent& event) override;
    void fixedUpdate() override;
    void render(const RenderContext& context) override;
    [[nodiscard]] std::string status() const override;
    void onEnter() override;

  private:
    void refreshLabels();

    Services& services_;
    Menu menu_;
    std::string toast_;
    int toastTicks_ = 0;
    bool selectShown_ = false;
    int blink_ = 0;
};

} // namespace iron
