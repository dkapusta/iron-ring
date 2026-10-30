#include "Game/SceneStack.h"

#include <algorithm>

namespace iron {

void SceneStack::push(std::unique_ptr<Scene> scene, Transition transition) {
    request(Kind::Push, std::move(scene), transition);
}
void SceneStack::pop(Transition transition) {
    request(Kind::PopTo, nullptr, transition, scenes_.empty() ? 0 : scenes_.size() - 1);
}
void SceneStack::popTo(std::size_t depth, Transition transition) { request(Kind::PopTo, nullptr, transition, depth); }
void SceneStack::replaceTop(std::unique_ptr<Scene> scene, Transition transition) {
    request(Kind::Replace, std::move(scene), transition);
}

void SceneStack::request(Kind kind, std::unique_ptr<Scene> scene, Transition transition, std::size_t depth) {
    if (pending_) return; // the first request in a frame wins (a double press can't push twice)
    pending_ = Request{kind, std::move(scene), transition, depth};
    if (transition == Transition::Fade) fadeOut_ = true;
}

void SceneStack::settle() {
    if (pending_ && pending_->transition == Transition::Cut) apply();
}

void SceneStack::apply() {
    Request r = std::move(*pending_);
    pending_.reset();
    switch (r.kind) {
    case Kind::Push: scenes_.push_back(std::move(r.scene)); break;
    case Kind::PopTo:
        while (scenes_.size() > r.depth) scenes_.pop_back(); // top first: upper scenes may refer to lower ones
        break;
    case Kind::Replace:
        if (!scenes_.empty()) scenes_.pop_back();
        scenes_.push_back(std::move(r.scene));
        break;
    }
    ++generation_;
    if (!scenes_.empty()) scenes_.back()->onEnter();
}

void SceneStack::handleEvent(const SDL_Event& event) {
    settle();
    if (scenes_.empty() || fading()) return;
    scenes_.back()->handleEvent(event);
    settle();
}

void SceneStack::handleMenu(const MenuEvent& event) {
    settle();
    if (scenes_.empty() || fading()) return;
    scenes_.back()->handleMenu(event);
    settle();
}

void SceneStack::fixedUpdate() {
    settle();
    if (fadeOut_) {
        if (++fade_ >= kFadeTicks) {
            fadeOut_ = false;
            if (pending_) apply();
        }
    } else if (fade_ > 0) {
        --fade_;
    }
    if (!scenes_.empty()) scenes_.back()->fixedUpdate();
    settle();
}

void SceneStack::render(const RenderContext& context, float width, float height) {
    SDL_Renderer* r = context.renderer;
    if (scenes_.empty()) {
        SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
        SDL_RenderClear(r);
        return;
    }
    // Draw from the topmost opaque scene upwards, so overlays show what's beneath.
    std::size_t first = scenes_.size() - 1;
    while (first > 0 && !scenes_[first]->opaque()) --first;
    for (std::size_t i = first; i < scenes_.size(); ++i) scenes_[i]->render(context);

    if (fade_ > 0) {
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
        const float t = static_cast<float>(fade_) / kFadeTicks;
        SDL_SetRenderDrawColor(r, 0, 0, 0, static_cast<Uint8>(std::clamp(t, 0.0f, 1.0f) * 255.0f));
        const SDL_FRect all{0, 0, width, height};
        SDL_RenderFillRect(r, &all);
    }
}

std::string SceneStack::status() const { return scenes_.empty() ? std::string("no scene") : scenes_.back()->status(); }

} // namespace iron
