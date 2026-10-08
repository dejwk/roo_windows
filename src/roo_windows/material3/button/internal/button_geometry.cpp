#include "roo_windows/material3/button/internal/button_geometry.h"

#include <algorithm>

#include "roo_windows/material3/typography.h"

namespace roo_windows::material3::internal {
namespace {

// Per-size geometry tokens transcribed from the Material 3 button spec.
constexpr ButtonGeometryTokens kButtonGeometryTokens[] = {
    {32, 12, 20, 4, 12, 8},    {40, 16, 24, 8, 12, 8},
    {56, 24, 24, 8, 16, 12},   {96, 48, 32, 12, 28, 16},
    {136, 64, 40, 16, 28, 16},
};

// Small buttons have an extra configuration knob in the spec: the same height
// can be paired with either the default or reduced horizontal padding.
int ButtonHorizontalPaddingDpFor(ButtonSize size,
                                 SmallButtonPadding small_button_padding) {
  if (size == ButtonSize::kSmall) {
    return small_button_padding == SmallButtonPadding::kDefault ? 24 : 16;
  }
  return ButtonGeometryTokensFor(size).horizontal_padding_dp;
}

// Height needed to keep all painted pixels around the original anchor center.
// Using the tighter of the two margins protects off-center artwork and even
// artwork extending outside its anchor box, without scanning or allocating.
int IconVerticalFootprint(const MonoIcon& icon) {
  roo_display::Box ink = icon.extents();
  if (ink.empty()) return 0;
  roo_display::Box anchor = icon.anchorExtents();
  int clear_top = ink.yMin() - anchor.yMin();
  int clear_bottom = anchor.yMax() - ink.yMax();
  return std::max(0, anchor.height() - 2 * std::min(clear_top, clear_bottom));
}

const TextStyle& ButtonTextStyle() { return text_style_label_large(); }

}  // namespace

const ButtonGeometryTokens& ButtonGeometryTokensFor(ButtonSize size) {
  return kButtonGeometryTokens[static_cast<uint8_t>(size)];
}

// Measures the content block without widget padding. For icons, keep at least
// the token slot size so the size tables remain stable even if the concrete
// drawable is smaller than the Material 3 target.
ButtonContentMetrics ResolveButtonContentMetrics(roo::string_view label,
                                                 const MonoIcon* icon,
                                                 ButtonSize size,
                                                 int8_t level) {
  DCHECK_GE(level, -5);
  DCHECK_LE(level, 0);
  const ButtonGeometryTokens& geometry = ButtonGeometryTokensFor(size);
  const TextStyle& style = ButtonTextStyle();
  const roo_display::Font& font = style.font();
  int16_t text_width = 0;
  int16_t text_height = 0;
  if (!label.empty()) {
    text_width =
        font.getHorizontalStringMetrics(label, style.fontOptions()).advance();
    text_height = style.lineHeight();
  }
  int16_t icon_slot_width = 0;
  int16_t icon_slot_height = 0;
  if (icon != nullptr) {
    int16_t token_icon_size = Scaled(geometry.icon_size_dp);
    icon_slot_width = token_icon_size;
    icon_slot_height = token_icon_size;
    icon_slot_width =
        std::max<int16_t>(icon_slot_width, icon->anchorExtents().width());
    icon_slot_height =
        std::max<int16_t>(icon_slot_height, icon->anchorExtents().height());
  }
  int16_t gap =
      (icon != nullptr && text_width > 0) ? Scaled(geometry.icon_gap_dp) : 0;
  int16_t content_width = text_width;
  if (icon != nullptr) {
    content_width = icon_slot_width;
    if (text_width > 0) {
      content_width += gap + text_width;
    }
  }
  int icon_content_height = icon_slot_height;
  if (level != 0 && icon != nullptr) {
    icon_content_height = IconVerticalFootprint(*icon);
  }
  int16_t content_height = std::max<int>(text_height, icon_content_height);
  return ButtonContentMetrics{text_width,       text_height, icon_slot_width,
                              icon_slot_height, gap,         content_width,
                              content_height};
}

Padding ResolveButtonPadding(ButtonSize size, SmallButtonPadding small_padding,
                             int content_height, int8_t level) {
  DCHECK_GE(level, -5);
  DCHECK_LE(level, 0);
  int horizontal = Scaled(ButtonHorizontalPaddingDpFor(size, small_padding));
  const ButtonGeometryTokens& tokens = ButtonGeometryTokensFor(size);
  // Scale a signed intermediate: the extra-large 136dp height exceeds a byte
  // at 200% zoom. Ordinary level-zero rounding remains unchanged.
  int target = level == 0
                   ? Scaled(static_cast<int>(tokens.height_dp))
                   : Scaled(std::max(24, static_cast<int>(tokens.height_dp) +
                                             4 * static_cast<int>(level)));
  if (level != 0) {
    // An odd target-content difference rounds down to symmetric padding.
    // Raise the target before that division so both edges retain their floor.
    target = std::max(target, content_height + 2 * Scaled(4));
  }
  // Suggested minimum dimensions exclude padding; these symmetric edges
  // supply the remaining natural height around the measured content block.
  int vertical = std::max(0, (target - content_height) / 2);
  // Padding uses biased byte storage. Clamp signed values before narrowing.
  return Padding(std::min(horizontal, 223), std::min(vertical, 223));
}

uint8_t ResolveButtonCornerRadius(ButtonSize size, ButtonShape shape,
                                  bool pressed, Dimensions measured) {
  const ButtonGeometryTokens& tokens = ButtonGeometryTokensFor(size);
  int half =
      std::max(0, std::min<int>(measured.width(), measured.height()) / 2);
  int radius = pressed ? Scaled(tokens.pressed_corner_radius_dp)
               : shape == ButtonShape::kRound
                   ? half
                   : Scaled(tokens.square_corner_radius_dp);
  return static_cast<uint8_t>(std::min({radius, half, 255}));
}

}  // namespace roo_windows::material3::internal
