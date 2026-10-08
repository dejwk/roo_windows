#include "roo_windows/core/clipper.h"

#include "roo_logging.h"
#include "roo_windows/core/canvas.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {
namespace internal {

roo_display::Box ClipperOutput::prepareClip(roo_display::Box requested) {
  setBounds(requested);
  prepareExclusions();
  const roo_display::Box visible = exclusion_union_.visibleBounds(requested);
  // The original prepared inputs also describe this subset. Adopting the
  // trimmed clip here lets its first output reuse them without another scan.
  // Overlays, if previously prepared, need the narrower bounds on next output.
  if (visible != bounds_) valid_ = false;
  bounds_ = visible;
  return visible;
}

void ClipperOutput::prepareExclusions() {
  if (exclusions_valid_) return;
  bounded_exclusions_.clear();
  for (const auto& e : exclusions_) {
    if (e.intersects(bounds_)) {
      bounded_exclusions_.push_back(e);
    }
  }
  const roo_display::Box* exclusion_begin =
      bounded_exclusions_.empty() ? nullptr : &bounded_exclusions_.front();
  const roo_display::Box* exclusion_end = exclusion_begin;
  if (exclusion_end != nullptr) {
    exclusion_end += bounded_exclusions_.size();
  }
  // ExclusionUnion borrows contiguous arrays until the next rebuild. Copy
  // only masked descriptors intersecting the current output bounds so the
  // subtraction walk does not inspect unrelated rounded regions.
  const MaskedExclusion* masked_begin = nullptr;
  const MaskedExclusion* masked_end = nullptr;
  if (state_.rounded_ != nullptr) {
    auto& bounded = state_.rounded_->bounded_exclusions;
    bounded.clear();
    for (const MaskedExclusion& e : state_.rounded_->exclusions) {
      if (e.bounds.intersects(bounds_)) bounded.push_back(e);
    }
    if (!bounded.empty()) {
      masked_begin = bounded.data();
      masked_end = masked_begin + bounded.size();
    }
  }
  exclusion_union_.reset(exclusion_begin, exclusion_end, masked_begin,
                         masked_end);

  exclusions_valid_ = true;
}

RoundedPaintState& ClipperOutput::roundedArena() {
  // Keep rectangular-only paints free of rounded/effect arena storage. Once
  // needed, the arena remains in ClipperState so later paints reuse its slots.
  if (state_.rounded_ == nullptr) {
    state_.rounded_.reset(new RoundedPaintState());
  }
  return *state_.rounded_;
}

const PressOverlay* ClipperOutput::configurePressOverlay(
    const PressOverlaySpec& spec) {
  if (!spec.enabled) return nullptr;
  // PaintEffect records borrow this shared object. The input model permits one
  // active press animation, so its sample remains unchanged for this paint.
  press_overlay_ =
      PressOverlay(spec.center_x, spec.center_y, spec.radius, spec.color);
  if (spec.clipped_to_circle) {
    press_overlay_.setClipCircle(spec.clip_circle_center_x,
                                 spec.clip_circle_center_y,
                                 spec.clip_circle_radius);
  }
  return &press_overlay_;
}

void ClipperOutput::pushOverlaySpec(Widget& widget, const Canvas& canvas) {
  OverlaySpec spec(widget, canvas);
  if (!spec.is_modded()) {
    // Consecutive inert widget frames are indistinguishable. Compress them so
    // ordinary deep trees do not grow the overlay-spec deque while painting.
    if (!overlay_specs_.empty() &&
        !overlay_specs_.back().overlay_spec.is_modded()) {
      ++overlay_specs_.back().refcount;
      return;
    }
    overlay_specs_.emplace_back(OverlaySpec(), 1);
    return;
  }
  if (HasContentEffect(spec)) {
    // Area overlays and disabled styling modulate the widget's complete
    // subtree. Capture an immutable linked scope so deferred descendants can
    // apply the same effect after this widget's call frame has returned.
    RoundedPaintState& arena = roundedArena();
    roo_display::Color tint = spec.base_overlay();
    if (spec.is_disabled()) {
      // Disabled content is faded toward its resolved background. SourceAtop
      // with background alpha (1 - disabled opacity) implements that transform
      // while preserving the source coverage used at rounded boundaries.
      const uint8_t opacity =
          widget.theme().framework.interaction.disabledContentOpacity;
      tint = canvas.bgcolor().withA(255 - ((255 * opacity) >> 7));
    }
    const PressOverlay* press = configurePressOverlay(spec.press_overlay());
    const PaintEffect snapshot(arena.active_effect, canvas.clip_box(), tint,
                               press);
    // Deferred overlays borrow effect nodes, so retain stable slots until the
    // complete paint finishes and reuse them by high-water index next frame.
    if (arena.effect_count == arena.effects.size()) {
      arena.effects.emplace_back(new PaintEffect(snapshot));
    } else {
      *arena.effects[arena.effect_count] = snapshot;
    }
    arena.active_effect = arena.effects[arena.effect_count++].get();
    valid_ = false;
  }
  overlay_specs_.emplace_back(std::move(spec), 1);
}

void ClipperOutput::popOverlaySpec() {
  if (overlay_specs_.empty()) return;
  if (overlay_specs_.back().refcount > 1) {
    --overlay_specs_.back().refcount;
    return;
  }
  if (HasContentEffect(overlay_specs_.back().overlay_spec)) {
    // active_effect mirrors only frames that created PaintEffect nodes; inert
    // and point/custom overlay frames do not change this linked stack.
    state_.rounded_->active_effect = activeEffect()->parent();
    valid_ = false;
  }
  overlay_specs_.pop_back();
}

void ClipperOutput::addOverlayWithEffects(
    const roo_display::Rasterizable* source, roo_display::Box clip, int16_t dx,
    int16_t dy, const PaintEffect* effects) {
  RoundedClip* masks = activeRoundedClip();
  if (masks != nullptr || effects != nullptr) {
    // Masks and effects are independent: rounded content may have no styling,
    // while an ordinary rectangular subtree may inherit an effect. A mask
    // requires boundary capture and clipping; an effect requires deferred
    // color modulation. Either one needs a wrapper that snapshots both chains.
    source = maskRoundedOverlay(source, clip, dx, dy, masks, effects);
  }
  overlays_.emplace_back(source, clip, dx, dy);
  if (overlays_.back().extents().empty()) {
    overlays_.pop_back();
    return;
  }
  valid_ = false;
}

void ClipperOutput::addDecoration(roo_display::Box clip,
                                  roo_display::Box extents, int elevation,
                                  roo_display::Color bgcolor,
                                  BorderStyle::CornerRadii radii,
                                  SmallNumber outline_width,
                                  roo_display::Color outline_color) {
  const PaintEffect* own = ownEffect();
  if (own != nullptr && currentOverlaySpec().is_disabled()) {
    // own identifies an effect created by this widget rather than an inherited
    // one. Decoration handles area overlays itself, but disabled styling is a
    // complete-content transform. Resolve that transform into this widget's
    // fill and outline now; the wrapper below starts at own->parent() so the
    // effect is not applied twice and the widget's own shadow stays unchanged.
    roo_display::Blender<roo_display::BlendingMode::kSourceAtop> tint;
    bgcolor = tint.apply(bgcolor, own->tint());
    outline_color = tint.apply(outline_color, own->tint());
  }
  decorations_.emplace_back(extents, elevation, currentOverlaySpec(),
                            own == nullptr ? nullptr : own->press(), bgcolor,
                            radii, outline_width, outline_color);
  // Decoration resolves its own fill/outline effect; inherited effects also
  // apply to child decoration layers before their enclosing owner's coverage.
  addOverlayWithEffects(&decorations_.back(), clip, 0, 0,
                        own == nullptr ? activeEffect() : own->parent());
}

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
  RoundedPaintState& arena = roundedArena();
  // clip_count partitions retained slots into records used by this paint and
  // reusable records left from a previous paint. Grow only when this paint
  // exceeds the previous high-water mark.
  if (arena.clip_count == arena.clips.size()) {
    arena.clips.emplace_back(new RoundedClip());
  }
  RoundedClip* clip = arena.clips[arena.clip_count++].get();
  clip->reset(owner_key, bounds, style);
  // Preparation precedes activation so unclipped children bypass this mask.
  // Remember the enclosing chain now; activation later verifies this nesting.
  clip->parent = arena.active;
  // Boundary capture applies effects introduced by descendants, stopping
  // before the owner's effect. RoundedDecoration applies that owner effect
  // once after it resolves the complete child/background group. Borrow the
  // mutable top pointer so samples captured inside child scopes see those
  // descendant effects.
  clip->effect_limit = arena.active_effect;
  clip->active_effects = &arena.active_effect;
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
  exclusions_valid_ = false;
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
  exclusions_valid_ = false;
}

