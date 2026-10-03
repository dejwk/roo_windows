#include "roo_windows/core/clipper.h"

namespace roo_windows {
namespace internal {

RoundedClip* ClipperOutput::roundedClip(const void* owner) const {
  if (state_.rounded_ == nullptr) return nullptr;
  for (size_t i = 0; i < state_.rounded_->clip_count; ++i) {
    RoundedClip* clip = state_.rounded_->clips[i].get();
    if (clip->owner() == owner) return clip;
  }
  return nullptr;
}

RoundedClip& ClipperOutput::beginRoundedClip(const void* owner,
                                             roo_display::Box bounds,
                                             BorderStyle style, bool& fresh) {
  if (state_.rounded_ == nullptr) {
    state_.rounded_.reset(new RoundedPaintState());
  }
  RoundedPaintState& arena = *state_.rounded_;
  RoundedClip* clip = roundedClip(owner);
  fresh = clip == nullptr;
  if (fresh) {
    if (arena.clip_count == arena.clips.size()) {
      arena.clips.emplace_back(new RoundedClip());
    }
    clip = arena.clips[arena.clip_count++].get();
    clip->reset(owner, bounds, style);
  }
  clip->fresh = fresh;
  clip->parent = arena.active;
  arena.active = clip;
  return *clip;
}

void ClipperOutput::addRoundedExclusion(const roo_display::Box& exclusion) {
  using roo_display::Box;
  bool direct = true;
  for (RoundedClip* clip = activeRoundedClip(); clip != nullptr;
       clip = clip->parent) {
    direct = direct && clip->containsOpaque(exclusion);
  }
  if (direct) {
    addRectExclusion(exclusion);
    return;
  }
  Box run(0, 0, -1, -1);
  for (int16_t y = exclusion.yMin(); y <= exclusion.yMax(); ++y) {
    int16_t lo = exclusion.xMin();
    int16_t hi = exclusion.xMax();
    for (RoundedClip* clip = activeRoundedClip(); clip != nullptr;
         clip = clip->parent) {
      int16_t cl;
      int16_t cr;
      clip->opaqueSpan(y, cl, cr);
      lo = std::max(lo, cl);
      hi = std::min(hi, cr);
    }
    if (!run.empty() && (lo != run.xMin() || hi != run.xMax())) {
      addRectExclusion(run);
      run = Box(0, 0, -1, -1);
    }
    if (hi >= lo) run = Box(lo, run.empty() ? y : run.yMin(), hi, y);
  }
  if (!run.empty()) addRectExclusion(run);
}

const roo_display::Rasterizable* ClipperOutput::maskRoundedOverlay(
    const roo_display::Rasterizable* source, roo_display::Box clip, int16_t& dx,
    int16_t& dy) {
  RoundedPaintState& arena = *state_.rounded_;
  bool direct = true;
  const roo_display::Box bounds =
      roo_display::Box::Intersect(source->extents().translate(dx, dy), clip);
  for (RoundedClip* mask = arena.active; mask != nullptr; mask = mask->parent) {
    mask->accumulateOverlay(*source, clip, dx, dy, arena.active);
    direct = direct && mask->containsOpaque(bounds);
  }
  if (direct) return source;
  RoundedOverlay wrapper(source, clip, dx, dy, arena.active);
  if (arena.overlay_count == arena.overlays.size()) {
    arena.overlays.emplace_back(new RoundedOverlay(wrapper));
  } else {
    *arena.overlays[arena.overlay_count] = wrapper;
  }
  dx = 0;
  dy = 0;
  return arena.overlays[arena.overlay_count++].get();
}

void ClipperOutput::addRoundedDecoration(
    const void* owner, roo_display::Box clip_box, roo_display::Box extents,
    int elevation, roo_display::Color bgcolor, BorderStyle border,
    roo_display::Color outline_color) {
  RoundedClip* clip = roundedClip(owner);
  if (clip == nullptr || !clip->completed || clip->published) return;
  const OverlaySpec& spec = currentOverlaySpec();
  const PressOverlay* press = ((spec.is_area() && spec.has_press_overlay()) ||
                               scoped_press_overlay_active_)
                                  ? &press_overlay_
                                  : nullptr;
  Decoration decoration(extents, elevation, spec, press, bgcolor,
                        border.corner_radii(), border.outline_width(),
                        outline_color, true);
  RoundedDecoration wrapper(
      std::move(decoration), clip, bgcolor,
      spec.is_area() ? spec.base_overlay() : roo_display::Color(0));
  RoundedPaintState& arena = *state_.rounded_;
  if (arena.decoration_count == arena.decorations.size()) {
    arena.decorations.emplace_back(new RoundedDecoration(wrapper));
  } else {
    *arena.decorations[arena.decoration_count] = wrapper;
  }
  clip->published = true;
  addOverlay(arena.decorations[arena.decoration_count++].get(), clip_box);
}

}  // namespace internal
}  // namespace roo_windows
