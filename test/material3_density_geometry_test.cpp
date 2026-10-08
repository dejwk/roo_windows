#include <algorithm>

#include "gtest/gtest.h"
#include "roo_icons/filled/navigation.h"
#include "roo_icons/outlined/48/action.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/button/internal/button_geometry.h"
#include "roo_windows/material3/text_field/internal/text_field_geometry.h"
#include "roo_windows/material3/text_field/text_field.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows::material3::internal {
namespace {

#if ROO_WINDOWS_ZOOM >= 200
constexpr int kButtonHeights[5][6] = {
    {64, 56, 56, 56, 56, 56},
    {80, 72, 64, 56, 56, 56},
    {112, 104, 96, 88, 80, 72},
    {192, 184, 176, 168, 160, 152},
    // Extra-large heights scale before narrowing.
    {272, 264, 256, 248, 240, 232}};
constexpr int kFieldHeights[2][6] = {{112, 104, 96, 96, 96, 96},
                                     {112, 104, 96, 88, 80, 72}};
#elif ROO_WINDOWS_ZOOM >= 150
constexpr int kButtonHeights[5][6] = {{48, 42, 42, 42, 42, 42},
                                      {60, 54, 48, 42, 42, 42},
                                      {84, 78, 72, 66, 60, 54},
                                      {144, 138, 132, 126, 120, 114},
                                      {204, 198, 192, 186, 180, 174}};
constexpr int kFieldHeights[2][6] = {{84, 78, 72, 72, 72, 72},
                                     {84, 78, 72, 66, 60, 54}};
#elif ROO_WINDOWS_ZOOM >= 100
constexpr int kButtonHeights[5][6] = {{32, 28, 28, 28, 28, 28},
                                      {40, 36, 32, 28, 28, 28},
                                      {56, 52, 48, 44, 40, 36},
                                      {96, 92, 88, 84, 80, 76},
                                      {136, 132, 128, 124, 120, 116}};
constexpr int kFieldHeights[2][6] = {{56, 52, 48, 48, 48, 48},
                                     {56, 52, 48, 44, 40, 36}};
#else
constexpr int kButtonHeights[5][6] = {{23, 21, 21, 21, 21, 21},
                                      {29, 27, 23, 21, 21, 21},
                                      {41, 39, 35, 33, 29, 27},
                                      {71, 69, 65, 63, 59, 57},
                                      {101, 99, 95, 93, 89, 87}};
constexpr int kFieldHeights[2][6] = {{42, 39, 36, 36, 36, 36},
                                     {42, 39, 36, 33, 30, 27}};
#endif

class DensityGeometryTest : public ::testing::Test {
 protected:
  DensityGeometryTest()
      : environment(scheduler),
        context(scheduler, environment.theme(),
                environment.keyboardColorTheme()) {}

  roo_scheduler::SchedulingService scheduler;
  Environment environment;
  ApplicationContext context;
};

// Verifies every size/level against independent pixel expectations, including
// odd-pixel rounding and unchanged production measurement at level zero.
TEST_F(DensityGeometryTest, ButtonSizeAndDensityMatrix) {
  const int horizontal_dp[] = {12, 16, 24, 48, 64};
  for (int size_index = 0; size_index < 5; ++size_index) {
    ButtonSize size = static_cast<ButtonSize>(size_index);
    Button button(context, "Label");
    button.setSize(size);
    ButtonContentMetrics metrics =
        ResolveButtonContentMetrics("Label", nullptr, size);
    for (int step = 0; step <= 5; ++step) {
      SCOPED_TRACE(::testing::Message()
                   << "size=" << size_index << " step=" << step);
      Padding padding = ResolveButtonPadding(size, SmallButtonPadding::kReduced,
                                             metrics.content_height, -step);
      EXPECT_EQ(padding.left(), Scaled(horizontal_dp[size_index]));
      EXPECT_EQ(padding.right(), padding.left());
      EXPECT_EQ(padding.top(), padding.bottom());
      EXPECT_EQ(metrics.content_height + padding.top() + padding.bottom(),
                kButtonHeights[size_index][step]);
      if (step > 0) {
        EXPECT_GE(padding.top(), Scaled(4));
      }
    }
    EXPECT_EQ(button.getNaturalDimensions().height(),
              kButtonHeights[size_index][0]);
    EXPECT_EQ(
        button.measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0))
            .height(),
        kButtonHeights[size_index][0]);
    EXPECT_EQ(
        button.measure(WidthSpec::Exactly(10), HeightSpec::Exactly(5)).height(),
        5);
  }
}

