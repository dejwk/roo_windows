#pragma once

#include <stddef.h>
#include <stdint.h>

#include "roo_backport/string_view.h"
#include "roo_collections/small_string.h"
#include "roo_windows/core/rect.h"

namespace roo_windows {

class PaintContext;
struct Theme;

namespace material3 {

/// Visibility/content mode for a Material 3 badge helper.
enum class BadgeMode : uint8_t {
  kHidden,
  kDot,
  kText,
};

/// Supported badge anchor gravities for v1.
enum class BadgeGravity : uint8_t {
  kTopEnd,
  kTopStart,
};

/// Owner-supplied placement adjustment for badge layout.
///
/// Offsets are interpreted in local coordinates as motion toward the anchor
/// center.
struct BadgePlacement {
  BadgeGravity gravity = BadgeGravity::kTopEnd;
  int8_t horizontal_offset = 0;
  int8_t vertical_offset = 0;
};

/// Lightweight Material 3 badge adornment painted directly by its owner.
///
/// `Badge` is intentionally not a widget. Owners choose the anchor bounds,
/// call `layout()` when content or anchor geometry changes, and call `paint()`
/// at the point where the badge becomes the front-most local geometry.
class Badge {
 public:
  /// Total inline storage budget, including the terminating zero.
  static constexpr size_t kTextStorage = 8;

  /// Maximum number of visible bytes stored inline.
  static constexpr size_t kMaxVisibleText = kTextStorage - 1;

  /// Creates a hidden badge with no cached layout.
  Badge();

  /// Returns the current badge mode.
  BadgeMode mode() const;

  /// Returns true when the badge is visible.
  bool visible() const;

  /// Returns true when the badge is in text mode and stores non-empty text.
  bool hasText() const;

  /// Returns the stored inline badge text.
  roo::string_view text() const;

  /// Hides the badge and clears any cached layout.
  void hide();

  /// Shows a dot badge and clears any stored text.
  void setDot();

  /// Stores up to the first 7 visible bytes and switches to text mode.
  void setText(roo::string_view text);

  /// Clears the stored inline text and any cached layout.
  void clearText();

  /// Formats `number` as decimal text or `999+` and switches to text mode.
  void setValue(unsigned int number);

  /// Resolves and caches badge bounds relative to `anchor_bounds`.
  bool layout(const Rect& anchor_bounds, const BadgePlacement& placement = {});

  /// Returns the most recently cached local badge bounds.
  Rect bounds() const;

  /// Returns a conservative envelope for dot or text badge overflow.
  static Rect ConservativeBounds(const Rect& anchor_bounds,
                                 const BadgePlacement& placement,
                                 bool for_text_badge);

  /// Paints the badge interior, centered text, decoration, and exclusion.
  void paint(PaintContext& ctx, const Theme& theme) const;

 private:
  roo_collections::SmallString<kTextStorage> text_;
  Rect bounds_;
  uint8_t mode_ : 2;
  uint8_t valid_ : 1;
};

}  // namespace material3
}  // namespace roo_windows