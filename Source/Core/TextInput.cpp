#include "Core/TextInput.h"

#include <algorithm>

namespace iron {

TextInput::TextInput(std::string charset, std::size_t maxLength, std::string text)
    : charset_(std::move(charset)), max_(maxLength) {
    set(text);
}

char TextInput::accept(char c) const {
    if (c >= 'a' && c <= 'z') c = static_cast<char>(c - 'a' + 'A');
    return charset_.find(c) != std::string::npos ? c : '\0';
}

void TextInput::set(std::string_view text) {
    text_.clear();
    type(text);
}

void TextInput::type(std::string_view utf8) {
    for (const char c : utf8) {
        if (text_.size() >= max_) break;
        if (static_cast<unsigned char>(c) >= 0x80) continue; // no accents in the font
        if (const char k = accept(c)) {
            if (k == ' ' && (text_.empty() || text_.back() == ' ')) continue; // no leading or double spaces
            text_ += k;
        }
    }
}

void TextInput::backspace() {
    if (!text_.empty()) text_.pop_back();
}

void TextInput::cycle(int delta) {
    if (charset_.empty()) return;
    if (text_.empty()) {
        text_ += charset_.front();
        return;
    }
    const auto n = static_cast<int>(charset_.size());
    auto i = static_cast<int>(charset_.find(text_.back()));
    if (i < 0) i = 0;
    text_.back() = charset_[static_cast<std::size_t>(((i + delta) % n + n) % n)];
}

void TextInput::addCharacter() {
    if (text_.size() < max_ && !charset_.empty()) text_ += text_.empty() ? charset_.front() : text_.back();
}

void remember(std::vector<std::string>& history, const std::string& entry, std::size_t limit) {
    if (entry.empty()) return;
    history.erase(std::remove(history.begin(), history.end(), entry), history.end());
    history.insert(history.begin(), entry);
    if (history.size() > limit) history.resize(limit);
}

std::vector<std::string> splitHistory(std::string_view text) {
    std::vector<std::string> out;
    std::size_t i = 0;
    while (i < text.size()) {
        while (i < text.size() && text[i] == ' ')
            ++i;
        const auto j = text.find(' ', i);
        const auto word = text.substr(i, j == std::string_view::npos ? std::string_view::npos : j - i);
        if (!word.empty()) out.emplace_back(word);
        if (j == std::string_view::npos) break;
        i = j + 1;
    }
    return out;
}

std::string joinHistory(const std::vector<std::string>& history) {
    std::string out;
    for (const auto& h : history)
        out += (out.empty() ? "" : " ") + h;
    return out;
}

} // namespace iron