// Verifies scaled check artwork does not impose transparent canvas height on
// compact buttons; nominal slot width/gap stay fixed and both ink edges fit.
TEST_F(DensityGeometryTest, CompactIconUsesPaintedFootprint) {
  Material3Theme material = DefaultTheme().material3Theme();
  Theme themed{MakeFrameworkTheme(material), &material};
  ApplicationContext themed_context(scheduler, themed,
                                    DefaultKeyboardColorTheme());
  const MonoIcon& icon = SCALED_ROO_ICON(filled, navigation_check);
  Button plain(themed_context, "Save");
  Button with_icon(themed_context, "Save");
  with_icon.setIcon(&icon);
  for (int level = 0; level >= -5; --level) {
    material.density = static_cast<Density>(level);
    ButtonContentMetrics metrics =
        ResolveButtonContentMetrics("Save", &icon, ButtonSize::kSmall, level);
    EXPECT_EQ(metrics.icon_slot_width, Scaled(24));
    EXPECT_EQ(metrics.gap, Scaled(8));
    if (level != 0) {
      EXPECT_EQ(with_icon.getNaturalDimensions().height(),
                plain.getNaturalDimensions().height());
      int anchor_top = (with_icon.getNaturalDimensions().height() -
                        icon.anchorExtents().height()) /
                       2;
      EXPECT_GE(
          anchor_top + icon.extents().yMin() - icon.anchorExtents().yMin(),
          Scaled(4));
      int ink_bottom =
          anchor_top + icon.extents().yMax() - icon.anchorExtents().yMin();
      EXPECT_GE(with_icon.getNaturalDimensions().height() - ink_bottom - 1,
                Scaled(4));
    }
  }
}

// Verifies off-center and out-of-anchor artwork cannot escape compact floors.
// The metadata-only icons are measured, never decoded or painted.
TEST_F(DensityGeometryTest, CompactOffCenterIconKeepsBothEdges) {
  static const uint8_t data[] = {0};
  const roo_display::Box anchor(0, 0, 23, 23);
  for (roo_display::Box ink :
       {roo_display::Box(2, 0, 21, 3), roo_display::Box(2, 20, 21, 23),
        roo_display::Box(2, -4, 21, 25)}) {
    MonoIcon icon(ink, anchor, data,
                  roo_display::Alpha4(roo_display::color::Black));
    ButtonContentMetrics metrics =
        ResolveButtonContentMetrics({}, &icon, ButtonSize::kSmall, -5);
    Padding padding =
        ResolveButtonPadding(ButtonSize::kSmall, SmallButtonPadding::kReduced,
                             metrics.content_height, -5);
    int height = metrics.content_height + padding.top() + padding.bottom();
    int anchor_top = (height - anchor.height()) / 2;
    EXPECT_GE(anchor_top + ink.yMin(), Scaled(4));
    EXPECT_GE(height - (anchor_top + ink.yMax()) - 1, Scaled(4));
  }
}

// Verifies empty and oversized content, icon metrics, unchanged horizontal
// selectors, and symmetric content floors without shrinking text or artwork.
TEST_F(DensityGeometryTest, ButtonContentAndPaddingFloors) {
  const MonoIcon& icon = ic_outlined_48_action_done();
  for (int size_index = 0; size_index < 5; ++size_index) {
    ButtonSize size = static_cast<ButtonSize>(size_index);
    ButtonContentMetrics empty = ResolveButtonContentMetrics({}, nullptr, size);
    EXPECT_EQ(empty.content_width, 0);
    EXPECT_EQ(empty.content_height, 0);
    if (size == ButtonSize::kExtraSmall || size == ButtonSize::kSmall) {
      Padding minimum =
          ResolveButtonPadding(size, SmallButtonPadding::kReduced, 0, -5);
      EXPECT_EQ(minimum.top() + minimum.bottom(), Scaled(24));
    }
    ButtonContentMetrics icon_only =
        ResolveButtonContentMetrics({}, &icon, size);
    ButtonContentMetrics combined =
        ResolveButtonContentMetrics("Save", &icon, size);
    EXPECT_GE(icon_only.content_height, icon.anchorExtents().height());
    EXPECT_EQ(combined.content_height, icon_only.content_height);
    EXPECT_EQ(combined.content_width,
              combined.text_width + combined.icon_slot_width + combined.gap);
    for (int step = 0; step <= 5; ++step) {
      for (int content : {0, 301, static_cast<int>(combined.content_height)}) {
        Padding padding = ResolveButtonPadding(
            size, SmallButtonPadding::kReduced, content, -step);
        Padding full = ResolveButtonPadding(size, SmallButtonPadding::kDefault,
                                            content, -step);
        EXPECT_EQ(padding.top(), full.top());
        EXPECT_EQ(full.left() - padding.left(),
                  size == ButtonSize::kSmall ? Scaled(8) : 0);
        if (step > 0) {
          EXPECT_GE(padding.top(), Scaled(4));
          if (content == 301) {
            EXPECT_EQ(padding.top(), Scaled(4));
          }
        } else if (content == 301) {
          EXPECT_EQ(padding.top(), 0);
        }
      }
    }
  }
}

