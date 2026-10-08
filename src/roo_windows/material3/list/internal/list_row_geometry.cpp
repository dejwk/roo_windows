#include "roo_windows/material3/list/internal/list_row_geometry.h"

#include <algorithm>

#include "roo_windows/core/child_layout.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows::material3::internal {
namespace {

int16_t TextWidth(const TextStyle& style, roo::string_view text) {
  if (text.empty()) return 0;
  return style.font()
      .getHorizontalStringMetrics(text, style.fontOptions())
      .advance();
}

uint8_t SlotLineCount(roo::string_view text, ListTextPolicy policy) {
  if (text.empty()) return 0;
  return std::max<uint8_t>(1, policy.max_lines);
}

Dimensions MeasureChild(Widget* child, WidthSpec width, HeightSpec height) {
  if (child == nullptr || child->isGone()) return Dimensions(0, 0);
  return MeasureChildWithMargins(*child, width, height);
}

int16_t ConstrainWidth(int16_t desired, WidthSpec spec) {
  switch (spec.kind()) {
    case UNSPECIFIED:
      return desired;
    case AT_MOST:
      return std::min<YDim>(desired, spec.value());
    case EXACTLY:
      return spec.value();
  }
  return desired;
}

YDim ConstrainHeight(YDim desired, HeightSpec spec) {
  switch (spec.kind()) {
    case UNSPECIFIED:
      return desired;
    case AT_MOST:
      return std::min<YDim>(desired, spec.value());
    case EXACTLY:
      return spec.value();
  }
  return desired;
}

// Measures and places one text slot, returning its occupied height.
YDim LayoutTextSlot(Widget* slot, XDim x, YDim y, XDim max_width) {
  if (slot == nullptr || slot->isGone()) return 0;
  if (max_width <= 0) {
    LayoutChildWithMargins(*slot, Rect(0, 0, -1, -1));
    return 0;
  }
  Dimensions measured = MeasureChildWithMargins(
      *slot, WidthSpec::AtMost(max_width), HeightSpec::Unspecified(0));
  int16_t slot_width = std::min<int16_t>(measured.width(), max_width);
  if (slot_width <= 0 || measured.height() <= 0) {
    LayoutChildWithMargins(*slot, Rect(0, 0, -1, -1));
    return 0;
  }
  LayoutChildWithMargins(
      *slot, Rect(x, y, x + slot_width - 1, y + measured.height() - 1));
  return measured.height();
}

YDim MiddleOffset(YDim outer, YDim inner) {
  return outer <= inner ? 0 : (outer - inner) / 2;
}

// Top-aligned slots share the band's resolved edge; other slots are centered.
YDim SlotY(bool top_text, VerticalVisualAlignment alignment, YDim band,
           YDim slot, int padding) {
  return top_text || alignment == VerticalVisualAlignment::kTop
             ? padding
             : MiddleOffset(band, slot);
}

}  // namespace

// Measures the descriptor-driven text stack without depending on any row-owned
// child widget state.
ListTextSlotMetrics ResolveListTextSlotMetrics(const ListItem* item) {
  if (item == nullptr) return ListTextSlotMetrics{0, 0, 0};

  uint8_t overline_lines =
      SlotLineCount(item->overlineText(), item->overlinePolicy());
  uint8_t headline_lines =
      SlotLineCount(item->headlineText(), item->headlinePolicy());
  uint8_t supporting_lines =
      SlotLineCount(item->supportingText(), item->supportingPolicy());

  int16_t width = 0;
  width = std::max(width,
                   TextWidth(text_style_label_small(), item->overlineText()));
  width =
      std::max(width, TextWidth(text_style_body_large(), item->headlineText()));
  width = std::max(width,
                   TextWidth(text_style_body_medium(), item->supportingText()));

  YDim height = overline_lines * text_style_label_small().lineHeight() +
                headline_lines * text_style_body_large().lineHeight() +
                supporting_lines * text_style_body_medium().lineHeight();
  return ListTextSlotMetrics{
      width, height,
      static_cast<uint8_t>(std::min<int>(
          255, overline_lines + headline_lines + supporting_lines))};
}

