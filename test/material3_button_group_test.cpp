#include <climits>

#include "button_group_test_support.h"
#include "gtest/gtest.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_icons/outlined/48/action.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/button/button_group.h"
#include "roo_windows/material3/button/internal/button_group_appearance.h"
#include "roo_windows/material3/button/internal/button_group_geometry.h"
#include "roo_windows/material3/button/internal/icon_geometry.h"
#include "roo_windows/material3/internal/density.h"

namespace roo_windows::material3 {
namespace {

using namespace internal;
// Verifies selected, unselected and disabled roles for every contained style.
TEST(ButtonGroupHelpers, ConnectedPalette) {
  const Theme& theme = DefaultTheme();
  const ColorScheme& colors = theme.material3Theme().color;
  EXPECT_EQ(colors.surfaceContainer,
            ResolveConnectedAppearance(theme, GroupStyle::kFilled, false, true)
                .container);
  EXPECT_EQ(colors.primary,
            ResolveConnectedAppearance(theme, GroupStyle::kFilled, true, true)
                .container);
  EXPECT_EQ(colors.secondary, ResolveConnectedAppearance(
                                  theme, GroupStyle::kFilledTonal, true, true)
                                  .container);
  EXPECT_EQ(colors.inverseSurface,
            ResolveConnectedAppearance(theme, GroupStyle::kOutlined, true, true)
                .container);
  for (GroupStyle style : {GroupStyle::kFilled, GroupStyle::kFilledTonal,
                           GroupStyle::kOutlined, GroupStyle::kElevated}) {
    EXPECT_EQ(ResolveConnectedAppearance(theme, style, false, false).container,
              ResolveConnectedAppearance(theme, style, true, false).container);
    EXPECT_EQ(roo_display::color::Transparent,
              ResolveConnectedAppearance(theme, style, true, false).outline);
  }
}

// Verifies exposed corners survive press and RTL mirrors only their placement.
TEST(ButtonGroupHelpers, ConnectedCorners) {
  Dimensions frame(80, 40);
  BorderStyle first = ResolveConnectedBorder(
      ButtonSize::kSmall, ButtonShape::kRound, LayoutDirection::kLeftToRight,
      true, false, false, 1, frame, 0);
  EXPECT_EQ(20, first.top_left_corner_radius());
  EXPECT_EQ(Scaled(4), first.top_right_corner_radius());
  BorderStyle rtl = ResolveConnectedBorder(
      ButtonSize::kSmall, ButtonShape::kRound, LayoutDirection::kRightToLeft,
      true, false, false, 1, frame, 0);
  EXPECT_EQ(first.top_left_corner_radius(), rtl.top_right_corner_radius());
  BorderStyle selected = ResolveConnectedBorder(
      ButtonSize::kSmall, ButtonShape::kSquare, LayoutDirection::kLeftToRight,
      false, false, true, 0, frame, 0);
  EXPECT_EQ(20, selected.top_left_corner_radius());
  BorderStyle single = ResolveConnectedBorder(
      ButtonSize::kSmall, ButtonShape::kRound, LayoutDirection::kLeftToRight,
      true, true, true, 1, frame, 0);
  EXPECT_EQ(20, single.top_right_corner_radius());
}

// Verifies extracted icon slots and widths preserve standalone geometry.
TEST(ButtonGroupHelpers, IconMetrics) {
  MonoIcon icon = ic_outlined_24_action_home();
  for (ButtonSize size :
       {ButtonSize::kExtraSmall, ButtonSize::kSmall, ButtonSize::kMedium,
        ButtonSize::kLarge, ButtonSize::kExtraLarge}) {
    EXPECT_EQ(Scaled(IconGeometryFor(size).height_dp),
              IconContainerWidth(size, IconButtonWidth::kUniform));
    EXPECT_GE(ResolveIconSlot(size, icon).width(),
              icon.anchorExtents().width());
    EXPECT_EQ(Scaled(2), ResolveGroupGap(size, true));
  }
}

// Verifies the SVG's worked example and bounded asymmetric donations.
TEST(ButtonGroupHelpers, WorkedPressArithmetic) {
  int16_t widths[] = {80, 120, 80};
  int16_t budgets[] = {20, 20, 20};
  RedistributeGroupWidths(3, 1, 1, widths, budgets);
  EXPECT_EQ(71, widths[0]);
  EXPECT_EQ(138, widths[1]);
  EXPECT_EQ(71, widths[2]);
  EXPECT_EQ(304, widths[0] + widths[1] + widths[2] + 2 * 12);
  int16_t bounded[] = {80, 120, 80};
  int16_t exhausted[] = {2, 20, 3};
  RedistributeGroupWidths(3, 1, 1, bounded, exhausted);
  EXPECT_EQ(78, bounded[0]);
  EXPECT_EQ(125, bounded[1]);
  EXPECT_EQ(77, bounded[2]);
  int16_t odd[] = {80, 100, 80};
  RedistributeGroupWidths(3, 1, 1, odd, budgets);
  EXPECT_EQ(73, odd[0]);
  EXPECT_EQ(115, odd[1]);
  EXPECT_EQ(72, odd[2]);
}

// Verifies conservation, floors and fixed-gap non-overlap for every press
// index/count, size gap, representative emphasis and mirrored constrained row.
TEST(ButtonGroupHelpers, PressGeometryInvariants) {
  for (int count : {0, 1, 2, 3, 5, 32}) {
    for (int active = -1; active < count; ++active) {
      for (float emphasis : {0.0f, 0.1f, 0.5f, 1.0f}) {
        int16_t widths[32] = {};
        int16_t original[32] = {};
        int16_t budgets[32] = {};
        int sum = 0;
        for (int i = 0; i < count; ++i) {
          widths[i] = original[i] = Scaled(40 + 3 * i);
          budgets[i] = Scaled(i % 3 == 0 ? 0 : 12);
          sum += widths[i];
        }
        RedistributeGroupWidths(count, active, emphasis, widths, budgets);
        int actual = 0;
        for (int i = 0; i < count; ++i) {
          actual += widths[i];
          EXPECT_GE(widths[i], original[i] - budgets[i]);
          if (i != active && i != active - 1 && i != active + 1) {
            EXPECT_EQ(original[i], widths[i]);
          }
        }
        EXPECT_EQ(sum, actual);
        for (ButtonSize size :
             {ButtonSize::kExtraSmall, ButtonSize::kSmall, ButtonSize::kMedium,
              ButtonSize::kLarge, ButtonSize::kExtraLarge}) {
          int gap = ResolveGroupGap(size, false);
          for (int assigned :
               {sum + std::max(0, count - 1) * gap, sum / 2, sum + 100}) {
            int logical = 0;
            int previous_right = -1;
            for (int i = 0; i < count; ++i) {
              int ltr = GroupIntervalLeft(assigned, logical, widths[i],
                                          LayoutDirection::kLeftToRight);
              int rtl = GroupIntervalLeft(assigned, logical, widths[i],
                                          LayoutDirection::kRightToLeft);
              EXPECT_EQ(assigned, ltr + rtl + widths[i]);
              if (i > 0) {
                EXPECT_EQ(gap, ltr - previous_right - 1);
              }
              previous_right = ltr + widths[i] - 1;
              logical += widths[i] + gap;
            }
          }
        }
      }
    }
  }
  int16_t gone[] = {80, 0, 120, 0, 80};
  int16_t budgets[] = {20, 0, 20, 0, 20};
  RedistributeGroupWidths(5, 2, 1, gone, budgets);
  EXPECT_EQ(71, gone[0]);
  EXPECT_EQ(138, gone[2]);
  EXPECT_EQ(71, gone[4]);
}

class GroupFixture : public testing::Test {
 protected:
  roo_scheduler::SchedulingService scheduler;
  Theme theme = DefaultTheme();
  Material3Theme& material = theme.material3Theme();
  Environment environment{scheduler, theme};
  ApplicationContext context{scheduler, environment.theme(),
                             environment.keyboardColorTheme()};
};

// Verifies selected elevated surfaces use on-primary overlays, while an
// unselected elevated surface retains primary interaction ink.
TEST_F(GroupFixture, ElevatedSelectionInteractionInk) {
  ButtonGroupButton entry(context, "Day", ButtonGroupButtonStyle::kElevated);
  ConnectedButtonGroup group(context);
  group.add(entry);
  EXPECT_EQ(material.color.onPrimary,
            entry.interactionColor(InteractionState::kHover));
  group.setSelectionRequired(false);
  group.setSelectionMask(0);
  EXPECT_EQ(material.color.primary,
            entry.interactionColor(InteractionState::kHover));
  group.setEnabled(false);
  EXPECT_EQ(Widget::OVERLAY_NONE, entry.getOverlayType());
}

// Verifies transparent empty groups and bounded adoption retain vector
// capacity.
TEST_F(GroupFixture, MembershipAndOwnership) {
  test_support::InternalGroup group(context);
  EXPECT_EQ(0, group.entryCount());
  EXPECT_EQ(0,
            group.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0))
                .width());
  EXPECT_FALSE(group.adopt(std::unique_ptr<ButtonGroupLabelEntry>()));
  for (int i = 0; i < 32; ++i)
    EXPECT_TRUE(
        group.adopt(std::make_unique<ButtonGroupLabelEntry>(context, "A")));
  EXPECT_FALSE(group.adopt(
      std::make_unique<ButtonGroupLabelEntry>(context, "Rejected")));
  ButtonGroupLabelEntry borrowed(context, "Borrowed");
  EXPECT_FALSE(group.add(borrowed));
  EXPECT_EQ(nullptr, borrowed.parent());
  size_t capacity = group.capacity();
  group.clear();
  EXPECT_EQ(0, group.entryCount());
  EXPECT_EQ(capacity, group.capacity());
  EXPECT_TRUE(group.add(borrowed));
  EXPECT_TRUE(borrowed.isClickable());
  group.clear();
  EXPECT_EQ(nullptr, borrowed.parent());
  EXPECT_FALSE(borrowed.isClickable());
}

