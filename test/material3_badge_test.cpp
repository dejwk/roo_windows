#include <string>

#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/badge/badge.h"

namespace roo_windows {
namespace material3 {
namespace {

std::string ToStdString(roo::string_view text) {
  if (text.empty()) return std::string();
  return std::string(text.data(), text.size());
}

std::string RectToString(const Rect& rect) {
  return "[" + std::to_string(rect.xMin()) + ", " +
         std::to_string(rect.yMin()) + ", " + std::to_string(rect.xMax()) +
         ", " + std::to_string(rect.yMax()) + "]";
}

// Verifies that Badge starts hidden, reports no text, and exposes no cached
// bounds before an owner supplies a layout anchor.
TEST(Material3Badge, StartsHiddenWithoutBounds) {
  Badge badge;

  EXPECT_EQ(BadgeMode::kHidden, badge.mode());
  EXPECT_FALSE(badge.visible());
  EXPECT_FALSE(badge.hasText());
  EXPECT_TRUE(badge.bounds().empty());
}

// Verifies that setText() switches to text mode and clamps the inline label to
// the 7 visible bytes that fit in the fixed-capacity storage.
TEST(Material3Badge, SetTextClampsToInlineCapacity) {
  Badge badge;

  badge.setText("abcdefghijk");

  EXPECT_EQ(BadgeMode::kText, badge.mode());
  EXPECT_TRUE(badge.visible());
  EXPECT_TRUE(badge.hasText());
  EXPECT_EQ("abcdefg", ToStdString(badge.text()));
}

// Verifies that the convenience number formatter renders decimal values up to
// 999 and caps larger values at the Material-style 999+ label.
TEST(Material3Badge, SetValueFormatsAndCapsAt999Plus) {
  Badge badge;

  badge.setValue(0);
  EXPECT_EQ("0", ToStdString(badge.text()));

  badge.setValue(7);
  EXPECT_EQ("7", ToStdString(badge.text()));

  badge.setValue(42);
  EXPECT_EQ("42", ToStdString(badge.text()));

  badge.setValue(999);
  EXPECT_EQ("999", ToStdString(badge.text()));

  badge.setValue(1000);
  EXPECT_EQ("999+", ToStdString(badge.text()));
}

// Verifies that the explicit mode setters move between dot, text, and hidden
// states without leaving stale inline text behind.
TEST(Material3Badge, ModeTransitionsClearStaleText) {
  Badge badge;

  badge.setText("7");
  EXPECT_EQ(BadgeMode::kText, badge.mode());
  EXPECT_TRUE(badge.hasText());

  badge.setDot();
  EXPECT_EQ(BadgeMode::kDot, badge.mode());
  EXPECT_TRUE(badge.visible());
  EXPECT_FALSE(badge.hasText());
  EXPECT_EQ("", ToStdString(badge.text()));

  badge.hide();
  EXPECT_EQ(BadgeMode::kHidden, badge.mode());
  EXPECT_FALSE(badge.visible());
  EXPECT_FALSE(badge.hasText());
}

// Verifies that top-end text badges keep their outer right edge stable as the
// label grows, so wider text expands away from that fixed edge.
TEST(Material3Badge, TopEndLayoutKeepsOuterEdgeStable) {
  Badge badge;
  Rect anchor(10, 20, 33, 43);

  badge.setText("1");
  ASSERT_TRUE(badge.layout(anchor));
  Rect short_bounds = badge.bounds();

  badge.setText("1234567");
  ASSERT_TRUE(badge.layout(anchor));
  Rect long_bounds = badge.bounds();

  EXPECT_EQ(short_bounds.xMax(), long_bounds.xMax());
  EXPECT_LE(long_bounds.xMin(), short_bounds.xMin());
}

// Verifies that top-start text badges keep their outer left edge stable as the
// label grows, so wider text expands away from that fixed edge.
TEST(Material3Badge, TopStartLayoutKeepsOuterEdgeStable) {
  Badge badge;
  Rect anchor(10, 20, 33, 43);

  badge.setText("1");
  ASSERT_TRUE(
      badge.layout(anchor, BadgePlacement{BadgeGravity::kTopStart, 0, 0}));
  Rect short_bounds = badge.bounds();

  badge.setText("1234567");
  ASSERT_TRUE(
      badge.layout(anchor, BadgePlacement{BadgeGravity::kTopStart, 0, 0}));
  Rect long_bounds = badge.bounds();

  EXPECT_EQ(short_bounds.xMin(), long_bounds.xMin());
  EXPECT_GE(long_bounds.xMax(), short_bounds.xMax());
}

// Verifies that placement offsets move the badge toward the anchor center:
// top-end badges move left/down and top-start badges move right/down.
TEST(Material3Badge, PlacementOffsetsMoveTowardAnchorCenter) {
  Rect anchor(10, 20, 33, 43);

  Badge top_end;
  top_end.setDot();
  ASSERT_TRUE(top_end.layout(anchor));
  Rect top_end_default = top_end.bounds();
  ASSERT_TRUE(
      top_end.layout(anchor, BadgePlacement{BadgeGravity::kTopEnd, 2, 3}));
  Rect top_end_offset = top_end.bounds();
  EXPECT_EQ(top_end_default.xMax() - 2, top_end_offset.xMax());
  EXPECT_EQ(top_end_default.yMin() + 3, top_end_offset.yMin());

  Badge top_start;
  top_start.setDot();
  ASSERT_TRUE(
      top_start.layout(anchor, BadgePlacement{BadgeGravity::kTopStart, 0, 0}));
  Rect top_start_default = top_start.bounds();
  ASSERT_TRUE(
      top_start.layout(anchor, BadgePlacement{BadgeGravity::kTopStart, 2, 3}));
  Rect top_start_offset = top_start.bounds();
  EXPECT_EQ(top_start_default.xMin() + 2, top_start_offset.xMin());
  EXPECT_EQ(top_start_default.yMin() + 3, top_start_offset.yMin());
}

// Verifies that ConservativeBounds() envelopes both dot badges and the widest
// 7-byte text badge layout for each supported gravity.
TEST(Material3Badge, ConservativeBoundsContainDotAndTextLayouts) {
  Rect anchor(10, 20, 33, 43);

  for (BadgeGravity gravity :
       {BadgeGravity::kTopEnd, BadgeGravity::kTopStart}) {
    BadgePlacement placement{gravity, 0, 0};

    Badge dot;
    dot.setDot();
    ASSERT_TRUE(dot.layout(anchor, placement));
    EXPECT_TRUE(Badge::ConservativeBounds(anchor, placement, false)
                    .contains(dot.bounds()));

    Badge text;
    text.setText("WWWWWWW");
    ASSERT_TRUE(text.layout(anchor, placement));
    Rect conservative_text = Badge::ConservativeBounds(anchor, placement, true);
    EXPECT_TRUE(conservative_text.contains(text.bounds()))
        << "gravity=" << static_cast<int>(gravity)
        << " conservative=" << RectToString(conservative_text)
        << " actual=" << RectToString(text.bounds());
  }
}

}  // namespace
}  // namespace material3
}  // namespace roo_windows