// Verifies resting round/square and pressed radii use measured dimensions,
// including narrow and zero-sized bounds at every compact natural height.
TEST_F(DensityGeometryTest, ButtonCornersFitMeasuredDimensions) {
  const int square_dp[] = {12, 12, 16, 28, 28};
  const int pressed_dp[] = {8, 8, 12, 16, 16};
  for (int size_index = 0; size_index < 5; ++size_index) {
    ButtonSize size = static_cast<ButtonSize>(size_index);
    for (int step = 0; step <= 5; ++step) {
      int height = kButtonHeights[size_index][step];
      EXPECT_EQ(ResolveButtonCornerRadius(size, ButtonShape::kRound, false,
                                          {Scaled(400), height}),
                height / 2);
      EXPECT_EQ(ResolveButtonCornerRadius(size, ButtonShape::kSquare, false,
                                          {Scaled(400), height}),
                std::min(Scaled(square_dp[size_index]), height / 2));
      EXPECT_EQ(ResolveButtonCornerRadius(size, ButtonShape::kRound, true,
                                          {Scaled(400), height}),
                std::min(Scaled(pressed_dp[size_index]), height / 2));
      EXPECT_EQ(ResolveButtonCornerRadius(size, ButtonShape::kSquare, false,
                                          {7, height}),
                3);
      EXPECT_EQ(ResolveButtonCornerRadius(size, ButtonShape::kSquare, true,
                                          {7, height}),
                3);
      EXPECT_EQ(ResolveButtonCornerRadius(size, ButtonShape::kRound, true,
                                          {0, height}),
                0);
    }
  }
  Button button(context, "Narrow");
  button.setSize(ButtonSize::kLarge);
  button.setShape(ButtonShape::kSquare);
  button.layout(Rect(0, 0, 6, 8));
  EXPECT_EQ(button.getBorderStyle().top_left_corner_radius(), 3);
  button.setPressed(true);
  EXPECT_EQ(button.getBorderStyle().top_left_corner_radius(), 3);
}

TextFieldMetrics StandardFieldMetrics() {
  return {text_style_body_large().lineHeight(),
          text_style_body_small().lineHeight(), 0, 0};
}

TextFieldSlotInput FieldInput(bool outlined, bool floating) {
  return {{Scaled(300), Scaled(160)}, outlined,   floating,   false,
          StandardFieldMetrics(),     Scaled(40), Scaled(20), Scaled(30)};
}

// Verifies both variants at all levels use actual font floors and that float
// transitions do not change container height, outline clearance, or assistive
// gaps.
TEST_F(DensityGeometryTest, FieldContainerAndFloatMatrix) {
  for (bool outlined : {false, true}) {
    TextField field(
        context, "Label",
        outlined ? TextFieldVariant::kOutlined : TextFieldVariant::kFilled);
    for (int step = 0; step <= 5; ++step) {
      TextFieldSlotInput input = FieldInput(outlined, false);
      int expected = kFieldHeights[outlined ? 1 : 0][step];
      int clearance = outlined ? input.metrics.small_height / 2 : 0;
      EXPECT_EQ(ResolveTextFieldContainerHeight(outlined, input.metrics, -step),
                expected);
      TextFieldSlots resting = ResolveTextFieldSlots(input, -step);
      input.floating = true;
      TextFieldSlots floated = ResolveTextFieldSlots(input, -step);
      EXPECT_EQ(resting.container, floated.container);
      EXPECT_EQ(floated.container.height(), expected);
      EXPECT_EQ(floated.container.yMin(), clearance);
      EXPECT_EQ(floated.viewport.height(), input.metrics.body_height);
      EXPECT_EQ(floated.label.height(), input.metrics.small_height);
      EXPECT_EQ(floated.prefix.width(), input.prefix_width);
      EXPECT_EQ(floated.suffix.width(), input.suffix_width);
      EXPECT_EQ(resting.prefix.width(), 0);
      EXPECT_EQ(resting.suffix.width(), 0);
      EXPECT_EQ(floated.assist.yMin(), clearance + expected + Scaled(4));
      EXPECT_EQ(
          ResolveTextFieldNaturalHeight(outlined, true, input.metrics, -step),
          clearance + expected + Scaled(4) + input.metrics.small_height);
      if (step > 0) {
        EXPECT_GE(floated.viewport.yMin() - clearance, Scaled(4));
        EXPECT_GE(floated.container.yMax() - floated.viewport.yMax(),
                  Scaled(4));
        if (!outlined) {
          EXPECT_GE(floated.label.yMin(), Scaled(4));
        }
      }
    }
    int default_height =
        kFieldHeights[outlined ? 1 : 0][0] +
        (outlined ? text_style_body_small().lineHeight() / 2 : 0);
    EXPECT_EQ(field.getNaturalDimensions().height(), default_height);
    field.setSupportingText("First line\nSecond line\nThird line");
    EXPECT_EQ(
        field.getNaturalDimensions().height(),
        default_height + Scaled(4) + text_style_body_small().lineHeight());
    field.setText("Populated");
    EXPECT_EQ(field.getSuggestedMinimumDimensions().height(),
              field.getNaturalDimensions().height());
  }
}