ListRowTokens ResolveListRowTokens(ListVariant variant, int8_t level) {
  DCHECK_GE(level, -5);
  DCHECK_LE(level, 0);
  int base = variant == ListVariant::kBaseline ? 8 : 10;
  return {
      Scaled(16),
      static_cast<int16_t>(
          Scaled(std::max(4, base + 2 * static_cast<int>(level)))),
      static_cast<int16_t>(Scaled(variant == ListVariant::kBaseline ? 16 : 12)),
      Scaled(8), static_cast<int16_t>(Scaled(base))};
}

ListRowGeometry ResolveListRowGeometry(const ListRowGeometryInput& input,
                                       int8_t level) {
  const ListRowTokens tokens = ResolveListRowTokens(input.variant, level);
  int base = input.top_text || input.line_count >= 3 ? 88
             : input.line_count == 2                 ? 72
                                                     : 56;
  YDim minimum = Scaled(std::max(36, base + 4 * static_cast<int>(level)));
  YDim content = std::max(
      {input.text_height, input.leading.height(), input.trailing.height()});
  YDim band = std::max(minimum, content + 2 * tokens.vertical_padding);
  bool has_body = input.body.width() > 0 || input.body.height() > 0;
  YDim body_y = band + (has_body ? tokens.body_gap : 0);
  YDim height = has_body
                    ? body_y + input.body.height() + tokens.body_bottom_padding
                    : band;
  return {band,
          height,
          input.top_text ? tokens.vertical_padding
                         : MiddleOffset(band, input.text_height),
          SlotY(input.top_text, input.leading_alignment, band,
                input.leading.height(), tokens.vertical_padding),
          SlotY(input.top_text, input.trailing_alignment, band,
                input.trailing.height(), tokens.vertical_padding),
          body_y};
}

