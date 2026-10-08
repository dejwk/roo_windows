#pragma once

#include "roo_windows/material3/list/list.h"

namespace roo_windows::material3::internal {

// Ephemeral row tokens. Appended-body spacing remains at its original value.
struct ListRowTokens {
  int16_t horizontal_padding;
  int16_t vertical_padding;
  int16_t slot_gap;
  int16_t body_gap;
  int16_t body_bottom_padding;
};

struct ListTextSlotMetrics {
  int16_t width;
  YDim height;
  uint8_t line_count;
};

struct ListRowLayoutMetrics {
  Dimensions leading;
  Dimensions trailing;
  Dimensions body;
  ListTextSlotMetrics text;
  int16_t width;
  YDim height;
  YDim main_height;
  YDim row_band_height;
  int16_t text_x;
  YDim text_y;
  int16_t text_width;
  int16_t leading_x;
  YDim leading_y;
  int16_t trailing_x;
  YDim trailing_y;
  int16_t body_x;
  YDim body_y;
  int16_t body_width;
};

// Actual occupied slot dimensions, including margins, from measurement.
struct ListRowGeometryInput {
  ListVariant variant;
  uint8_t line_count;
  bool top_text;
  VerticalVisualAlignment leading_alignment;
  VerticalVisualAlignment trailing_alignment;
  YDim text_height;
  Dimensions leading;
  Dimensions trailing;
  Dimensions body;
  YDim minimum_band_height = 0;
  YDim extra_content_height = 0;
};

// Shared vertical geometry for natural measurement and child placement.
struct ListRowGeometry {
  YDim band_height;
  YDim height;
  YDim text_y;
  YDim leading_y;
  YDim trailing_y;
  YDim body_y;
};

/// Computes cheap descriptor metrics without measuring attached children.
ListTextSlotMetrics ResolveListTextSlotMetrics(const ListItem* item);

/// Measures a row's actual slots and resolves geometry at a valid level [-5,
/// 0]. Zero retains the legacy descriptor text budget. Compact levels measure
/// the attached text slots at their final width before applying the content
/// floor. Optional band/content floors include specialized row tokens and
/// owner-painted content without applying a second height clamp afterward.
ListRowLayoutMetrics ResolveListRowLayout(ListEntry& entry, WidthSpec width,
                                          HeightSpec height, int8_t level,
                                          YDim minimum_band_height = 0,
                                          YDim extra_content_height = 0);

/// Places the same measured text, control, and body slots used by the resolver.
void LayoutListRow(ListEntry& entry, const ListRowLayoutMetrics& layout);

/// Resolves row padding for valid signed levels [-5, 0]; other tokens stay
/// fixed.
ListRowTokens ResolveListRowTokens(ListVariant variant, int8_t level);

/// Reduces only the 56/72/88 dp band, then applies actual slot content floors.
/// Appended body content retains its gap and bottom padding. Alignment is based
/// on the natural band even when a parent later imposes tighter constraints.
ListRowGeometry ResolveListRowGeometry(const ListRowGeometryInput& input,
                                       int8_t level);

}  // namespace roo_windows::material3::internal
