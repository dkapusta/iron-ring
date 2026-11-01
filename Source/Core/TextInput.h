#pragma once

// The editable text of a menu field (player name, server address, room code):
// typing and pasting with a keyboard, and arcade-style entry on a pad (Up/Down
// change the character under the cursor, Right adds one, Left removes one).
// Only characters in the field's charset get in, raised to the font's
// capitals; the length is capped. No SDL: the scenes feed it.

#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace iron {

class TextInput {
  public:
    TextInput(std::string charset, std::size_t maxLength, std::string text = {});

    [[nodiscard]] const std::string& text() const { return text_; }
    void set(std::string_view text); // filtered like typing
    [[nodiscard]] bool empty() const { return text_.empty(); }

    // Keyboard.
    void type(std::string_view utf8); // what SDL_EVENT_TEXT_INPUT delivers; anything outside the charset is dropped
    void backspace();
    void paste(std::string_view text) { type(text); }

    // Pad: change the last character through the charset (adds one if empty), add one, remove one.
    void cycle(int delta);
    void addCharacter();
    void removeCharacter() { backspace(); }

    // Editing with an undo point: begin() remembers, cancel() restores.
    void begin() { saved_ = text_; }
    void cancel() { text_ = saved_; }

    [[nodiscard]] const std::string& charset() const { return charset_; }

  private:
    [[nodiscard]] char accept(char c) const; // the character as stored, or 0
    std::string charset_;
    std::size_t max_;
    std::string text_;
    std::string saved_;
};

// Charsets of the online fields.
inline constexpr std::string_view kNameChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.- ";
inline constexpr std::string_view kAddressChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.-:/";

// A most-recent-first list of distinct entries, at most `limit` (server history).
void remember(std::vector<std::string>& history, const std::string& entry, std::size_t limit = 8);
// Space-separated in prefs.cfg (addresses have no spaces).
[[nodiscard]] std::vector<std::string> splitHistory(std::string_view text);
[[nodiscard]] std::string joinHistory(const std::vector<std::string>& history);

} // namespace iron
