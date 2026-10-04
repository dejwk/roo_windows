#include "roo_windows/core/clipper.h"

#include "roo_logging.h"

namespace roo_windows {
namespace internal {

RoundedClip* ClipperOutput::roundedClip(const void* owner_key) const {
  if (state_.rounded_ == nullptr) return nullptr;
  // Callers normally ask for the owner that was just prepared, or for an
  // enclosing owner immediately after its descendants finish. Search backward
  // so previously completed sibling subtrees do not add lookup cost.
  for (size_t i = state_.rounded_->clip_count; i > 0; --i) {
    RoundedClip* clip = state_.rounded_->clips[i - 1].get();
    if (clip->ownerKey() == owner_key) return clip;
  }
  return nullptr;
}

RoundedClip& ClipperOutput::prepareRoundedClip(const void* owner_key,
                                               roo_display::Box bounds,
                                               BorderStyle style) {
  if (state_.rounded_ == nullptr) {
    state_.rounded_.reset(new RoundedPaintState());
  }
  RoundedPaintState& arena = *state_.rounded_;
  // clip_count partitions retained slots into records used by this paint and
  // reusable records left from a previous paint. Grow only when this paint
  // exceeds the previous high-water mark.
  if (arena.clip_count == arena.clips.size()) {
    arena.clips.emplace_back(new RoundedClip());
  }
  RoundedClip* clip = arena.clips[arena.clip_count++].get();
  clip->reset(owner_key, bounds, style);
  clip->parent = arena.active;
  return *clip;
}

void ClipperOutput::activateRoundedClip(RoundedClip& clip) {
  DCHECK_NOTNULL(state_.rounded_.get());
  // Preparation records the mask that was active before this owner. Requiring
  // the same parent here catches overlapping or out-of-order clip scopes.
  DCHECK_EQ(state_.rounded_->active, clip.parent);
  state_.rounded_->active = &clip;
}

void ClipperOutput::deactivateRoundedClip() {
  RoundedClip* clip = activeRoundedClip();
  DCHECK_NOTNULL(clip);
  // Restore the exact enclosing chain captured at preparation. The record
  // itself remains retained for exclusions and final decoration reads.
  state_.rounded_->active = clip->parent;
}

void ClipperOutput::addRoundedExclusion(const roo_display::Box& exclusion) {
  using roo_display::Box;
  // A rectangle wholly inside every opaque mask interior needs no curved
  // geometry: ordinary rectangular subtraction is exact and cheaper.
  bool direct = true;
  for (RoundedClip* clip = activeRoundedClip(); clip != nullptr;
       clip = clip->parent) {
    direct = direct && clip->containsOpaque(exclusion);
  }
  if (direct) {
    addRectExclusion(exclusion);
    return;
  }

  // Keep one descriptor with the mask chain instead of expanding every row
  // into rectangles. Viewport intersection gives a conservative outer bound;
  // ExclusionUnion obtains the exact opaque span when it processes each row.
  Box bounds = exclusion;
  for (RoundedClip* clip = activeRoundedClip(); clip != nullptr;
       clip = clip->parent) {
    bounds.clip(clip->viewport());
  }
  if (bounds.empty()) return;
  const MaskedExclusion masked{bounds, activeRoundedClip()};
  // Registration is foreground first, so only the most recent tail entries
  // can be children covered by this enclosing exclusion. Remove an entry only
  // when the new descriptor proves complete coverage through the same masks.
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
  if (clip == nullptr) return;
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
  addOverlay(arena.decorations[arena.decoration_count++].get(), clip_box);
}

}  // namespace internal
}  // namespace roo_windows
