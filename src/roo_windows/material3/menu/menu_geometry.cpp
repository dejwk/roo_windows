#include "roo_windows/material3/menu/menu_geometry.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <tuple>

namespace roo_windows::material3::internal {
namespace {

enum class Side : uint8_t { kBelow, kAbove, kAfter, kBefore };
enum class Alignment : uint8_t { kStart, kEnd };

struct Candidate {
  Rect bounds;
  Side side;
  Alignment alignment;
  uint8_t index;
};

int64_t VisibleArea(const Rect& candidate, const Rect& viewport) {
  Rect visible = Rect::Intersect(candidate, viewport);
  return visible.empty()
             ? 0
             : static_cast<int64_t>(visible.width()) * visible.height();
}

bool Fits(const Rect& candidate, const Rect& viewport) {
  return viewport.contains(candidate);
}

Rect BoundsFor(const Rect& anchor, int16_t width, int16_t height, Side side,
               Alignment alignment, LayoutDirection direction) {
  bool rtl = direction == LayoutDirection::kRightToLeft;
  int16_t start = rtl ? anchor.xMax() - width + 1 : anchor.xMin();
  int16_t end = rtl ? anchor.xMin() : anchor.xMax() - width + 1;
  int16_t x = alignment == Alignment::kStart ? start : end;
  int32_t y = anchor.yMax() + 1;
  switch (side) {
    case Side::kBelow:
      y = anchor.yMax() + 1;
      break;
    case Side::kAbove:
      y = anchor.yMin() - height;
      break;
    case Side::kAfter:
      x = rtl ? anchor.xMin() - width : anchor.xMax() + 1;
      y = alignment == Alignment::kStart ? anchor.yMin()
                                         : anchor.yMax() - height + 1;
      break;
    case Side::kBefore:
      x = rtl ? anchor.xMax() + 1 : anchor.xMin() - width;
      y = alignment == Alignment::kStart ? anchor.yMin()
                                         : anchor.yMax() - height + 1;
      break;
  }
  return Rect(x, y, x + width - 1, y + height - 1);
}

std::array<Candidate, 6> Candidates(const Rect& anchor, int16_t width,
                                    int16_t height, MenuPlacement preference,
                                    LayoutDirection direction) {
  std::array<std::pair<Side, Alignment>, 6> order;
  switch (preference) {
    case MenuPlacement::kBelowStart:
      order = {{{Side::kBelow, Alignment::kStart},
                {Side::kBelow, Alignment::kEnd},
                {Side::kAbove, Alignment::kStart},
                {Side::kAbove, Alignment::kEnd},
                {Side::kAfter, Alignment::kStart},
                {Side::kBefore, Alignment::kStart}}};
      break;
    case MenuPlacement::kBelowEnd:
      order = {{{Side::kBelow, Alignment::kEnd},
                {Side::kBelow, Alignment::kStart},
                {Side::kAbove, Alignment::kEnd},
                {Side::kAbove, Alignment::kStart},
                {Side::kAfter, Alignment::kStart},
                {Side::kBefore, Alignment::kStart}}};
      break;
    case MenuPlacement::kAboveStart:
      order = {{{Side::kAbove, Alignment::kStart},
                {Side::kAbove, Alignment::kEnd},
                {Side::kBelow, Alignment::kStart},
                {Side::kBelow, Alignment::kEnd},
                {Side::kAfter, Alignment::kStart},
                {Side::kBefore, Alignment::kStart}}};
      break;
    case MenuPlacement::kAboveEnd:
      order = {{{Side::kAbove, Alignment::kEnd},
                {Side::kAbove, Alignment::kStart},
                {Side::kBelow, Alignment::kEnd},
                {Side::kBelow, Alignment::kStart},
                {Side::kAfter, Alignment::kStart},
                {Side::kBefore, Alignment::kStart}}};
      break;
    case MenuPlacement::kBefore:
      order = {{{Side::kBefore, Alignment::kStart},
                {Side::kBefore, Alignment::kEnd},
                {Side::kAfter, Alignment::kStart},
                {Side::kAfter, Alignment::kEnd},
                {Side::kBelow, Alignment::kStart},
                {Side::kAbove, Alignment::kStart}}};
      break;
    case MenuPlacement::kAfter:
      order = {{{Side::kAfter, Alignment::kStart},
                {Side::kAfter, Alignment::kEnd},
                {Side::kBefore, Alignment::kStart},
                {Side::kBefore, Alignment::kEnd},
                {Side::kBelow, Alignment::kStart},
                {Side::kAbove, Alignment::kStart}}};
      break;
  }
  std::array<Candidate, 6> result;
  for (uint8_t i = 0; i < result.size(); ++i) {
    result[i] = {BoundsFor(anchor, width, height, order[i].first,
                           order[i].second, direction),
                 order[i].first, order[i].second, i};
  }
  return result;
}

}  // namespace

MenuPlacementResult ResolveRootMenuPlacement(const Rect& viewport,
                                             const Rect& anchor,
                                             Dimensions desired,
                                             MenuPlacement preference,
                                             LayoutDirection direction) {
  if (viewport.empty()) return {Rect(), false};
  int16_t width = std::max<int16_t>(
      1, std::min<int16_t>(desired.width(), viewport.width()));
  int16_t height = std::max<int16_t>(
      1, std::min<int32_t>(desired.height(), viewport.height()));
  auto candidates = Candidates(anchor, width, height, preference, direction);
  const Candidate* winner = &candidates[0];
  for (const Candidate& candidate : candidates) {
    auto score = [&](const Candidate& value) {
      return std::make_tuple(Fits(value.bounds, viewport),
                             value.side == candidates[0].side,
                             VisibleArea(value.bounds, viewport),
                             value.alignment == candidates[0].alignment,
                             -static_cast<int>(value.index));
    };
    if (score(candidate) > score(*winner)) winner = &candidate;
  }
  int16_t x = std::clamp<int16_t>(winner->bounds.xMin(), viewport.xMin(),
                                  viewport.xMax() - width + 1);
  int32_t y = std::clamp<int32_t>(winner->bounds.yMin(), viewport.yMin(),
                                  viewport.yMax() - height + 1);
  return {Rect(x, y, x + width - 1, y + height - 1), desired.height() > height};
}

SubmenuPlacementResult ResolveSubmenuPlacement(
    const Rect& viewport, const Rect& opener, const Rect& parent,
    Dimensions desired, int16_t min_width, int16_t gutter,
    LayoutDirection direction) {
  int16_t width = std::max<int16_t>(
      1, std::min<int16_t>(desired.width(), viewport.width()));
  int16_t height = std::max<int16_t>(
      1, std::min<int32_t>(desired.height(), viewport.height()));
  bool rtl = direction == LayoutDirection::kRightToLeft;
  int16_t after_space = rtl ? opener.xMin() - viewport.xMin() - gutter
                            : viewport.xMax() - opener.xMax() - gutter;
  int16_t before_space = rtl ? viewport.xMax() - opener.xMax() - gutter
                             : opener.xMin() - viewport.xMin() - gutter;
  bool use_after = after_space >= min_width;
  bool use_before = !use_after && before_space >= min_width;
  if (!use_after && !use_before) return {parent, false};
  int16_t x;
  if (use_after) {
    x = rtl ? opener.xMin() - gutter - width : opener.xMax() + gutter + 1;
  } else {
    x = rtl ? opener.xMax() + gutter + 1 : opener.xMin() - gutter - width;
  }
  x = std::clamp<int16_t>(x, viewport.xMin(), viewport.xMax() - width + 1);
  int32_t y = std::clamp<int32_t>(opener.yMin(), viewport.yMin(),
                                  viewport.yMax() - height + 1);
  return {Rect(x, y, x + width - 1, y + height - 1), true};
}

}  // namespace roo_windows::material3::internal