// Resolves one shared row geometry so measurement and child layout agree on
// the same slot positions and text bounds.
ListRowLayoutMetrics ResolveListRowLayout(ListEntry& entry,
                                          WidthSpec width_spec,
                                          HeightSpec height_spec,
                                          int8_t level) {
  ListItem* item = entry.item_;
  const ListRowTokens tokens =
      internal::ResolveListRowTokens(entry.visual_context_.variant, level);

  Dimensions leading =
      MeasureChild(item == nullptr ? nullptr : item->leading(),
                   WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  Dimensions trailing =
      MeasureChild(item == nullptr ? nullptr : item->trailing(),
                   WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  ListTextSlotMetrics text = ResolveListTextSlotMetrics(item);

  bool has_leading = leading.width() > 0 || leading.height() > 0;
  bool has_trailing = trailing.width() > 0 || trailing.height() > 0;
  bool has_text = text.width > 0 || text.height > 0;

  int16_t horizontal_gaps = 0;
  if (has_leading && has_text) horizontal_gaps += tokens.slot_gap;
  if (has_trailing && (has_text || has_leading))
    horizontal_gaps += tokens.slot_gap;

  int16_t desired_main_width = tokens.horizontal_padding * 2 + leading.width() +
                               trailing.width() + text.width + horizontal_gaps;

  Dimensions body =
      MeasureChild(item == nullptr ? nullptr : item->body(),
                   WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  bool has_body = body.width() > 0 || body.height() > 0;
  int16_t desired_body_width =
      has_body ? tokens.horizontal_padding * 2 +
                     std::max<int16_t>(body.width(), text.width)
               : 0;
  int16_t desired_width = std::max(desired_main_width, desired_body_width);
  int16_t resolved_width = ConstrainWidth(desired_width, width_spec);

  // The text column expands into whatever horizontal space remains between the
  // fixed leading and trailing slots.
  int16_t content_width =
      std::max<int16_t>(0, resolved_width - 2 * tokens.horizontal_padding);
  int16_t text_x = tokens.horizontal_padding;
  if (has_leading) text_x += leading.width() + (has_text ? tokens.slot_gap : 0);
  int16_t trailing_x =
      resolved_width - tokens.horizontal_padding - trailing.width();
  int16_t text_right = has_trailing
                           ? trailing_x - tokens.slot_gap - 1
                           : resolved_width - tokens.horizontal_padding - 1;
  int16_t text_width =
      has_text ? std::max<int16_t>(0, text_right - text_x + 1) : 0;

  if (level != 0) {
    // Compact floors use the same actual text slots placed by onLayout().
    // Zero retains the existing descriptor-based height budget exactly.
    text.height = 0;
    for (Widget* slot :
         {entry.overline_text_, entry.headline_text_, entry.supporting_text_}) {
      text.height += MeasureChild(slot, WidthSpec::AtMost(text_width),
                                  HeightSpec::Unspecified(0))
                         .height();
    }
  }
  int16_t body_width = has_body ? std::max<int16_t>(0, content_width) : 0;
  if (has_body && body_width != body.width()) {
    // Re-measure the optional body at the resolved content width so stacked
    // body content and row height stay consistent with the final row width.
    body = MeasureChild(item->body(), WidthSpec::Exactly(body_width),
                        HeightSpec::Unspecified(0));
  }
  internal::ListRowGeometryInput input{
      entry.visual_context_.variant,
      text.line_count,
      item != nullptr && item->preferTopTextAlignment(),
      item == nullptr ? VerticalVisualAlignment::kMiddle
                      : item->leadingAlignment(),
      item == nullptr ? VerticalVisualAlignment::kMiddle
                      : item->trailingAlignment(),
      text.height,
      leading,
      trailing,
      body};
  const internal::ListRowGeometry geometry =
      internal::ResolveListRowGeometry(input, level);
  YDim resolved_height = ConstrainHeight(geometry.height, height_spec);
  YDim main_content_height =
      std::max({text.height, leading.height(), trailing.height()});

  return ListRowLayoutMetrics{leading,
                              trailing,
                              body,
                              text,
                              resolved_width,
                              resolved_height,
                              main_content_height,
                              geometry.band_height,
                              text_x,
                              geometry.text_y,
                              text_width,
                              tokens.horizontal_padding,
                              geometry.leading_y,
                              trailing_x,
                              geometry.trailing_y,
                              tokens.horizontal_padding,
                              geometry.body_y,
                              body_width};
}

void LayoutListRow(ListEntry& entry, const ListRowLayoutMetrics& layout) {
  YDim text_y = layout.text_y;
  text_y += LayoutTextSlot(entry.overline_text_, layout.text_x, text_y,
                           layout.text_width);
  text_y += LayoutTextSlot(entry.headline_text_, layout.text_x, text_y,
                           layout.text_width);
  LayoutTextSlot(entry.supporting_text_, layout.text_x, text_y,
                 layout.text_width);

  if (entry.leading_child_ != nullptr && !entry.leading_child_->isGone()) {
    LayoutChildWithMargins(
        *entry.leading_child_,
        Rect(layout.leading_x, layout.leading_y,
             layout.leading_x + layout.leading.width() - 1,
             layout.leading_y + layout.leading.height() - 1));
  }
  if (entry.trailing_child_ != nullptr && !entry.trailing_child_->isGone()) {
    LayoutChildWithMargins(
        *entry.trailing_child_,
        Rect(layout.trailing_x, layout.trailing_y,
             layout.trailing_x + layout.trailing.width() - 1,
             layout.trailing_y + layout.trailing.height() - 1));
  }
  if (entry.body_child_ != nullptr && !entry.body_child_->isGone()) {
    LayoutChildWithMargins(*entry.body_child_,
                           Rect(layout.body_x, layout.body_y,
                                layout.body_x + layout.body.width() - 1,
                                layout.body_y + layout.body.height() - 1));
  }
}

}  // namespace roo_windows::material3::internal
