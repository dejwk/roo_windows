#pragma once

#include <stdint.h>

#include "roo_windows/core/theme.h"

namespace roo_windows {
namespace material3 {

enum class SliderSize : uint8_t;

namespace internal {

struct SliderSizeMetrics {
  int16_t track_height;
  int16_t track_radius;
  int16_t track_handle_gap;
  int16_t handle_width;
  int16_t handle_height;
  bool supports_inset_icon;
  int16_t icon_size;

  constexpr float handleCornerRadius() const {
    return 0.5f * (float)handle_width;
  }
};

inline constexpr SliderSizeMetrics kExtraSmallSliderSizeMetrics = {
    Scaled(16), Scaled(8), Scaled(6), Scaled(4), Scaled(44), false, 0};

inline constexpr SliderSizeMetrics kSmallSliderSizeMetrics = {
    Scaled(24), Scaled(8), Scaled(6), Scaled(4), Scaled(44), false, 0};

inline constexpr SliderSizeMetrics kMediumSliderSizeMetrics = {
    Scaled(40), Scaled(12), Scaled(6), Scaled(4), Scaled(52), true, Scaled(24)};

inline constexpr SliderSizeMetrics kLargeSliderSizeMetrics = {
    Scaled(56), Scaled(16), Scaled(6), Scaled(4), Scaled(68), true, Scaled(24)};

inline constexpr SliderSizeMetrics kExtraLargeSliderSizeMetrics = {
    Scaled(96),  Scaled(28), Scaled(6), Scaled(4),
    Scaled(108), true,       Scaled(32)};

inline constexpr const SliderSizeMetrics& ResolveSliderSizeMetrics(
    SliderSize size) {
  switch (size) {
    case SliderSize::kExtraSmall:
      return kExtraSmallSliderSizeMetrics;
    case SliderSize::kSmall:
      return kSmallSliderSizeMetrics;
    case SliderSize::kMedium:
      return kMediumSliderSizeMetrics;
    case SliderSize::kLarge:
      return kLargeSliderSizeMetrics;
    case SliderSize::kExtraLarge:
      return kExtraLargeSliderSizeMetrics;
  }
  return kExtraSmallSliderSizeMetrics;
}

}  // namespace internal
}  // namespace material3
}  // namespace roo_windows