// Verifies duplicate, cross-context and already-parented admissions are
// checked.
TEST_F(GroupFixture, InvalidAdmissionDiesBeforeCapacityCheck) {
  ButtonGroupLabelEntry entry(context, "Day");
  test_support::InternalGroup group(context);
  group.add(entry);
  EXPECT_DEATH(group.add(entry), "");
  test_support::InternalGroup other(context);
  EXPECT_DEATH(other.add(entry), "");
  ApplicationContext another(scheduler, environment.theme(),
                             environment.keyboardColorTheme());
  ButtonGroupLabelEntry foreign(another, "Foreign");
  EXPECT_DEATH(group.add(foreign), "");
}

// Verifies mixed density, exact frames, constraint clipping, RTL and gone
// slots.
TEST_F(GroupFixture, StableMixedLayout) {
  MonoIcon icon = ic_outlined_24_action_home();
  ButtonGroupLabelEntry label(context, "Day");
  ButtonGroupIconEntry artwork(context, icon);
  test_support::InternalGroup group(context);
  group.add(label);
  group.add(artwork);
  for (int level = 0; level >= -5; --level) {
    material.density = Density(static_cast<int8_t>(level));
    for (LayoutDirection direction :
         {LayoutDirection::kLeftToRight, LayoutDirection::kRightToLeft}) {
      group.setLayoutDirection(direction);
      Dimensions a = label.entryDimensions();
      Dimensions b = artwork.entryDimensions();
      Dimensions natural =
          group.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
      EXPECT_EQ(a.width() + b.width() + Scaled(2), natural.width());
      EXPECT_EQ(std::max(a.height(), b.height()), natural.height());
      group.measure(WidthSpec::Exactly(natural.width() + 5),
                    HeightSpec::Exactly(natural.height() + 10));
      group.layout(Rect(0, 0, natural.width() + 4, natural.height() + 9));
      EXPECT_EQ(a.width() + 3, label.width());
      EXPECT_EQ(b.width() + 2, artwork.width());
      EXPECT_EQ(0, label.offsetTop());
      EXPECT_EQ(natural.height(), artwork.height());
      EXPECT_EQ(direction == LayoutDirection::kLeftToRight
                    ? 0
                    : natural.width() + 5 - label.width(),
                label.offsetLeft());
      group.measure(WidthSpec::Exactly(20), HeightSpec::Exactly(12));
      group.layout(Rect(0, 0, 19, 11));
      EXPECT_EQ(a.width(), label.width());
      EXPECT_EQ(b.width(), artwork.width());
    }
  }
  label.setVisibility(Visibility::kInvisible);
  EXPECT_GT(group.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0))
                .width(),
            artwork.entryDimensions().width());
  label.setVisibility(Visibility::kGone);
  EXPECT_EQ(artwork.entryDimensions().width(),
            group.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0))
                .width());
  artwork.setVisibility(Visibility::kGone);
  EXPECT_EQ(0,
            group.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0))
                .height());
}

