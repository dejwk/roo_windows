#include "roo_windows/material3/text_field/internal/text_field_geometry.h"

#include <algorithm>

#include "roo_windows/core/theme.h"

namespace roo_windows::material3::internal {
namespace {

Rect Empty() { return Rect(0, 0, -1, -1); }

}  // namespace

int ResolveTextFieldContainerHeight(bool outlined,
                                    const TextFieldMetrics& metrics,
                                    int8_t level) {
  DCHECK_GE(level, -5);
  DCHECK_LE(level, 0);
  if (level == 0) return Scaled(56);
  int target = Scaled(std::max(36, 56 + 4 * static_cast<int>(level)));
  int text_height = metrics.body_height + (outlined ? 0 : metrics.small_height);
  int content =
      std::max({text_height, metrics.leading_height, metrics.trailing_height});
  return std::max(target, content + 2 * Scaled(4));
}

int ResolveTextFieldNaturalHeight(bool outlined, bool assistive,
                                  const TextFieldMetrics& metrics,
                                  int8_t level) {
  return ResolveTextFieldContainerHeight(outlined, metrics, level) +
         (outlined ? metrics.small_height / 2 : 0) +
         (assistive ? Scaled(4) + metrics.small_height : 0);
}

TextFieldSlots ResolveTextFieldSlots(const TextFieldSlotInput& input,
                                     int8_t level) {
  const int container_height =
      ResolveTextFieldContainerHeight(input.outlined, input.metrics, level);
  // An outlined floating label straddles the top stroke, so reserve half of
  // its line height above the resolved container.
  int top = input.outlined ? input.metrics.small_height / 2 : 0;
  TextFieldSlots s;
  s.container =
      Rect(0, top, input.bounds.width() - 1, top + container_height - 1);

  // First reserve the edge affordances, then assign the remaining span to the
  // label, prefix, editable viewport, and suffix.
  int left = input.metrics.leading_height > 0 ? Scaled(12) : Scaled(16);
  int right = input.bounds.width() -
              (input.metrics.trailing_height > 0 ? Scaled(12) : Scaled(16));
  int leading_height =
      level == 0 ? ROO_WINDOWS_ICON_SIZE : input.metrics.leading_height;
  int trailing_height =
      level == 0 ? ROO_WINDOWS_ICON_SIZE : input.metrics.trailing_height;
  int iy = top + (container_height - leading_height) / 2;
  s.leading =
      input.metrics.leading_height > 0
          ? Rect(left, iy, std::min(right, left + ROO_WINDOWS_ICON_SIZE) - 1,
                 iy + leading_height - 1)
          : Empty();
  if (input.metrics.leading_height > 0) {
    left = std::min(right, left + ROO_WINDOWS_ICON_SIZE + Scaled(16));
  }
  iy = top + (container_height - trailing_height) / 2;
  s.trailing = input.metrics.trailing_height > 0
                   ? Rect(std::max(left, right - ROO_WINDOWS_ICON_SIZE), iy,
                          right - 1, iy + trailing_height - 1)
                   : Empty();
  if (input.metrics.trailing_height > 0) {
    right = std::max(left, right - ROO_WINDOWS_ICON_SIZE - Scaled(16));
  }
  bool floating = input.floating;
  int ty = top + (container_height - input.metrics.body_height) / 2;
  if (floating && !input.outlined) {
    ty = top +
         (container_height - input.metrics.body_height -
          input.metrics.small_height) /
             2 +
         input.metrics.small_height;
  }
  if (floating) {
    int ly = input.outlined ? 0 : ty - input.metrics.small_height;
    int lw = std::min(std::max(0, right - left), input.label_width);
    s.label =
        Rect(left, ly, left + lw - 1, ly + input.metrics.small_height - 1);
  } else {
    s.label = Rect(left, ty, right - 1, ty + input.metrics.body_height - 1);
  }
  int pw =
      floating ? std::min(std::max(0, right - left), input.prefix_width) : 0;
  s.prefix = Rect(left, ty, left + pw - 1, ty + input.metrics.body_height - 1);
  left += pw;
  int sw =
      floating ? std::min(std::max(0, right - left), input.suffix_width) : 0;
  s.suffix =
      Rect(right - sw, ty, right - 1, ty + input.metrics.body_height - 1);
  right -= sw;
  s.viewport = Rect(left, ty, right - 1, ty + input.metrics.body_height - 1);
  s.assist =
      Rect(Scaled(16), top + container_height + Scaled(4),
           input.bounds.width() - Scaled(16) - 1,
           top + container_height + Scaled(4) + input.metrics.small_height - 1);
  if (input.rtl) {
    // Compute logical slots left-to-right once, then mirror their physical
    // rectangles without changing the UTF-8 value or prefix/suffix roles.
    auto mirror = [&](Rect& r) {
      r = Rect(input.bounds.width() - 1 - r.xMax(), r.yMin(),
               input.bounds.width() - 1 - r.xMin(), r.yMax());
    };
    mirror(s.leading);
    mirror(s.trailing);
    mirror(s.label);
    mirror(s.prefix);
    mirror(s.suffix);
    mirror(s.viewport);
  }
  // Tiny layouts can make slots overlap or invert; clipping leaves painting
  // with empty rectangles instead of coordinates outside the widget.
  auto clip = [&](Rect& r) {
    r = Rect::Intersect(
        r, Rect(0, 0, input.bounds.width() - 1, input.bounds.height() - 1));
  };
  clip(s.container);
  clip(s.label);
  clip(s.viewport);
  clip(s.prefix);
  clip(s.suffix);
  clip(s.leading);
  clip(s.trailing);
  clip(s.assist);
  return s;
}

}  // namespace roo_windows::material3::internal
