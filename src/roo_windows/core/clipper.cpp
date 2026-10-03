#include "roo_windows/core/clipper.h"

#include "roo_logging.h"

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

RoundedClip& ClipperOutput::prepareRoundedClip(const void* owner,
                                               roo_display::Box bounds,
                                               BorderStyle style, bool& fresh) {
  if (state_.rounded_ == nullptr) {
    state_.rounded_.reset(new RoundedPaintState());
  }
  RoundedPaintState& arena = *state_.rounded_;
  RoundedClip* clip = roundedClip(owner);
  if (clip == nullptr) {
    if (arena.clip_count == arena.clips.size()) {
      arena.clips.emplace_back(new RoundedClip());
    }
    clip = arena.clips[arena.clip_count++].get();
    clip->reset(owner, bounds, style);
    clip->parent = arena.active;
  }
  fresh = clip->fresh;
  return *clip;
}

void ClipperOutput::activateRoundedClip(RoundedClip& clip) {
  DCHECK_NOTNULL(state_.rounded_.get());
  DCHECK_EQ(state_.rounded_->active, clip.parent);
  state_.rounded_->active = &clip;
}

void ClipperOutput::deactivateRoundedClip() {
  RoundedClip* clip = activeRoundedClip();
  DCHECK_NOTNULL(clip);
  state_.rounded_->active = clip->parent;
  clip->fresh = false;
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
  Box bounds = exclusion;
  for (RoundedClip* clip = activeRoundedClip(); clip != nullptr;
       clip = clip->parent) {
    bounds.clip(clip->viewport());
  }
  if (bounds.empty()) return;
  const MaskedExclusion masked{bounds, activeRoundedClip()};
  // Prove coverage cheaply, otherwise retain the older descriptor/overlay.
  while (!exclusions_.empty() && masked.contains(exclusions_.back())) {
    exclusions_.pop_back();
  }
  auto& masks = state_.rounded_->exclusions;
  while (!masks.empty() && masked.contains(masks.back().bounds)) {
    masks.pop_back();
  }
  masks.push_back(masked);
  while (!overlays_.empty() && masked.contains(overlays_.back().extents())) {
    overlays_.pop_back();
  }
  valid_ = false;
}

void ClipperOutput::addRectExclusion(const roo_display::Box& exclusion) {
  // Foreground-first registration lets enclosing rectangles fold recent child
  // exclusions. Masked bounds are a conservative coverage proof too.
  while (!exclusions_.empty() && exclusion.contains(exclusions_.back())) {
    exclusions_.pop_back();
  }
  if (state_.rounded_ != nullptr) {
    auto& masks = state_.rounded_->exclusions;
    while (!masks.empty() && exclusion.contains(masks.back().bounds)) {
      masks.pop_back();
    }
  }
  exclusions_.push_back(exclusion);
  while (!overlays_.empty() && exclusion.contains(overlays_.back().extents())) {
    overlays_.pop_back();
  }
  valid_ = false;
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