// Verifies full positive int16 exact widths survive framework spec encoding.
TEST_F(GroupFixture, LargeExactFramesAndNaturalOverflow) {
  EXPECT_EQ(INT16_MAX, WidthSpec::Exactly(INT16_MAX).value());
  EXPECT_EQ(30000, WidthSpec::AtMost(30000).value());
  ButtonGroupLabelEntry entry(context, "A");
  test_support::InternalGroup group(context);
  group.add(entry);
  group.measure(WidthSpec::Exactly(30000), HeightSpec::Exactly(40));
  group.layout(Rect(0, 0, 29999, 39));
  EXPECT_EQ(30000, entry.width());
  std::string oversized(8000, 'W');
  entry.setLabel(oversized);
  EXPECT_DEATH(
      group.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0)), "");
}

// Verifies adopted entries are deleted once and borrowed entries stay alive.
TEST_F(GroupFixture, ClearAndDestruction) {
  class Counted : public ButtonGroupLabelEntry {
   public:
    Counted(ApplicationContext& context, int& count)
        : ButtonGroupLabelEntry(context, "A"), count_(count) {}
    ~Counted() override { ++count_; }

   private:
    int& count_;
  };
  int destroyed = 0;
  ButtonGroupLabelEntry borrowed(context, "Day");
  {
    test_support::InternalGroup group(context);
    group.add(borrowed);
    group.adopt(std::make_unique<Counted>(context, destroyed));
    group.clear();
    EXPECT_EQ(1, destroyed);
    group.adopt(std::make_unique<Counted>(context, destroyed));
  }
  EXPECT_EQ(2, destroyed);
  EXPECT_EQ(nullptr, borrowed.parent());
}

