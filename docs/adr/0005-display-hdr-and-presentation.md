# ADR-0005: Native-resolution fullscreen, 640×360 logical canvas, scRGB HDR

- **Status:** Accepted
- **Date:** 2026-10-28

## Context

Requirements: fullscreen at the native desktop resolution, and HDR on compatible displays. The art direction is pixel art on a 640×360 logical playfield (concept kit asset guide), but the current placeholder art is high-resolution (1920×1080 stages, ~1774×887 atlases).

## Decision

**Window.** `SDL_WINDOW_FULLSCREEN` without an exclusive mode = borderless fullscreen at the primary display's desktop mode, plus `SDL_WINDOW_HIGH_PIXEL_DENSITY` so HiDPI platforms render at true native pixels. No mode switching. `-screen-fullscreen 0` gives a resizable window; F11 / Alt+Enter toggles.

**Presentation.** Game code draws in a **640×360 logical coordinate space** via `SDL_SetRenderLogicalPresentation`:

- `integer` (default): largest integer scale that fits, black borders otherwise (1080p → 3×, 1440p → 4×, 2160p → 6×). Pixel-perfect.
- `letterbox`: fill one axis with a fractional scale. For odd displays (1366×768) where integer bars are too large.

SDL3 applies logical presentation by scaling draw coordinates, not via an intermediate 640×360 texture, so high-resolution placeholder textures still render at full native detail. Texture filtering is a per-project setting (`GameSettings.json`, `textureFilter`): `linear` for the concept art, `pixelart` (`SDL_SCALEMODE_PIXELART`) once sprites live on the 640×360 grid. *Update (M1): all art is now on the 640×360 grid and the project uses `pixelart` ([ADR-0011](0011-procedural-interim-art.md)).*

**HDR.** At startup the window's `SDL_PROP_WINDOW_HDR_ENABLED_BOOLEAN` decides (`boot.config: hdr=auto`; `on`/`off` override). If HDR is wanted, the renderer is created with `SDL_COLORSPACE_SRGB_LINEAR` (scRGB; SDL documents support in the direct3d11, direct3d12 and metal renderers). If creation fails, we fall back to the default sRGB renderer and log why.

With scRGB output SDL multiplies the SDR white point into the colour scale automatically, so regular sRGB art looks identical to SDR. We spend the extra range only on **highlights**: impact sparks are drawn with `SDL_SetRenderColorScale(min(headroom, hdr-max-boost))`, default boost ceiling 2.0. On SDR the boost is 1.0 and nothing changes.

`SDL_EVENT_WINDOW_HDR_STATE_CHANGED` refreshes headroom/white level. If the display turns HDR on while the renderer is sRGB, we log that a restart is needed; renderer recreation is M2.

## Consequences

- One code path for all displays; HDR is additive and can't break SDR.
- Overlay and debug text are also in logical pixels (8×8 debug font at 640×360 is chunky but legible).
- Linux: `SDL_Renderer` backends there (OpenGL/Vulkan/GPU) may not expose scRGB on every compositor; Windows and macOS are the primary HDR targets.
- HDR highlights need tuning on real hardware; `hdr-max-boost` in `boot.config` makes that a data change.
- Future HDR-native VFX can use float textures (`SDL_PIXELFORMAT_RGBA64_FLOAT`) without changing the pipeline.

## Alternatives considered

- **Exclusive fullscreen mode changes**: slower alt-tab, flicker, no benefit for a 60 Hz 2D game.
- **Render to a 640×360 target texture and upscale**: crisp for final pixel art, but destroys detail of the current high-res placeholders; can be revisited after the art pass.
- **HDR10 (PQ) output**: SDL supports it for some backends, but scRGB is simpler (linear, SDR-compatible) and widely supported on Windows.
