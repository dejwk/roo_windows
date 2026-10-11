#include "roo_windows/material3/toolbar/toolbar.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "roo_display/color/color.h"
#include "roo_logging.h"
#include "roo_windows/material3/theme.h"
#include "roo_windows/material3/toolbar/toolbar_tokens.h"

namespace roo_windows::material3 {
namespace {

int SlotIndex(ToolbarSlot slot) {
  const int result = static_cast<int>(slot);
  CHECK(result >= 0 && result < 3);
  return result;
}

void CheckChild(const Container& parent, const Widget& child) {
  for (const Widget* ancestor = &parent; ancestor != nullptr;
       ancestor = ancestor->parent()) {
    CHECK(ancestor != &child);
  }
  CHECK(child.parent() == nullptr);
  CHECK(parent.sharesContextWith(child));
  const Margins margins = child.getMargins();
  CHECK(margins.left() >= 0 && margins.top() >= 0 && margins.right() >= 0 &&
        margins.bottom() >= 0);
}

XDim ClampWidth(int64_t value) {
  return static_cast<XDim>(std::min<int64_t>(
      std::max<int64_t>(0, value), std::numeric_limits<uint16_t>::max() >> 2));
}

YDim ClampHeight(int64_t value) {
  return static_cast<YDim>(std::min<int64_t>(
      std::max<int64_t>(0, value), std::numeric_limits<uint32_t>::max() >> 2));
}

// Converts an intersected half-open rectangle to the framework's inclusive
// right and bottom coordinates, leaving no area for a clipped-off child.
Rect ClippedChildRect(int64_t x, int64_t y, int64_t width, int64_t height,
                      const Rect& content) {
  const int64_t left = std::max<int64_t>(x, content.xMin());
  const int64_t top = std::max<int64_t>(y, content.yMin());
  const int64_t right = std::min<int64_t>(x + width, content.xMax() + 1LL);
  const int64_t bottom = std::min<int64_t>(y + height, content.yMax() + 1LL);
  if (right <= left || bottom <= top) return Rect(0, 0, -1, -1);
  return Rect(static_cast<XDim>(left), static_cast<YDim>(top),
              static_cast<XDim>(right - 1), static_cast<YDim>(bottom - 1));
}

}  // namespace

DockedToolbar::DockedToolbar(ApplicationContext& context)
    : Material3Container(context),
      color_style_(static_cast<uint8_t>(ToolbarColorStyle::kStandard)),
      arrangement_(
          static_cast<uint8_t>(DockedToolbarArrangement::kEvenlySpaced)),
      show_group_dividers_(false),
      direction_(static_cast<uint8_t>(LayoutDirection::kLeftToRight)) {}

DockedToolbar::~DockedToolbar() { clear(); }

void DockedToolbar::add(WidgetRef child, ToolbarSlot slot) {
  if (child.get() == nullptr) return;
  SlotIndex(slot);
  Widget* ptr = child.get();
  CheckChild(*this, *ptr);
  attachChild(std::move(child));
  children_.push_back({ptr, 0, 0, slot});
  requestLayout();
}

void DockedToolbar::clear() {
  if (children_.empty()) return;
  while (!children_.empty()) {
    Widget* child = children_.back().widget;
    children_.pop_back();
    detachChild(child);
  }
  requestLayout();
}

void DockedToolbar::clear(ToolbarSlot slot) {
  SlotIndex(slot);
  bool removed = false;
  for (size_t index = 0; index < children_.size();) {
    if (children_[index].slot != slot) {
      ++index;
      continue;
    }
    Widget* child = children_[index].widget;
    children_.erase(children_.begin() + index);
    detachChild(child);
    removed = true;
  }
  if (removed) requestLayout();
}

int DockedToolbar::childCount() const { return children_.size(); }

ToolbarColorStyle DockedToolbar::colorStyle() const {
  return static_cast<ToolbarColorStyle>(color_style_);
}

void DockedToolbar::setColorStyle(ToolbarColorStyle style) {
  if (colorStyle() == style) return;
  color_style_ = static_cast<uint8_t>(style);
  invalidateDescending();
  invalidateInterior();
}

DockedToolbarArrangement DockedToolbar::arrangement() const {
  return static_cast<DockedToolbarArrangement>(arrangement_);
}

void DockedToolbar::setArrangement(DockedToolbarArrangement value) {
  if (arrangement() == value) return;
  arrangement_ = static_cast<uint8_t>(value);
  requestLayout();
}

bool DockedToolbar::showsGroupDividers() const { return show_group_dividers_; }

void DockedToolbar::setShowGroupDividers(bool show) {
  if (showsGroupDividers() == show) return;
  show_group_dividers_ = show;
  invalidateInterior();
}

LayoutDirection DockedToolbar::layoutDirection() const {
  return static_cast<LayoutDirection>(direction_);
}

void DockedToolbar::setLayoutDirection(LayoutDirection direction) {
  if (layoutDirection() == direction) return;
  direction_ = static_cast<uint8_t>(direction);
  requestLayout();
}

ColorToken DockedToolbar::containerRole() const {
  return colorStyle() == ToolbarColorStyle::kVibrant
             ? ColorToken::kPrimaryContainer
             : theme().material3Theme().components.toolbar.standardContainer;
}

Padding DockedToolbar::getPadding() const {
  return Padding(Scaled(internal::kToolbarPaddingDp));
}

PreferredSize DockedToolbar::getPreferredSize() const {
  return PreferredSize(PreferredSize::MatchParentWidth(),
                       PreferredSize::WrapContentHeight());
}

int DockedToolbar::getChildrenCount() const { return childCount(); }

const Widget& DockedToolbar::getChild(int index) const {
  return *children_[index].widget;
}

Widget& DockedToolbar::getChild(int index) { return *children_[index].widget; }

int64_t DockedToolbar::itemMainExtent(const Entry& entry) const {
  const Margins margins = entry.widget->getMargins();
  return static_cast<int64_t>(entry.measured_width) + margins.left() +
         margins.right();
}

// Measures in structural order while accumulating logical slot widths. Gone
// entries retain their attachment but do not contribute an item or gap.
Dimensions DockedToolbar::onMeasure(WidthSpec width, HeightSpec height) {
  const int padding = Scaled(internal::kToolbarPaddingDp);
  const int item_gap = Scaled(internal::kToolbarItemGapDp);
  const int group_gap = Scaled(internal::kToolbarGroupGapDp);
  int64_t group_widths[3] = {0, 0, 0};
  int group_counts[3] = {0, 0, 0};
  int64_t max_cross = 0;

  for (Entry& entry : children_) {
    Widget& child = *entry.widget;
    entry.measured_width = 0;
    entry.measured_height = 0;
    if (child.isGone()) continue;
    const Margins margins = child.getMargins();
    CHECK(margins.left() >= 0 && margins.top() >= 0 && margins.right() >= 0 &&
          margins.bottom() >= 0);
    const PreferredSize preferred = child.getPreferredSize();
    const PreferredSize::Width preferred_width =
        preferred.width().isMatchParent() ? PreferredSize::WrapContentWidth()
                                          : preferred.width();
    const PreferredSize::Height preferred_height =
        preferred.height().isMatchParent() ? PreferredSize::WrapContentHeight()
                                           : preferred.height();
    const WidthSpec child_width = width.getChildWidthSpec(
        2 * padding + margins.left() + margins.right(), preferred_width);
    const HeightSpec child_height = height.getChildHeightSpec(
        2 * padding + margins.top() + margins.bottom(), preferred_height);
    const Dimensions measured = child.measure(child_width, child_height);
    entry.measured_width = measured.width();
    entry.measured_height = measured.height();
    const int slot = SlotIndex(entry.slot);
    if (group_counts[slot]++ > 0) group_widths[slot] += item_gap;
    group_widths[slot] += itemMainExtent(entry);
    max_cross = std::max<int64_t>(
        max_cross, measured.height() + margins.top() + margins.bottom());
  }

  int64_t packed = 0;
  int occupied_groups = 0;
  for (int slot = 0; slot < 3; ++slot) {
    if (group_counts[slot] == 0) continue;
    if (occupied_groups++ > 0) packed += group_gap;
    packed += group_widths[slot];
  }
  const XDim desired_width = ClampWidth(2LL * padding + packed);
  const YDim desired_height = ClampHeight(std::max<int64_t>(
      Scaled(internal::kToolbarMinimumCrossDp), 2LL * padding + max_cross));
  const XDim actual_width =
      width.kind() == UNSPECIFIED ? desired_width : width.value();
  const YDim actual_height = height.resolveSize(desired_height);

  // Cross-axis fill is resolved once after the toolbar itself has a height.
  for (Entry& entry : children_) {
    Widget& child = *entry.widget;
    if (child.isGone() || !child.getPreferredSize().height().isMatchParent()) {
      continue;
    }
    const Margins margins = child.getMargins();
    const YDim available =
        ClampHeight(static_cast<int64_t>(actual_height) - 2 * padding -
                    margins.top() - margins.bottom());
    const Dimensions measured =
        child.measure(WidthSpec::Exactly(entry.measured_width),
                      HeightSpec::Exactly(available));
    entry.measured_height = measured.height();
  }
  return Dimensions(actual_width, actual_height);
}

void DockedToolbar::computeGroups(GroupGeometry (&groups)[3], int64_t& packed,
                                  int& count) const {
  const int item_gap = Scaled(internal::kToolbarItemGapDp);
  const int group_gap = Scaled(internal::kToolbarGroupGapDp);
  packed = 0;
  count = 0;
  int occupied = 0;
  for (int slot = 0; slot < 3; ++slot) {
    for (const Entry& entry : children_) {
      if (SlotIndex(entry.slot) != slot || entry.widget->isGone()) continue;
      if (groups[slot].count++ > 0) groups[slot].width += item_gap;
      groups[slot].width += itemMainExtent(entry);
      ++count;
    }
    if (groups[slot].count == 0) continue;
    if (occupied++ > 0) packed += group_gap;
    packed += groups[slot].width;
  }
}

// Derives occupied group intervals without retaining a second child order.
void DockedToolbar::placeGroups(GroupGeometry (&groups)[3], int64_t packed,
                                int count) const {
  const int64_t padding = Scaled(internal::kToolbarPaddingDp);
  const int64_t inner =
      std::max<int64_t>(0, static_cast<int64_t>(width()) - 2 * padding);
  const int64_t free = std::max<int64_t>(0, inner - packed);
  const int64_t group_gap = Scaled(internal::kToolbarGroupGapDp);
  if (arrangement() == DockedToolbarArrangement::kEdgeAnchored &&
      packed <= inner) {
    groups[0].start = padding;
    groups[1].start = padding + (inner - groups[1].width) / 2;
    groups[2].start = padding + inner - groups[2].width;
    bool fits = true;
    int previous = -1;
    for (int slot = 0; slot < 3; ++slot) {
      if (groups[slot].count == 0) continue;
      fits = fits && groups[slot].width <= inner &&
             (previous < 0 || groups[slot].start >= groups[previous].start +
                                                        groups[previous].width +
                                                        group_gap);
      previous = slot;
    }
    if (fits) {
      for (GroupGeometry& group : groups) {
        group.end = group.start + group.width;
      }
      return;
    }
  }

  const bool evenly =
      arrangement() == DockedToolbarArrangement::kEvenlySpaced &&
      packed <= inner && count > 1;
  const int64_t base = padding + (packed <= inner && !evenly ? free / 2 : 0);
  int ordinal = 0;
  int64_t cursor = base;
  bool previous = false;
  for (GroupGeometry& group : groups) {
    if (group.count == 0) continue;
    if (previous) cursor += group_gap;
    group.start = cursor + (evenly ? ordinal * free / (count - 1) : 0);
    cursor += group.width;
    ordinal += group.count;
    group.end = cursor + (evenly ? (ordinal - 1) * free / (count - 1) : 0);
    previous = true;
  }
  if (count == 1 && packed <= inner &&
      arrangement() == DockedToolbarArrangement::kEvenlySpaced) {
    for (GroupGeometry& group : groups) {
      if (group.count == 0) continue;
      group.start += free / 2;
      group.end += free / 2;
    }
  }
}

void DockedToolbar::onLayout(bool changed, const Rect& rect) {
  GroupGeometry groups[3];
  int64_t packed = 0;
  int count = 0;
  computeGroups(groups, packed, count);
  placeGroups(groups, packed, count);
  const int64_t padding = Scaled(internal::kToolbarPaddingDp);
  const int64_t item_gap = Scaled(internal::kToolbarItemGapDp);
  const int64_t inner_height = std::max<int64_t>(0, height() - 2 * padding);
  const Rect content = ClippedChildRect(
      padding, padding, std::max<int64_t>(0, width() - 2 * padding),
      inner_height, bounds());
  int ordinal = 0;
  int64_t cursor[3] = {groups[0].start, groups[1].start, groups[2].start};
  const bool evenly =
      arrangement() == DockedToolbarArrangement::kEvenlySpaced &&
      packed <= std::max<int64_t>(0, width() - 2 * padding) && count > 1;
  const int64_t free = std::max<int64_t>(0, width() - 2 * padding - packed);
  for (int slot = 0; slot < 3; ++slot) {
    const int first_ordinal = ordinal;
    bool first = true;
    for (Entry& entry : children_) {
      if (SlotIndex(entry.slot) != slot) continue;
      Widget& child = *entry.widget;
      if (child.isGone()) {
        child.layout(Rect(0, 0, -1, -1));
        continue;
      }
      if (!first) cursor[slot] += item_gap;
      const Margins margins = child.getMargins();
      const int64_t box_width = itemMainExtent(entry);
      const int64_t box_height = static_cast<int64_t>(entry.measured_height) +
                                 margins.top() + margins.bottom();
      const int64_t logical_x =
          cursor[slot] + (evenly ? ordinal * free / (count - 1) -
                                       first_ordinal * free / (count - 1)
                                 : 0);
      const int64_t box_x =
          layoutDirection() == LayoutDirection::kRightToLeft
              ? static_cast<int64_t>(width()) - logical_x - box_width
              : logical_x;
      const int64_t box_y =
          padding + std::max<int64_t>(0, (inner_height - box_height) / 2);
      child.layout(ClippedChildRect(box_x + margins.left(),
                                    box_y + margins.top(), entry.measured_width,
                                    entry.measured_height, content));
      cursor[slot] += box_width;
      ++ordinal;
      first = false;
    }
  }
}

void DockedToolbar::paint(PaintContext& ctx) const {
  if (showsGroupDividers()) {
    GroupGeometry groups[3];
    int64_t packed = 0;
    int count = 0;
    computeGroups(groups, packed, count);
    placeGroups(groups, packed, count);
    const int64_t padding = Scaled(internal::kToolbarPaddingDp);
    const int64_t thickness = Scaled(internal::kToolbarDividerWidthDp);
    const int64_t available_height =
        std::max<int64_t>(0, height() - 2 * padding);
    const int64_t divider_height = std::min<int64_t>(
        Scaled(internal::kToolbarDividerHeightDp), available_height);
    const Rect content = ClippedChildRect(
        padding, padding, std::max<int64_t>(0, width() - 2 * padding),
        available_height, bounds());
    const ColorScheme& colors = theme().material3Theme().color;
    const roo_display::Color divider_color =
        colorStyle() == ToolbarColorStyle::kVibrant
            ? roo_display::AlphaBlend(background(),
                                      colors.onPrimaryContainer.withA(31))
            : colors.outlineVariant;
    int previous = -1;
    for (int slot = 0; slot < 3; ++slot) {
      if (groups[slot].count == 0) continue;
      if (previous >= 0 && thickness > 0 && divider_height > 0) {
        const int64_t gap = groups[slot].start - groups[previous].end;
        const int64_t logical_x = groups[previous].end + (gap - thickness) / 2;
        const int64_t x = layoutDirection() == LayoutDirection::kRightToLeft
                              ? width() - logical_x - thickness
                              : logical_x;
        const Rect divider = ClippedChildRect(
            x, padding + (available_height - divider_height) / 2, thickness,
            divider_height, content);
        if (!divider.empty()) {
          ctx.fillRect(divider, divider_color);
          ctx.addExclusion(divider);
        }
      }
      previous = slot;
    }
  }
  Container::paint(ctx);
}

}  // namespace roo_windows::material3