const roo_display::Rasterizable* ClipperOutput::maskRoundedOverlay(
    const roo_display::Rasterizable* source, roo_display::Box clip, int16_t& dx,
    int16_t& dy, RoundedClip* masks, const PaintEffect* effects) {
  RoundedPaintState& arena = roundedArena();
  bool direct = true;
  const roo_display::Box bounds =
      roo_display::Box::Intersect(source->extents().translate(dx, dy), clip);
  // Fractional boundary pixels cannot be emitted later as ordinary opaque
  // overlay pixels. Capture their contribution now in every mask they reach.
  // A source wholly inside every opaque interior can still bypass masking.
  for (RoundedClip* mask = masks; mask != nullptr; mask = mask->parent) {
    mask->accumulateOverlay(*source, clip, dx, dy, masks, effects);
    direct = direct && mask->containsOpaque(bounds);
  }
  if (direct && effects == nullptr) return source;

  // Retain a raster wrapper for the opaque spans and/or deferred effects. Its
  // mask and effect pointers borrow stable arena nodes for the rest of paint.
  RoundedOverlay wrapper(source, clip, dx, dy, masks, effects);
  if (arena.overlay_count == arena.overlays.size()) {
    arena.overlays.emplace_back(new RoundedOverlay(wrapper));
  } else {
    *arena.overlays[arena.overlay_count] = wrapper;
  }
  // The wrapper stores the original translation and exposes device-space
  // extents, so its ClippedOverlay descriptor must not translate it again.
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
  const PaintEffect* own = ownEffect();
  // RoundedDecoration substitutes the sparsely captured child/background group
  // before fill and outline coverage. Give Decoration an inert OverlaySpec;
  // the wrapper applies the owner's complete-content effect exactly once after
  // resolving that group.
  Decoration decoration(extents, elevation, OverlaySpec(), nullptr, bgcolor,
                        border.corner_radii(), border.outline_width(),
                        outline_color, true);
  RoundedDecoration wrapper(std::move(decoration), clip, bgcolor, outline_color,
                            own);
  RoundedPaintState& arena = *state_.rounded_;
  // ClippedOverlay borrows this rasterizer, so use a stable high-water slot
  // rather than a local object or a movable vector element.
  if (arena.decoration_count == arena.decorations.size()) {
    arena.decorations.emplace_back(new RoundedDecoration(wrapper));
  } else {
    *arena.decorations[arena.decoration_count] = wrapper;
  }
  addOverlayWithEffects(arena.decorations[arena.decoration_count++].get(),
                        clip_box, 0, 0,
                        own == nullptr ? activeEffect() : own->parent());
}

}  // namespace internal
bool internal::ClipperOutput::overlaysIntersect(
    const roo_display::Box& box) const {
  for (const ClippedOverlay& overlay : overlays_) {
    if (!roo_display::Box::Intersect(box, overlay.extents()).empty())
      return true;
  }
  return false;
}

