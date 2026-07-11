#include "roo_windows/material3/badge/badge.h"

#include <algorithm>
#include <cstdio>
#include <cstring>

#include "roo_display/ui/text_label.h"
#include "roo_windows/config.h"
#include "roo_windows/core/number.h"
#include "roo_windows/core/paint_context.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/material3/theme.h"

namespace roo_windows {
namespace material3 {

namespace {

using roo_display::Box;
using roo_display::StringViewLabel;

constexpr int16_t kDotSize = Scaled(8);
constexpr int16_t kTextMinHeight = Scaled(16);
constexpr int16_t kTextMinWidth = Scaled(16);
constexpr int16_t kTextPaddingH = Scaled(4);
constexpr int16_t kTextPaddingV = Scaled(2);
constexpr int16_t kDotDefaultOffset = Scaled(6);
constexpr int16_t kTextDefaultOffset = Scaled(12);

Rect EmptyRect() { return Rect(0, 0, -1, -1); }

/// Copies at most 7 visible bytes so the inline string remains null-terminated.
void CopyClampedText(roo_collections::SmallString<Badge::kTextStorage>& target,
                     roo::string_view text) {
  size_t size = std::min(text.size(), Badge::kMaxVisibleText);
  char buffer[Badge::kTextStorage];
  if (size > 0) {
    std::memcpy(buffer, text.data(), size);
  }
  buffer[size] = '\0';
  target = roo::string_view(buffer, size);
}

void MeasureTextBadge(roo::string_view text, int16_t& width, int16_t& height) {
  StringViewLabel label(text, font_caption(), roo_display::color::Black);
  Box extents = label.anchorExtents();
  width = std::max<int16_t>(kTextMinWidth, extents.width() + 2 * kTextPaddingH);
  height =
      std::max<int16_t>(kTextMinHeight, extents.height() + 2 * kTextPaddingV);
}

int16_t MaxCaptionCharacterSpan() {
  static const int16_t kSpan = []() {
    int16_t max_span = 0;
    char sample[2] = {' ', '\0'};
    const roo_display::Font& font = font_caption();
    for (int code = 32; code <= 126; ++code) {
      sample[0] = static_cast<char>(code);
      roo_display::GlyphMetrics metrics =
          font.getHorizontalStringMetrics(sample);
      max_span = std::max<int16_t>(
          max_span, std::max<int16_t>(metrics.advance(),
                                      metrics.screen_extents().width()));
    }
    return max_span;
  }();
  return kSpan;
}

int16_t ConservativeTextWidth() {
  return std::max<int16_t>(
      kTextMinWidth,
      2 * kTextPaddingH + Badge::kMaxVisibleText * MaxCaptionCharacterSpan());
}

int16_t ConservativeTextHeight() {
  return std::max<int16_t>(
      kTextMinHeight,
      2 * kTextPaddingV + font_caption().metrics().linespace() + 1);
}

int16_t DefaultOffset(BadgeMode mode) {
  return mode == BadgeMode::kText ? kTextDefaultOffset : kDotDefaultOffset;
}

uint8_t CornerRadius(const Rect& bounds) {
  int16_t min_dimension =
      std::min<int16_t>(bounds.width(), static_cast<int16_t>(bounds.height()));
  return std::max<int16_t>(0, min_dimension / 2);
}

int16_t CornerInset(uint8_t radius) { return radius - (181 * radius) / 256; }

Rect ResolveBadgeBounds(const Rect& anchor_bounds,
                        const BadgePlacement& placement, int16_t badge_width,
                        int16_t badge_height, int16_t anchor_width,
                        int16_t anchor_height, BadgeMode mode) {
  if (anchor_bounds.empty() || badge_width <= 0 || badge_height <= 0 ||
      anchor_width <= 0 || anchor_height <= 0) {
    return EmptyRect();
  }

  int16_t horizontal_offset = DefaultOffset(mode) + placement.horizontal_offset;
  int16_t vertical_offset = DefaultOffset(mode) + placement.vertical_offset;
  int16_t top = anchor_bounds.yMin() - anchor_height / 2 + vertical_offset;

  if (placement.gravity == BadgeGravity::kTopStart) {
    int16_t left = anchor_bounds.xMin() - anchor_width / 2 + horizontal_offset;
    return Rect(left, top, left + badge_width - 1, top + badge_height - 1);
  }

  int16_t right = anchor_bounds.xMax() + anchor_width / 2 - horizontal_offset;
  return Rect(right - badge_width + 1, top, right, top + badge_height - 1);
}

Rect InnerBounds(const Rect& bounds) {
  uint8_t radius = CornerRadius(bounds);
  int16_t inset = CornerInset(radius);
  return Rect(bounds.xMin() + inset, bounds.yMin() + inset,
              bounds.xMax() - inset, bounds.yMax() - inset);
}

}  // namespace

Badge::Badge() : text_(), bounds_(EmptyRect()), mode_(0), valid_(false) {}

BadgeMode Badge::mode() const { return static_cast<BadgeMode>(mode_); }

bool Badge::visible() const { return mode() != BadgeMode::kHidden; }

bool Badge::hasText() const {
  return mode() == BadgeMode::kText && !text_.empty();
}

roo::string_view Badge::text() const {
  return static_cast<roo::string_view>(text_);
}

void Badge::hide() {
  mode_ = static_cast<uint8_t>(BadgeMode::kHidden);
  clearText();
}

void Badge::setDot() {
  mode_ = static_cast<uint8_t>(BadgeMode::kDot);
  clearText();
}

void Badge::setText(roo::string_view text) {
  CopyClampedText(text_, text);
  mode_ = static_cast<uint8_t>(BadgeMode::kText);
  bounds_ = EmptyRect();
  valid_ = false;
}

void Badge::clearText() {
  text_ = "";
  bounds_ = EmptyRect();
  valid_ = false;
}

void Badge::setValue(unsigned int number) {
  char buffer[Badge::kTextStorage];
  if (number > 999) {
    std::snprintf(buffer, sizeof(buffer), "999+");
  } else {
    std::snprintf(buffer, sizeof(buffer), "%u", number);
  }
  setText(roo::string_view(buffer));
}

bool Badge::layout(const Rect& anchor_bounds, const BadgePlacement& placement) {
  bounds_ = EmptyRect();
  valid_ = false;
  if (!visible() || anchor_bounds.empty()) return false;

  BadgeMode current_mode = mode();
  int16_t width = 0;
  int16_t height = 0;
  int16_t anchor_width = 0;
  int16_t anchor_height = 0;
  if (current_mode == BadgeMode::kDot) {
    width = kDotSize;
    height = kDotSize;
    anchor_width = kDotSize;
    anchor_height = kDotSize;
  } else {
    MeasureTextBadge(text(), width, height);
    anchor_width = kTextMinWidth;
    anchor_height = kTextMinHeight;
  }

  bounds_ = ResolveBadgeBounds(anchor_bounds, placement, width, height,
                               anchor_width, anchor_height, current_mode);
  valid_ = !bounds_.empty();
  return valid_;
}

Rect Badge::bounds() const { return valid_ ? bounds_ : EmptyRect(); }

Rect Badge::ConservativeBounds(const Rect& anchor_bounds,
                               const BadgePlacement& placement,
                               bool for_text_badge) {
  if (for_text_badge) {
    return ResolveBadgeBounds(anchor_bounds, placement, ConservativeTextWidth(),
                              ConservativeTextHeight(), kTextMinWidth,
                              kTextMinHeight, BadgeMode::kText);
  }
  return ResolveBadgeBounds(anchor_bounds, placement, kDotSize, kDotSize,
                            kDotSize, kDotSize, BadgeMode::kDot);
}

void Badge::paint(PaintContext& ctx, const Theme& theme) const {
  if (!valid_ || bounds_.empty() || !visible()) return;

  roo_display::Color badge_color = theme.material3Theme().color.error;
  roo_display::Color text_color = theme.material3Theme().color.onError;
  Rect inner = InnerBounds(bounds_);
  if (!inner.empty()) {
    PaintContext sub = ctx.clipped(inner);
    if (!sub.empty()) {
      sub.setBgcolor(badge_color);
      if (mode() == BadgeMode::kText && hasText()) {
        sub.drawTiled(StringViewLabel(text(), font_caption(), text_color),
                      inner, roo_display::kCenter | roo_display::kMiddle);
      } else {
        sub.fillRect(inner, badge_color);
      }
    }
  }

  uint8_t radius = CornerRadius(bounds_);
  PaintDecoration decoration;
  decoration.bounds = bounds_;
  decoration.background = badge_color;
  decoration.corner_radii = {radius, radius, radius, radius};
  decoration.outline_width = SmallNumber(0);
  decoration.outline_color = badge_color;
  ctx.addDecoration(decoration);
  if (!inner.empty()) {
    ctx.addExclusion(inner);
  }
}

}  // namespace material3
}  // namespace roo_windows