// Verifies grouped icon density and oversized ink floors at all five sizes,
// selected outline reservation and live descendant theme relayout.
TEST_F(GroupFixture, AllGroupSizesAndDensityFloors) {
  MonoIcon icon = ic_outlined_48_action_home();
  ButtonGroupButton text(context, "View", ButtonGroupButtonStyle::kOutlined);
  ButtonGroupIconButton artwork(context, icon, ButtonGroupIconStyle::kOutlined);
  ConnectedButtonGroup group(context);
  group.add(text);
  group.add(artwork);
  for (ButtonSize size :
       {ButtonSize::kExtraSmall, ButtonSize::kSmall, ButtonSize::kMedium,
        ButtonSize::kLarge, ButtonSize::kExtraLarge}) {
    group.setSize(size);
    int previous_height = INT16_MAX;
    for (int level = 0; level >= -5; --level) {
      material.density = static_cast<Density>(level);
      group.requestLayoutDescending();
      group.invalidateDescending();
      Dimensions natural =
          group.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
      EXPECT_LE(natural.height(), previous_height);
      previous_height = natural.height();
      group.layout(Rect(0, 0, natural.width() - 1, natural.height() - 1));
      EXPECT_EQ(text.height(), artwork.height());
      EXPECT_GE(artwork.width(), icon.anchorExtents().width());
      if (level != 0) {
        int ink_floor =
            icon.anchorExtents().height() -
            2 * std::min(icon.extents().yMin() - icon.anchorExtents().yMin(),
                         icon.anchorExtents().yMax() - icon.extents().yMax());
        EXPECT_GE(artwork.height(), ink_floor + 2 * Scaled(4));
      }
      Dimensions before = text.entryDimensions();
      group.setSelectionMask(2);
      EXPECT_EQ(before.width(), text.entryDimensions().width());
      EXPECT_EQ(before.height(), text.entryDimensions().height());
      EXPECT_EQ(0, artwork.getBorderStyle().outline_width().ceil());
      group.setSelectionMask(1);
      EXPECT_EQ(0, text.getBorderStyle().outline_width().ceil());
      EXPECT_EQ(Scaled(SmallNumber(1)).ceil(),
                artwork.getBorderStyle().outline_width().ceil());
    }
  }
}

static_assert(sizeof(ButtonGroupLabelEntry) - sizeof(SurfaceWidget) <=
              (sizeof(void*) == 4 ? 20 : 40));
static_assert(sizeof(ButtonGroupIconEntry) - sizeof(SurfaceWidget) <=
              (sizeof(void*) == 4 ? 12 : 24));
static_assert(sizeof(test_support::InternalGroup) - sizeof(Container) <=
              (sizeof(void*) == 4 ? 16 : 32));
}  // namespace
}  // namespace roo_windows::material3
