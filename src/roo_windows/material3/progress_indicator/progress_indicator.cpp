#include "roo_windows/material3/progress_indicator/progress_indicator.h"

#include <algorithm>
#include <cmath>

#include "roo_display/shape/smooth.h"
#include "roo_windows/material3/progress_indicator/progress_geometry.h"
#include "roo_windows/material3/theme.h"

namespace roo_windows::material3 {
namespace {
constexpr float kScale = ROO_WINDOWS_ZOOM / 100.0f;
constexpr float kTurn = 6.283185307179586f;

/// Registers one visible interval without prematurely rounding its endpoints.
void AddSegment(PaintContext& ctx, const Rect& bounds,
                internal::ProgressInterval interval, float thickness, bool rtl,
                roo_display::Color color) {
  if (interval.end <= interval.start) return;
  float width = bounds.width();
  if (rtl) interval = {width - interval.end, width - interval.start};
  thickness = std::min(thickness, interval.end - interval.start);
  float y = (bounds.height() - thickness) * 0.5f;
  ctx.addOverlayShape(
      roo_display::SmoothFilledRoundRect(
          interval.start - 0.5f, y - 0.5f, interval.end - 0.5f,
          y + thickness - 0.5f,
          std::min(thickness, interval.end - interval.start) * 0.5f, color),
      bounds);
}

/// Registers the frontmost stop before the track, without retaining shape
/// storage in the caller's geometry frame (required by the 256-byte target
/// gate).
[[gnu::noinline]] void AddStop(PaintContext& ctx, const Rect& bounds,
                               internal::ProgressInterval stop, float thickness,
                               bool rtl, roo_display::Color color) {
  if (stop.end <= stop.start) return;
  if (stop.end - stop.start < thickness) {
    AddSegment(ctx, bounds, stop, thickness, rtl, color);
    return;
  }
  float x = bounds.width() - thickness * 0.5f - 0.5f;
  if (rtl) x = bounds.width() - 1 - x;
  ctx.addOverlayShape(
      roo_display::SmoothFilledCircle({x, (bounds.height() - 1) * 0.5f},
                                      thickness * 0.5f, color),
      bounds);
}

/// Registers a ring or rounded partial arc using the shared smooth rasterizer.
void AddArc(PaintContext& ctx, const Rect& bounds, float radius,
            internal::ProgressArc arc, roo_display::Color color) {
  if (arc.thickness <= 0 || arc.end <= arc.start) return;
  roo_display::FpPoint center{(bounds.width() - 1) * 0.5f,
                              (bounds.height() - 1) * 0.5f};
  if (arc.end - arc.start >= kTurn) {
    ctx.addOverlayShape(
        roo_display::SmoothThickCircle(center, radius, arc.thickness, color),
        bounds);
  } else {
    ctx.addOverlayShape(
        roo_display::SmoothThickArc(center, radius, arc.thickness, arc.start,
                                    arc.end, color),
        bounds);
  }
}
}  // namespace

ProgressIndicator::ProgressIndicator(ApplicationContext& context, bool circular)
    : Widget(context),
      indeterminate_(false),
      motion_enabled_(true),
      rtl_(false),
      circular_(circular) {}

bool ProgressIndicator::setProgress(float fraction) {
  if (!std::isfinite(fraction)) return false;
  fraction = std::max(0.0f, std::min(1.0f, fraction));
  if (!indeterminate_ && fraction_ == fraction) return true;
  indeterminate_ = false;
  fraction_ = fraction;
  invalidateInk();
  return true;
}

void ProgressIndicator::setIndeterminate() {
  if (indeterminate_) return;
  indeterminate_ = true;
  if (motion_enabled_) LOG(WARNING) << "Unimplemented: progress animation";
  invalidateInk();
}

void ProgressIndicator::setMotionEnabled(bool enabled) {
  if (motion_enabled_ == enabled) return;
  motion_enabled_ = enabled;
  if (enabled && indeterminate_)
    LOG(WARNING) << "Unimplemented: progress animation";
  invalidateInk();
}

void ProgressIndicator::setRightToLeft(bool rtl) {
  if (rtl_ == rtl) return;
  rtl_ = rtl;
  invalidateInk();
}

void ProgressIndicator::invalidateInk() {
  if (bounds().empty()) return;
  float ink_width =
      circular_
          ? std::min(40 * kScale, std::min<float>(width(), height()) * 40 / 48)
          : width();
  float ink_height =
      circular_ ? ink_width : std::min<float>(4 * kScale, height());
  float x = (width() - ink_width) * 0.5f;
  float y = (height() - ink_height) * 0.5f;
  Rect ink(std::floor(x), std::floor(y), std::ceil(x + ink_width) - 1,
           std::ceil(y + ink_height) - 1);
  invalidateInterior(ink);
  notifyParentInvalidatedRegion(ink.translate(offsetLeft(), offsetTop()));
}

LinearProgressIndicator::LinearProgressIndicator(ApplicationContext& context)
    : ProgressIndicator(context, false) {}
void LinearProgressIndicator::setLayoutDirection(LayoutDirection direction) {
  setRightToLeft(direction == LayoutDirection::kRightToLeft);
}
LayoutDirection LinearProgressIndicator::layoutDirection() const {
  return rightToLeft() ? LayoutDirection::kRightToLeft
                       : LayoutDirection::kLeftToRight;
}
Dimensions LinearProgressIndicator::getSuggestedMinimumDimensions() const {
  return Dimensions(Scaled(240), Scaled(4));
}
Dimensions LinearProgressIndicator::onMeasure(WidthSpec width,
                                              HeightSpec height) {
  return Dimensions(width.kind() == UNSPECIFIED ? Scaled(240) : width.value(),
                    height.resolveSize(Scaled(4)));
}

void LinearProgressIndicator::paint(PaintContext& ctx) const {
  if (bounds().empty()) return;
  float thickness = std::min<float>(4 * kScale, height());
  internal::LinearProgressGeometry geometry =
      mode() == ProgressIndicatorMode::kDeterminate
          ? internal::LinearDeterminate(width(), thickness, 4 * kScale,
                                        progress())
          : internal::LinearSegments(width(), 4 * kScale,
                                     {width() * 0.4f, width() * 0.6f});
  const auto& colors = theme().material3Theme().color;
  for (int i = 0; i < geometry.active_count; ++i)
    AddSegment(ctx, bounds(), geometry.active[i], thickness, rightToLeft(),
               colors.primary);
  AddStop(ctx, bounds(), geometry.stop, thickness, rightToLeft(),
          colors.primary);
  for (int i = 0; i < geometry.track_count; ++i)
    AddSegment(ctx, bounds(), geometry.track[i], thickness, rightToLeft(),
               colors.secondaryContainer);
}

CircularProgressIndicator::CircularProgressIndicator(
    ApplicationContext& context)
    : ProgressIndicator(context, true) {}
Dimensions CircularProgressIndicator::getSuggestedMinimumDimensions() const {
  return Dimensions(Scaled(48), Scaled(48));
}
Dimensions CircularProgressIndicator::onMeasure(WidthSpec width,
                                                HeightSpec height) {
  return Dimensions(width.resolveSize(Scaled(48)),
                    height.resolveSize(Scaled(48)));
}
void CircularProgressIndicator::paint(PaintContext& ctx) const {
  if (bounds().empty()) return;
  float scale = std::min(kScale, std::min<float>(width(), height()) / 48);
  float radius = 18 * scale;
  float thickness = 4 * scale;
  const auto& colors = theme().material3Theme().color;
  bool known = mode() == ProgressIndicatorMode::kDeterminate;
  AddArc(ctx, bounds(), radius,
         internal::FitProgressArc(0, (known ? progress() : 0.25f) * kTurn,
                                  radius, thickness),
         colors.primary);
  if (known)
    AddArc(ctx, bounds(), radius,
           internal::CircularTrack(progress(), radius, thickness, 4 * scale),
           colors.secondaryContainer);
}
}  // namespace roo_windows::material3
