#include "Game/OnlineScenes.h"

#include "Game/CharacterSelectScene.h"
#include "Game/FightScene.h"
#include "Game/MenuView.h"
#include "Game/OnlineSession.h"
#include "Net/Codes.h"
#include "Net/Net.h"
#include "Render/Ui.h"

#include <algorithm>
#include <cmath>

namespace iron {
namespace {

constexpr std::size_t kAddressLength = 60;

void backdrop(Services& services, SDL_Renderer* r, const char* title) {
    const SDL_FRect all{0, 0, services.width, services.height};
    SDL_RenderTexture(r, services.data.backdrop().handle.get(), nullptr, &all);
    services.data.large().draw(r, title, services.width / 2, 22, ui::kWhite, Align::Centre);
}

SDL_FRect panel(SDL_Renderer* r, float y, float height) {
    const SDL_FRect box{150, y, 340, height};
    ui::frame(r, box);
    ui::fill(r, box.x, box.y, box.w, box.h, ui::kNight);
    return box;
}

std::string clipboard() {
    std::string text;
    if (char* c = SDL_GetClipboardText()) {
        text = c;
        SDL_free(c);
    }
    return text;
}

std::string withCursor(const std::string& text, bool editing, int blink) {
    return text + (editing && (blink / 15) % 2 == 0 ? "_" : editing ? " " : "");
}

} // namespace

// ---- FieldEditor ----------------------------------------------------------------------------

void FieldEditor::start(TextInput& field, SDL_Window* window) {
    field_ = &field;
    window_ = window;
    field.begin();
    keyEcho_ = false; // the press that started editing has done its job
    if (window_) SDL_StartTextInput(window_);
}

void FieldEditor::stop() {
    if (window_) SDL_StopTextInput(window_);
    field_ = nullptr;
}

bool FieldEditor::swallow() {
    const bool echo = keyEcho_;
    keyEcho_ = false;
    return echo;
}

FieldEditor::Result FieldEditor::onEvent(const SDL_Event& event) {
    // Every SDL event comes here before its menu action (Application::handleEvent),
    // so this marks whether the next menu action belongs to a key the editor took.
    if (event.type != SDL_EVENT_TEXT_INPUT) keyEcho_ = false;
    if (!field_) return Result::None;
    if (event.type == SDL_EVENT_TEXT_INPUT) {
        field_->type(event.text.text);
        return Result::None;
    }
    if (event.type != SDL_EVENT_KEY_DOWN) return Result::None;
    keyEcho_ = true; // a keyboard key: its menu action (if any) is not for the menu
    const auto key = event.key.scancode;
    if (key == SDL_SCANCODE_BACKSPACE)
        field_->backspace();
    else if (key == SDL_SCANCODE_V && (event.key.mod & SDL_KMOD_CTRL))
        field_->paste(clipboard());
    else if (key == SDL_SCANCODE_RETURN || key == SDL_SCANCODE_KP_ENTER) {
        if (event.key.repeat) return Result::None;
        stop();
        return Result::Done;
    } else if (key == SDL_SCANCODE_ESCAPE) {
        field_->cancel();
        stop();
        return Result::Cancelled;
    }
    return Result::None;
}

FieldEditor::Result FieldEditor::onMenu(const MenuEvent& event) {
    if (!field_) return Result::None;
    // A pad: arcade name entry.
    switch (event.action) {
    case MenuAction::Up: field_->cycle(1); break;
    case MenuAction::Down: field_->cycle(-1); break;
    case MenuAction::Right: field_->addCharacter(); break;
    case MenuAction::Left: field_->removeCharacter(); break;
    case MenuAction::Info: field_->paste(clipboard()); break;
    case MenuAction::Confirm: stop(); return Result::Done;
    case MenuAction::Back:
    case MenuAction::Pause:
        field_->cancel();
        stop();
        return Result::Cancelled;
    }
    return Result::None;
}

bool followMatchStart(Services& services, bool replaceTop) {
    auto& online = *services.online;
    auto start = online.takeMatchStart();
    if (!start) return false;
    FightSetup setup;
    setup.control = Control::Online;
    setup.fighters = start->fighters;
    setup.stage = start->stage;
    setup.online = std::move(*start);
    auto fight = std::make_unique<FightScene>(services, std::move(setup));
    if (replaceTop)
        services.scenes.replaceTop(std::move(fight));
    else
        services.scenes.push(std::move(fight));
    return true;
}

// ---- ONLINE -----------------------------------------------------------------------------------

OnlineScene::OnlineScene(Services& services)
    : services_(services), name_(std::string(kNameChars), net::kMaxNameLength),
      server_(std::string(kAddressChars), kAddressLength), invite_(std::string(kAddressChars), kAddressLength) {
    auto& prefs = services_.prefs;
    name_.set(services_.settings.onlineName.empty() ? prefs.get("online-name").value_or("")
                                                    : services_.settings.onlineName);
    history_ = splitHistory(prefs.get("online-servers").value_or(""));
    const auto fallback = history_.empty() ? services_.data.defaultServer() : history_.front();
    server_.set(services_.settings.connect.empty() ? fallback : services_.settings.connect);
    autoConnect_ = !services_.settings.connect.empty();
    menu_.add(MenuItem::action("name", ""));
    menu_.add(MenuItem::action("server", ""));
    menu_.add(MenuItem::action("connect", "CONNECT"));
    menu_.add(MenuItem::action("invite", ""));
    menu_.add(MenuItem::action("paste", "PASTE INVITE"));
    menu_.add(MenuItem::action("back", "BACK"));
    menu_.focusOn(name_.empty() ? "name" : "connect");
    refreshLabels();
}

void OnlineScene::refreshLabels() {
    const bool nameEdit = editor_.field() == &name_, serverEdit = editor_.field() == &server_;
    auto* name = menu_.find("name");
    name->label = "NAME    " + withCursor(name_.empty() && !nameEdit ? "-" : name_.text(), nameEdit, blink_);
    name->note = nameEdit ? "TYPE A NAME - ENTER: DONE" : "UP TO 12 LETTERS AND DIGITS";
    auto* server = menu_.find("server");
    server->label = "SERVER  " + withCursor(server_.empty() && !serverEdit ? "-" : server_.text(), serverEdit, blink_);
    server->note = serverEdit            ? "ADDRESS:PORT, OR A WHOLE INVITE - CTRL+V PASTES"
                   : history_.size() > 1 ? "LEFT/RIGHT: RECENT SERVERS"
                                         : "CONFIRM TO TYPE AN ADDRESS";
    const bool inviteEdit = editor_.field() == &invite_;
    auto* invite = menu_.find("invite");
    invite->label = "INVITE  " + withCursor(invite_.empty() && !inviteEdit ? "-" : invite_.text(), inviteEdit, blink_);
    invite->note = inviteEdit ? "SERVER:PORT/CODE, OR JUST THE CODE - ENTER: JOIN"
                              : "TYPE A FRIEND'S INVITE TO CONNECT AND JOIN";
    menu_.find("paste")->note = "THE INVITE FROM THE CLIPBOARD: CONNECT AND JOIN";
}

void OnlineScene::onEnter() {
    services_.data.playTheme(services_.audio, services_.data.menuTheme());
    if (lobbyShown_) {
        // Back from the lobby: the connection ended, or the player left.
        lobbyShown_ = false;
        message_ = services_.online->takeMessage();
        services_.online->disconnect();
    }
}

void OnlineScene::connect(const std::string& address) {
    const auto name = net::sanitizeName(name_.text());
    if (name.empty()) {
        message_ = "ENTER A NAME FIRST";
        menu_.focusOn("name");
        return;
    }
    const auto invite = net::parseInvite(address);
    if (!invite) {
        message_ = "NOT A SERVER ADDRESS";
        return;
    }
    server_.set(net::formatInvite({invite->host, invite->port, {}}));
    remember(history_, server_.text());
    historyIndex_ = -1;
    auto& prefs = services_.prefs;
    prefs.set("online-name", name);
    prefs.set("online-servers", joinHistory(history_));
    if (!services_.settings.batchMode) (void)prefs.save();
    message_.clear();
    if (!services_.online->connect(*invite, name)) message_ = services_.online->takeMessage();
}

void OnlineScene::join() {
    std::string text = invite_.text();
    // A code on its own joins on the server in the SERVER field.
    if (text.size() == net::kCodeLength && text.find_first_of(":/.") == std::string::npos) {
        const auto server = net::parseInvite(server_.text());
        if (!server) {
            message_ = "ENTER THE SERVER FIRST, OR THE WHOLE INVITE";
            return;
        }
        text = net::formatInvite({server->host, server->port, text});
    }
    const auto invite = net::parseInvite(text);
    if (!invite || invite->code.empty()) {
        message_ = invite_.empty() ? "TYPE OR PASTE AN INVITE FIRST" : "NOT AN INVITE (SERVER:PORT/CODE)";
        return;
    }
    invite_.set(text);
    connect(text);
}

void OnlineScene::paste() {
    const auto text = clipboard();
    const auto invite = net::parseInvite(text);
    if (!invite || invite->code.empty()) {
        message_ = text.empty() ? "THE CLIPBOARD IS EMPTY" : "NO INVITE IN THE CLIPBOARD";
        return;
    }
    invite_.set(text); // shown in the INVITE field, to edit if it needs it
    join();
}

void OnlineScene::finishEdit(FieldEditor::Result result) {
    if (result == FieldEditor::Result::None) return;
    services_.audio.play(result == FieldEditor::Result::Done ? "menu_confirm" : "menu_back");
    if (result != FieldEditor::Result::Done) return;
    // An invite typed into the server field connects and joins at once, as the INVITE field does.
    if (menu_.focused().id == "server") {
        const auto invite = net::parseInvite(server_.text());
        if (invite && !invite->code.empty()) connect(server_.text());
    } else if (menu_.focused().id == "invite" && !invite_.empty()) {
        join();
    }
}

void OnlineScene::handleEvent(const SDL_Event& event) {
    finishEdit(editor_.onEvent(event));
}

void OnlineScene::handleMenu(const MenuEvent& event) {
    if (editor_.swallow()) return;
    if (editor_.editing()) return finishEdit(editor_.onMenu(event));
    if (services_.online->status() == OnlineSession::Status::Connecting) {
        if (event.action == MenuAction::Back || event.action == MenuAction::Pause) {
            services_.online->disconnect();
            message_ = "CANCELLED";
        }
        return;
    }
    // The server field: Left/Right step through the recent servers.
    if (menu_.focused().id == "server" && !history_.empty() &&
        (event.action == MenuAction::Left || event.action == MenuAction::Right)) {
        const int n = static_cast<int>(history_.size());
        historyIndex_ = ((historyIndex_ < 0 ? 0 : historyIndex_) + (event.action == MenuAction::Right ? 1 : n - 1)) % n;
        server_.set(history_[static_cast<std::size_t>(historyIndex_)]);
        services_.audio.play("menu_move");
        return;
    }
    switch (navigate(menu_, event, services_.audio)) {
    case MenuResult::Chosen: {
        const auto& id = menu_.focused().id;
        if (id == "name")
            editor_.start(name_, services_.display.window());
        else if (id == "server")
            editor_.start(server_, services_.display.window());
        else if (id == "invite")
            editor_.start(invite_, services_.display.window());
        else if (id == "paste")
            paste();
        else if (id == "connect")
            connect(server_.text());
        else if (id == "back")
            services_.scenes.pop();
        break;
    }
    case MenuResult::Back: services_.scenes.pop(); break;
    default: break;
    }
}

void OnlineScene::fixedUpdate() {
    ++blink_;
    if (autoConnect_) {
        autoConnect_ = false;
        connect(server_.text());
    }
    auto& online = *services_.online;
    if (online.online() && !lobbyShown_) {
        lobbyShown_ = true;
        services_.scenes.push(std::make_unique<LobbyScene>(services_));
    }
    if (online.status() == OnlineSession::Status::Offline)
        if (auto m = online.takeMessage(); !m.empty()) message_ = m;
    refreshLabels();
}

void OnlineScene::render(const RenderContext& context) {
    SDL_Renderer* r = context.renderer;
    backdrop(services_, r, "ONLINE");
    const auto box = panel(r, 80, 112);
    MenuStyle style;
    style.y = box.y + 14;
    style.width = 300;
    style.spacing = 16;
    style.noteGap = 14;
    drawMenu(r, menu_, services_.data.small(), style, blink_);
    const auto& small = services_.data.small();
    const bool connecting = services_.online->status() == OnlineSession::Status::Connecting;
    const std::string line = connecting ? "CONNECTING TO " + server_.text() + "..." : message_;
    if (!line.empty())
        small.draw(r, line, 320, 236, connecting ? ui::kBrassLight : ui::kCoralLight, Align::Centre, true);
    small.draw(r, "PLAY A FRIEND ON ANY IRON RING SERVER. ONE OF YOU CREATES A ROOM", 320, 300, ui::kFog, Align::Centre,
               true);
    small.draw(r, "AND SENDS THE INVITE. THE OTHER TYPES OR PASTES IT HERE.", 320, 310, ui::kFog, Align::Centre, true);
}

std::string OnlineScene::status() const {
    return "online: " + menu_.focused().id + (editor_.editing() ? " (editing)" : "");
}

// ---- LOBBY ------------------------------------------------------------------------------------

LobbyScene::LobbyScene(Services& services)
    : services_(services), code_(std::string(net::kCodeAlphabet), net::kCodeLength) {
    menu_.add(MenuItem::action("create", "CREATE ROOM"));
    menu_.add(MenuItem::action("code", ""));
    menu_.add(MenuItem::action("open", "OPEN ROOMS"));
    menu_.add(MenuItem::action("back", "LEAVE SERVER"));
    menu_.find("create")->note = "A PRIVATE ROOM: SEND THE INVITE TO A FRIEND";
    menu_.find("open")->note = "ROOMS WAITING FOR ANY CHALLENGER";
    refreshLabels();
}

void LobbyScene::refreshLabels() {
    const bool editing = editor_.editing();
    auto* code = menu_.find("code");
    code->label = "JOIN WITH CODE  " + withCursor(code_.empty() && !editing ? "------" : code_.text(), editing, blink_);
    code->note = editing ? "SIX CHARACTERS - ENTER: JOIN" : "A FRIEND'S ROOM CODE";
}

void LobbyScene::onEnter() {
    if (roomShown_) {
        roomShown_ = false;
        if (auto m = services_.online->takeMessage(); !m.empty()) message_ = m;
    }
}

void LobbyScene::showRooms() {
    rooms_ = Menu{};
    const auto& list = services_.online->roomList();
    if (list)
        for (const auto& room : list->rooms)
            rooms_.add(MenuItem::action("room:" + room.code, ui::upper(room.host) + "  (" +
                                                                 std::to_string(room.waitingSeconds / 60) + " MIN)"));
    rooms_.add(MenuItem::action("refresh", "REFRESH"));
    rooms_.add(MenuItem::action("back", "BACK"));
    listing_ = true;
}

void LobbyScene::handleEvent(const SDL_Event& event) {
    const auto result = editor_.onEvent(event);
    if (result == FieldEditor::Result::Done && code_.text().size() == net::kCodeLength)
        services_.online->send(net::JoinRoom{code_.text()});
}

void LobbyScene::handleMenu(const MenuEvent& event) {
    auto& online = *services_.online;
    if (editor_.swallow()) return;
    if (editor_.editing()) {
        if (editor_.onMenu(event) == FieldEditor::Result::Done && code_.text().size() == net::kCodeLength)
            online.send(net::JoinRoom{code_.text()});
        return;
    }
    if (listing_) {
        switch (navigate(rooms_, event, services_.audio)) {
        case MenuResult::Chosen: {
            const auto& id = rooms_.focused().id;
            if (id == "refresh") {
                online.clearRoomList();
                online.send(net::ListRooms{});
            } else if (id == "back") {
                listing_ = false;
            } else {
                online.send(net::JoinRoom{id.substr(5)});
            }
            break;
        }
        case MenuResult::Back: listing_ = false; break;
        default: break;
        }
        return;
    }
    switch (navigate(menu_, event, services_.audio)) {
    case MenuResult::Chosen: {
        const auto& id = menu_.focused().id;
        message_.clear();
        if (id == "create")
            online.send(net::CreateRoom{false});
        else if (id == "code")
            editor_.start(code_, services_.display.window());
        else if (id == "open") {
            online.clearRoomList();
            online.send(net::ListRooms{});
            showRooms();
        } else if (id == "back")
            services_.scenes.pop();
        break;
    }
    case MenuResult::Back: services_.scenes.pop(); break;
    default: break;
    }
}

void LobbyScene::fixedUpdate() {
    ++blink_;
    auto& online = *services_.online;
    if (!online.online()) return services_.scenes.pop(); // the ONLINE screen says why
    if (online.inRoom() && !roomShown_) {
        roomShown_ = true;
        listing_ = false;
        services_.scenes.push(std::make_unique<RoomScene>(services_));
        return;
    }
    if (auto m = online.takeMessage(); !m.empty()) message_ = m;
    if (listing_) {
        // Rebuild the list when a fresh one arrives (keeps REFRESH / BACK at the end).
        const auto& list = online.roomList();
        if (list && rooms_.items().size() != list->rooms.size() + 2) showRooms();
    }
    // -autoplay (soak tests): join a waiting open room, or open one.
    if (services_.settings.autoplay) {
        if (autoplayStep_ == 0) {
            online.send(net::ListRooms{});
            autoplayStep_ = 1;
        } else if (autoplayStep_ == 1 && online.roomList()) {
            const auto& rooms = online.roomList()->rooms;
            if (!rooms.empty())
                online.send(net::JoinRoom{rooms.front().code});
            else
                online.send(net::CreateRoom{true});
            autoplayStep_ = 2;
        }
    }
    refreshLabels();
}

void LobbyScene::render(const RenderContext& context) {
    SDL_Renderer* r = context.renderer;
    auto& online = *services_.online;
    const auto& w = online.welcome();
    backdrop(services_, r, "LOBBY");
    const auto& small = services_.data.small();
    // Below the title's rule: the server, its message of the day, and how busy it is.
    small.draw(r, ui::upper(w.serverName), 320, 68, ui::kBrassLight, Align::Centre, true);
    if (!w.motd.empty()) small.draw(r, "\"" + ui::upper(w.motd) + "\"", 320, 78, ui::kChalk, Align::Centre, true);
    const auto count = [](int n, const char* one, const char* many) {
        return std::to_string(n) + " " + (n == 1 ? one : many);
    };
    small.draw(r,
               count(w.players, "PLAYER", "PLAYERS") + " - " + count(w.rooms, "ROOM", "ROOMS") + " - PING " +
                   std::to_string(online.pingMs()),
               320, w.motd.empty() ? 78 : 88, ui::kFog, Align::Centre, true);
    const auto box = panel(r, 104, listing_ ? 150 : 82);
    MenuStyle style;
    style.y = box.y + 14;
    style.width = 300;
    style.spacing = 16;
    style.noteGap = 14;
    if (listing_) {
        small.draw(r, "OPEN ROOMS", 320, box.y + 4, ui::kBrassLight, Align::Centre, true);
        style.y = box.y + 20;
        if (!online.roomList())
            small.draw(r, "ASKING THE SERVER...", 320, box.y + 60, ui::kFog, Align::Centre, true);
        else if (online.roomList()->rooms.empty())
            small.draw(r, "NO OPEN ROOMS RIGHT NOW", 320, box.y + 60, ui::kFog, Align::Centre, true);
        drawMenu(r, rooms_, small, style, blink_);
    } else {
        drawMenu(r, menu_, small, style, blink_);
    }
    if (!message_.empty()) small.draw(r, message_, 320, 262, ui::kCoralLight, Align::Centre, true);
}

std::string LobbyScene::status() const {
    return "lobby: " + (listing_ ? std::string("open rooms") : menu_.focused().id);
}

// ---- ROOM -------------------------------------------------------------------------------------

RoomScene::RoomScene(Services& services) : services_(services) {
    menu_.add(MenuItem::action("copy", "COPY INVITE"));
    menu_.add(MenuItem::action("openness", ""));
    menu_.add(MenuItem::action("leave", "LEAVE ROOM"));
    refreshLabels();
}

void RoomScene::refreshLabels() {
    const auto& room = services_.online->room();
    auto* openness = menu_.find("openness");
    const bool open = room && room->open;
    openness->label = open ? "MAKE PRIVATE" : "MAKE OPEN";
    openness->enabled = services_.online->hosting();
    openness->note = open ? "LISTED UNDER OPEN ROOMS FOR ANYONE ON THE SERVER" : "ONLY THE CODE GETS PLAYERS IN";
    menu_.find("copy")->note = "PASTE IT TO A FRIEND: ONLINE > PASTE INVITE";
}

void RoomScene::onEnter() {
    selectShown_ = false;
    services_.data.playTheme(services_.audio, services_.data.menuTheme());
}

void RoomScene::handleMenu(const MenuEvent& event) {
    auto& online = *services_.online;
    switch (navigate(menu_, event, services_.audio)) {
    case MenuResult::Chosen: {
        const auto& id = menu_.focused().id;
        if (id == "copy") {
            SDL_SetClipboardText(online.invite().c_str());
            toast_ = "INVITE COPIED: " + online.invite();
            toastTicks_ = 180;
        } else if (id == "openness") {
            online.send(net::SetOpen{!(online.room() && online.room()->open)});
        } else if (id == "leave") {
            online.leaveRoom();
            services_.scenes.pop();
        }
        break;
    }
    case MenuResult::Back:
        online.leaveRoom();
        services_.scenes.pop();
        break;
    default: break;
    }
}

void RoomScene::fixedUpdate() {
    ++blink_;
    if (toastTicks_ > 0) --toastTicks_;
    auto& online = *services_.online;
    if (!online.online() || !online.inRoom()) return services_.scenes.pop();
    if (followMatchStart(services_, false)) return;
    const auto phase = online.room()->phase;
    // Two players: on to the select (also after CHANGE FIGHTERS, or a match left from its menu).
    if (online.hasOpponent() && !selectShown_ && phase != net::RoomPhase::Waiting &&
        phase != net::RoomPhase::Fighting) {
        selectShown_ = true;
        online.wantSelect = false;
        services_.scenes.push(std::make_unique<CharacterSelectScene>(services_, SelectMode::Online));
        return;
    }
    refreshLabels();
}

void RoomScene::render(const RenderContext& context) {
    SDL_Renderer* r = context.renderer;
    auto& online = *services_.online;
    backdrop(services_, r, "ROOM");
    const auto& room = online.room();
    if (!room) return;
    const auto& small = services_.data.small();
    const auto& large = services_.data.large();
    // The code, large enough to read out over voice.
    large.draw(r, room->code, 320, 66, ui::kBrassLight, Align::Centre);
    small.draw(r, room->open ? "OPEN ROOM" : "PRIVATE ROOM", 320, 94, ui::kFog, Align::Centre, true);
    const auto box = panel(r, 108, 46);
    for (std::size_t i = 0; i < 2; ++i) {
        const float y = box.y + 10 + static_cast<float>(i) * 18;
        if (i < room->members.size()) {
            const auto& m = room->members[i];
            small.draw(r, ui::upper(m.name) + (i == room->you ? "  (YOU)" : ""), box.x + 12, y, ui::kSide[i],
                       Align::Left, true);
            small.draw(r, (i == 0 ? "HOST  " : "") + std::string("PING ") + std::to_string(m.ping), box.x + box.w - 12,
                       y, ui::kFog, Align::Right, true);
        } else if ((blink_ / 30) % 2 == 0) {
            small.draw(r, "WAITING FOR A CHALLENGER...", box.x + 12, y, ui::kChalk, Align::Left, true);
        }
    }
    const auto box2 = panel(r, 166, 64);
    MenuStyle style;
    style.y = box2.y + 12;
    style.width = 300;
    style.spacing = 16;
    style.noteGap = 14;
    drawMenu(r, menu_, small, style, blink_);
    small.draw(r, online.invite(), 320, 270, ui::kChalk, Align::Centre, true);
    if (toastTicks_ > 0) small.draw(r, toast_, 320, 284, ui::kTealLight, Align::Centre, true);
}

std::string RoomScene::status() const {
    const auto& room = services_.online->room();
    return "room " + (room ? room->code : std::string("?")) + ": " + menu_.focused().id;
}

} // namespace iron