roo_display::Box Clipper::opaqueInterior(roo_display::Box viewport) const {
  for (const internal::RoundedClip* mask = out_.activeRoundedClip();
       mask != nullptr; mask = mask->parent) {
    viewport = roo_display::Box::Intersect(viewport, mask->opaqueInterior());
  }
  return viewport;
}

bool Clipper::backgroundUnobscured(const roo_display::Box& viewport) const {
  if (hasContentEffects() || out_.overlaysIntersect(viewport)) return false;
  for (const roo_display::Box& exclusion : exclusions()) {
    if (!roo_display::Box::Intersect(viewport, exclusion).empty()) return false;
  }
  for (const internal::MaskedExclusion& exclusion : maskedExclusions()) {
    if (!roo_display::Box::Intersect(viewport, exclusion.bounds).empty())
      return false;
  }
  return true;
}

bool Clipper::canDeferBackground(const roo_display::Box& box,
                                 roo_display::Color background) const {
  const internal::BackgroundDeferralScope* scope = background_scope_;
  if (scope == nullptr || scope->viewport_.empty() ||
      !scope->interior_.contains(box) || !background.isOpaque() ||
      hasContentEffects())
    return false;
  for (const internal::RoundedClip* mask = out_.activeRoundedClip();
       mask != nullptr; mask = mask->parent) {
    if (!mask->containsOpaque(box)) return false;
  }
  int band = (box.yMin() - scope->viewport_.yMin()) / 16;
  return band != scope->band_ && paintBudgetExceeded();
}

void Clipper::preserveBackground(const roo_display::Box& box) {
  out_.addRectExclusion(box);
  background_scope_->deferred_ = true;
  background_deferred_ = true;
}

}  // namespace roo_windows