// Verifies tall icons and custom text metrics dominate compact heights and
// retain four-dp edges while zero preserves legacy slots and fixed height.
TEST(DensityFieldGeometry, TallContentFloors) {
  for (bool outlined : {false, true}) {
    TextFieldSlotInput input = FieldInput(outlined, true);
    input.metrics.leading_height = Scaled(90);
    input.metrics.trailing_height = Scaled(75);
    for (int step = 0; step <= 5; ++step) {
      TextFieldSlots slots = ResolveTextFieldSlots(input, -step);
      if (step == 0) {
        EXPECT_EQ(slots.container.height(), Scaled(56));
        EXPECT_EQ(slots.leading.height(), ROO_WINDOWS_ICON_SIZE);
      } else {
        EXPECT_EQ(slots.container.height(), Scaled(90) + 2 * Scaled(4));
        EXPECT_EQ(slots.leading.height(), Scaled(90));
        EXPECT_EQ(slots.trailing.height(), Scaled(75));
        EXPECT_GE(slots.leading.yMin() - slots.container.yMin(), Scaled(4));
        EXPECT_GE(slots.container.yMax() - slots.leading.yMax(), Scaled(4));
      }
      EXPECT_EQ(slots.leading.width(), ROO_WINDOWS_ICON_SIZE);
      EXPECT_EQ(slots.leading.xMin(), Scaled(12));
      EXPECT_EQ(slots.viewport.xMin(), Scaled(12) + ROO_WINDOWS_ICON_SIZE +
                                           Scaled(16) + input.prefix_width);
    }
    input.metrics = {Scaled(70), Scaled(30), 0, 0};
    EXPECT_EQ(ResolveTextFieldContainerHeight(outlined, input.metrics, -5),
              ROO_WINDOWS_ZOOM < 100 ? (outlined ? 58 : 80)
                                     : Scaled(outlined ? 78 : 108));
  }
}

// Verifies RTL mirrors logical slots without changing vertical geometry, and
// tight constraints clip every occupied rectangle inside the exact hit bounds.
TEST(DensityFieldGeometry, MirroringAndTightConstraints) {
  for (bool outlined : {false, true}) {
    for (bool floating : {false, true}) {
      for (int step = 0; step <= 5; ++step) {
        TextFieldSlotInput input = FieldInput(outlined, floating);
        input.metrics.leading_height = ROO_WINDOWS_ICON_SIZE;
        input.metrics.trailing_height = ROO_WINDOWS_ICON_SIZE;
        TextFieldSlots ltr = ResolveTextFieldSlots(input, -step);
        input.rtl = true;
        TextFieldSlots rtl = ResolveTextFieldSlots(input, -step);
        for (auto slot : {&TextFieldSlots::leading, &TextFieldSlots::trailing,
                          &TextFieldSlots::label, &TextFieldSlots::prefix,
                          &TextFieldSlots::suffix, &TextFieldSlots::viewport}) {
          Rect left = ltr.*slot;
          Rect right = rtl.*slot;
          EXPECT_EQ(left.yMin(), right.yMin());
          EXPECT_EQ(left.yMax(), right.yMax());
          EXPECT_EQ(input.bounds.width() - 1 - left.xMax(), right.xMin());
          EXPECT_EQ(input.bounds.width() - 1 - left.xMin(), right.xMax());
        }
        for (Dimensions bounds : {Dimensions(7, 5), Dimensions(0, 0)}) {
          input.bounds = bounds;
          TextFieldSlots tight = ResolveTextFieldSlots(input, -step);
          Rect clip(0, 0, bounds.width() - 1, bounds.height() - 1);
          for (Rect slot :
               {tight.container, tight.label, tight.viewport, tight.prefix,
                tight.suffix, tight.leading, tight.trailing, tight.assist}) {
            EXPECT_TRUE(slot.empty() || clip.contains(slot));
          }
        }
      }
    }
  }
}

}  // namespace
}  // namespace roo_windows::material3::internal
