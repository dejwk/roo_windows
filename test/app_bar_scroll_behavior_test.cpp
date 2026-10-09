#include <cstdlib>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display/ui/text_label.h"
#include "roo_icons/outlined/24/navigation.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/app_bar/app_bar.h"
#include "roo_windows/material3/button/icon_button.h"
#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"
#include "roo_windows/material3/typography.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace {
using namespace material3;
using namespace test_support;
using AppBarScrollBehaviorTest = RooWindowsRenderTestSized<320, 240>;

// Locate painted foreground rows without relying on widget/font geometry.
template <typename Raster>
std::pair<int, int> DarkInkRows(const Raster& raster, int16_t top) {
  int first = 64;
  int last = -1;
  for (int16_t dy = 0; dy < 64; ++dy) {
    int16_t y = top + dy;
    for (int16_t x = 0; x < 240; ++x) {
      roo_display::Color color;
      raster.readColors(&x, &y, 1, &color);
      if (color.r() < 128 && color.g() < 128 && color.b() < 128) {
        first = std::min(first, int(dy));
        last = std::max(last, int(dy));
      }
    }
  }
  return std::make_pair(first, last);
}

class MotionPanel : public SimpleScrollablePanel {
 public:
  using SimpleScrollablePanel::onDrag;
  using SimpleScrollablePanel::onDragFinished;
  using SimpleScrollablePanel::onDragStart;
  using SimpleScrollablePanel::onFling;
  using SimpleScrollablePanel::SimpleScrollablePanel;
  bool moving() const {
    return context().animations().contains(*this, kMotion);
  }
};

class MovingAppBarTest : public RooWindowsRenderTestSized<320, 240> {
 protected:
  void install(
      AppBarScrollBehavior behavior = AppBarScrollBehavior::kEnterAlways,
      AppBarVariant variant = AppBarVariant::kMediumFlexible,
      int content_height = 600) {
    auto column =
        std::make_unique<FlexLayout>(context(), FlexDirection::kColumn);
    auto bar = std::make_unique<AppBar>(context(), variant);
    bar_ = bar.get();
    bar_->setTitle("Equipment");
    auto panel = std::make_unique<MotionPanel>(context());
    panel_ = panel.get();
    panel_->setContents(std::make_unique<ColorBoxWidget>(
        context(), roo_display::color::White, Dimensions(320, content_height)));
    EXPECT_EQ(ScrollConnectionStatus::kSuccess,
              bar_->setScrollBehavior(*panel_, behavior));
    column->add(std::move(bar));
    column->add(std::move(panel),
                {.flex_grow = 1, .flex_basis = FlexBasis::kZero});
    app_.add(std::move(column), roo_display::Box(0, 0, 319, 239));
    refresh();
  }
  AppBar* bar_ = nullptr;
  MotionPanel* panel_ = nullptr;
};

// Verifies the documented 60-pixel transaction moves the screen-space body
// origin by exactly 60 pixels, and reverse input expands immediately.
TEST_F(MovingAppBarTest, EnterAlwaysConservesMovementThroughLayout) {
  install();
  ASSERT_EQ(112, bar_->height());
  ASSERT_EQ(128, panel_->height());
  panel_->onDragStart(0, 0);
  panel_->onDrag(0, 0, 0, -60);
  refresh();
  EXPECT_EQ(64, bar_->height());
  EXPECT_EQ(176, panel_->height());
  EXPECT_EQ(-12, panel_->getScrollPosition().y);
  panel_->onDrag(0, 0, 0, 10);
  refresh();
  EXPECT_EQ(74, bar_->height());
  EXPECT_EQ(-12, panel_->getScrollPosition().y);
  panel_->onDragFinished(0, 0);
  refresh();
  delay(200);
  refresh();
  EXPECT_EQ(64, bar_->height());
  EXPECT_FALSE(context().animations().contains(
      *bar_, material3::internal::AppBarScrollConnection::kSettle));
}

// Verifies a single fling continues from bar travel into content motion across
// viewport changes, and a new touch cancels the previous kinetic track.
TEST_F(MovingAppBarTest, FlingSurvivesCollapsingViewport) {
  install();
  panel_->onDragStart(0, 0);
  panel_->onDrag(0, 0, 0, -10);
  panel_->onFling(0, 0, 0, -1200);
  panel_->onDragFinished(0, -1200);
  refresh();
  delay(100);
  refresh();
  EXPECT_EQ(64, bar_->height());
  EXPECT_LT(panel_->getScrollPosition().y, 0);
  EXPECT_TRUE(panel_->moving());
  panel_->onDragStart(0, 0);
  auto stopped = panel_->getScrollPosition();
  delay(200);
  refresh();
  EXPECT_EQ(stopped.y, panel_->getScrollPosition().y);
  EXPECT_FALSE(panel_->moving());
}

// Verifies small bars slide completely away even with short content, and Home
// restores the bar when the content origin has never left zero.
TEST_F(MovingAppBarTest, SmallBarAndShortContent) {
  install(AppBarScrollBehavior::kEnterAlways, AppBarVariant::kSmall, 20);
  panel_->onDragStart(0, 0);
  panel_->onDrag(0, 0, 0, -32);
  refresh();
  auto* title = static_cast<material3::internal::AppBarTitle*>(
      static_cast<Widget&>(*bar_).focusChildAt(0));
  EXPECT_EQ(bar_->theme().material3Theme().color.onSurface,
            title->textColor(bar_->background()));
  panel_->onDrag(0, 0, 0, -32);
  refresh();
  EXPECT_EQ(0, bar_->height());
  EXPECT_EQ(240, panel_->height());
  panel_->scrollToTop();
  refresh();
  EXPECT_EQ(64, bar_->height());
}

// Verifies hiding the visual endpoint stops motion without dropping the
// binding.
TEST_F(MovingAppBarTest, HidingBarStopsCoordinatedMotion) {
  install();
  panel_->onDragStart(0, 0);
  panel_->onDrag(0, 0, 0, -20);
  panel_->onFling(0, 0, 0, -1200);
  bar_->setVisibility(Visibility::kInvisible);
  refresh();
  EXPECT_FALSE(panel_->moving());
  EXPECT_TRUE(bar_->hasScrollBehavior());
}

// Verifies the expanded title follows the bottom edge, then the compact
// title stays in the action row. Both phases fade through the surface color,
// and reversing collapse restores the same geometry and colors.
TEST_F(MovingAppBarTest, TitlesScrollOutAndFadeInAtFixedAnchors) {
  install();
  for (AppBarScrollBehavior behavior :
       {AppBarScrollBehavior::kEnterAlways,
        AppBarScrollBehavior::kExitUntilCollapsed}) {
    for (AppBarVariant variant :
         {AppBarVariant::kMediumFlexible, AppBarVariant::kLargeFlexible}) {
      for (AppBarTitleAlignment alignment :
           {AppBarTitleAlignment::kLeading, AppBarTitleAlignment::kCentered}) {
        for (const char* subtitle : {"", "Solar heating"}) {
          panel_->scrollToTop();
          bar_->setVariant(variant);
          bar_->setTitleAlignment(alignment);
          bar_->setSubtitle(subtitle);
          ASSERT_EQ(ScrollConnectionStatus::kSuccess,
                    bar_->setScrollBehavior(*panel_, behavior));
          refresh();
          auto* title = static_cast<material3::internal::AppBarTitle*>(
              static_cast<Widget&>(*bar_).focusChildAt(0));
          const Rect expanded = title->parent_bounds();
          const int travel = bar_->height() - 64;
          const roo_display::Color foreground =
              bar_->theme().material3Theme().color.onSurface;
          panel_->onDragStart(0, 0);
          for (int step = 0; step <= 2 * travel; ++step) {
            const int collapse = step <= travel ? step : 2 * travel - step;
            refresh();
            SCOPED_TRACE(collapse);
            const roo_display::Color background = bar_->background();
            if (collapse * 2 < travel) {
              EXPECT_EQ(expanded.translate(0, -collapse),
                        title->parent_bounds());
              EXPECT_EQ(expanded.height(), title->height());
            } else {
              EXPECT_EQ(text_style_title_large().lineHeight(), title->height());
              EXPECT_EQ((64 - title->height()) / 2, title->offsetTop());
              EXPECT_EQ(16, title->offsetLeft());
            }
            if (collapse == 0 || collapse == travel) {
              EXPECT_EQ(foreground, title->textColor(background));
            } else if (collapse * 2 == travel) {
              EXPECT_EQ(background, title->textColor(background));
            } else if (collapse * 4 == travel || collapse * 4 == 3 * travel) {
              EXPECT_EQ(
                  roo_display::AlphaBlend(background, foreground.withA(128)),
                  title->textColor(background));
            }
            if (step < 2 * travel) {
              panel_->onDrag(0, 0, 0, step < travel ? -1 : 1);
            }
          }
          panel_->onDragFinished(0, 0);
        }
      }
    }
  }
}

// Verifies the subtitle stays below the moving title, sharing its leading
// edge or center, throughout collapse and expansion in both flexible variants.
TEST_F(MovingAppBarTest, SubtitleFollowsTitleDuringCollapseAndExpansion) {
  install();
  bar_->setTitle("Pool");
  bar_->setSubtitle("Solar heating equipment");
  bar_->setLeading(std::make_unique<IconButton>(
      context(), ic_outlined_24_navigation_menu()));
  for (AppBarVariant variant :
       {AppBarVariant::kMediumFlexible, AppBarVariant::kLargeFlexible}) {
    bar_->setVariant(variant);
    for (AppBarTitleAlignment alignment :
         {AppBarTitleAlignment::kLeading, AppBarTitleAlignment::kCentered}) {
      bar_->setTitleAlignment(alignment);
      panel_->scrollToTop();
      refresh();
      Widget* title = static_cast<Widget&>(*bar_).focusChildAt(0);
      Widget* subtitle = static_cast<Widget&>(*bar_).focusChildAt(1);
      ASSERT_NE(nullptr, title);
      ASSERT_NE(nullptr, subtitle);
      const int travel = bar_->height() - 64;
      const int subtitle_height = subtitle->height();
      panel_->onDragStart(0, 0);
      for (int step = 0; step <= 2 * travel; ++step) {
        const int collapse = step <= travel ? step : 2 * travel - step;
        SCOPED_TRACE(collapse);
        EXPECT_EQ(collapse * 2 >= travel, subtitle->isGone());
        if (!subtitle->isGone()) {
          EXPECT_EQ(title->parent_bounds().yMax() + 1, subtitle->offsetTop());
          EXPECT_EQ(subtitle_height, subtitle->height());
          if (alignment == AppBarTitleAlignment::kLeading) {
            EXPECT_EQ(title->offsetLeft(), subtitle->offsetLeft());
            EXPECT_EQ(subtitle->width(), title->width());
          } else {
            EXPECT_EQ(
                title->parent_bounds().xMin() + title->parent_bounds().xMax(),
                subtitle->parent_bounds().xMin() +
                    subtitle->parent_bounds().xMax());
          }
        }
        if (step < 2 * travel) {
          panel_->onDrag(0, 0, 0, step < travel ? -1 : 1);
        }
      }
      panel_->onDragFinished(0, 0);
    }
  }
}

// Flexible action rows stay fixed while the title reaches their center.
TEST_F(MovingAppBarTest, CollapsedTitleAndActionsShareCenter) {
  install(AppBarScrollBehavior::kExitUntilCollapsed);
  auto leading = std::make_unique<IconButton>(
      context(), ic_outlined_24_navigation_arrow_back(),
      IconButtonStyle::kStandard);
  auto trailing = std::make_unique<IconButton>(
      context(), ic_outlined_24_navigation_more_vert(),
      IconButtonStyle::kStandard);
  IconButton* leading_ptr = leading.get();
  IconButton* trailing_ptr = trailing.get();
  bar_->setLeading(std::move(leading));
  bar_->setTrailing(0, std::move(trailing));
  for (AppBarVariant variant :
       {AppBarVariant::kMediumFlexible, AppBarVariant::kLargeFlexible}) {
    for (ButtonSize size : {ButtonSize::kExtraSmall, ButtonSize::kSmall}) {
      bar_->setVariant(variant);
      leading_ptr->setSize(size);
      trailing_ptr->setSize(size);
      panel_->scrollToTop();
      refresh();
      int travel = bar_->height() - 64;
      const int action_y = leading_ptr->offsetTop();
      const int trailing_y = trailing_ptr->offsetTop();
      panel_->onDragStart(0, 0);
      for (int i = 0; i < travel; ++i) {
        panel_->onDrag(0, 0, 0, -1);
        EXPECT_EQ(action_y, leading_ptr->offsetTop());
        EXPECT_EQ(trailing_y, trailing_ptr->offsetTop());
        if (2 * (i + 1) >= travel) {
          Widget* title = static_cast<Widget&>(*bar_).focusChildAt(0);
          EXPECT_EQ(56, title->offsetLeft());
          EXPECT_EQ(208, title->width());
          EXPECT_EQ((64 - title->height()) / 2, title->offsetTop());
        }
      }
      refresh();
      EXPECT_EQ(64, bar_->height());
      Widget* title = static_cast<Widget&>(*bar_).focusChildAt(0);
      for (Widget* child : {title, static_cast<Widget*>(leading_ptr),
                            static_cast<Widget*>(trailing_ptr)}) {
        EXPECT_NEAR(
            63, child->parent_bounds().yMin() + child->parent_bounds().yMax(),
            1);
      }
      if (variant == AppBarVariant::kMediumFlexible &&
          size == ButtonSize::kSmall) {
        EXPECT_TRUE(test::CompareOrUpdateGolden(
            test::CaptureRgb(offscreen_.raster(), 0, 0, 320, 240),
            "test/goldens/app_bar_scroll/collapsed_actions.ppm",
            "app_bar_collapsed_actions"));
      }
      for (int i = 0; i < travel; ++i) {
        panel_->onDrag(0, 0, 0, 1);
        EXPECT_EQ(action_y, leading_ptr->offsetTop());
        EXPECT_EQ(trailing_y, trailing_ptr->offsetTop());
      }
    }
  }
}

// Checks painted glyph bounds independently from the title's layout box.
TEST_F(MovingAppBarTest, CollapsedTitlePaintIsCentered) {
  install(AppBarScrollBehavior::kExitUntilCollapsed);
  bar_->setTitle("Solar heating");
  bar_->setSubtitle("Equipment details");
  refresh();
  panel_->onDragStart(0, 0);
  panel_->onDrag(0, 0, 0, -(bar_->height() - 64));
  refresh();
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      test::CaptureRgb(offscreen_.raster(), 0, 0, 320, 240),
      "test/goldens/app_bar_scroll/collapsed_solar.ppm",
      "app_bar_collapsed_solar"));
  const TextStyle& style = text_style_title_large();
  const roo_display::Box ink =
      style.font()
          .getHorizontalStringMetrics("Solar heating", style.fontOptions())
          .screen_extents();
  const int baseline = (64 + style.ascent()) / 2;
  EXPECT_EQ(std::make_pair(baseline + ink.yMin(), baseline + ink.yMax()),
            DarkInkRows(offscreen_.raster(), 0));
  // Compare every pixel with an unclipped label at the intended baseline.
  // Checking just the vertical ink rows cannot detect a clipped first glyph.
  roo::byte reference_pixels[320 * 64 * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> reference_device(
      320, 64, reference_pixels, roo_display::Argb4444());
  roo_display::Display reference_display(reference_device);
  const roo_display::Color background = bar_->background();
  reference_display.init(background);
  {
    roo_display::DrawingContext dc(reference_display);
    dc.setBackgroundColor(background);
    dc.draw(roo_display::StringViewLabel(
                "Solar heating", style.font(),
                context().theme().material3Theme().color.onSurface,
                style.fontOptions()),
            16, baseline);
  }
  for (int16_t y = 0; y < 64; ++y) {
    for (int16_t x = 0; x < 320; ++x) {
      roo_display::Color expected;
      reference_device.raster().readColors(&x, &y, 1, &expected);
      ASSERT_EQ(expected, pixelAt(x, y)) << "pixel " << x << ", " << y;
    }
  }
}

// Verifies half-ascent placement for odd/even line heights, different fonts,
// and text with descenders, independently of the label alignment helper.
TEST_F(AppBarScrollBehaviorTest, TitleBaselineUsesHalfAscent) {
  auto title = std::make_unique<material3::internal::AppBarTitle>(context());
  auto* title_ptr = title.get();
  app_.add(std::move(title), roo_display::Box(0, 0, 239, 63));
  for (const TextStyle* style :
       {&text_style_title_large(), &text_style_headline_small(),
        &text_style_headline_medium()}) {
    title_ptr->setTextStyle(*style);
    for (int height : {48, 49}) {
      title_ptr->layout(Rect(0, 0, 239, height - 1));
      for (const char* text : {"H", "gyp"}) {
        SCOPED_TRACE(text);
        SCOPED_TRACE(height);
        SCOPED_TRACE(style->ascent());
        title_ptr->setText(text);
        title_ptr->invalidateInterior();
        refresh();
        const roo_display::Box ink =
            style->font()
                .getHorizontalStringMetrics(text, style->fontOptions())
                .screen_extents();
        const int baseline = (height + style->ascent()) / 2;
        EXPECT_EQ(std::make_pair(baseline + ink.yMin(), baseline + ink.yMax()),
                  DarkInkRows(offscreen_.raster(), 0));
      }
    }
  }
}

// Verifies centered titles use the same advance-width and ascent anchors as
// ordinary app-bar text, including spaces that do not contribute any ink.
TEST_F(AppBarScrollBehaviorTest, CenteredTitleMatchesStandardTextAlignment) {
  auto title = std::make_unique<material3::internal::AppBarTitle>(context());
  auto reference = std::make_unique<material3::internal::AppBarText>(context());
  auto* title_ptr = title.get();
  auto* reference_ptr = reference.get();
  title_ptr->setAlignment(roo_display::kCenter | roo_display::kMiddle);
  reference_ptr->setAlignment(roo_display::kCenter | roo_display::kMiddle);
  app_.add(std::move(title), roo_display::Box(0, 0, 239, 63));
  app_.add(std::move(reference), roo_display::Box(0, 64, 239, 127));
  for (const TextStyle* style :
       {&text_style_title_large(), &text_style_headline_medium()}) {
    title_ptr->setTextStyle(*style);
    reference_ptr->setTextStyle(*style);
    for (const char* text : {" Wi-Fi", "gyp ", "Equipment"}) {
      SCOPED_TRACE(text);
      title_ptr->setText(text);
      reference_ptr->setText(text);
      refresh();
      for (int16_t y = 0; y < 64; ++y) {
        int16_t reference_y = y + 64;
        for (int16_t x = 0; x < 240; ++x) {
          roo_display::Color actual;
          roo_display::Color expected;
          offscreen_.raster().readColors(&x, &y, 1, &actual);
          offscreen_.raster().readColors(&x, &reference_y, 1, &expected);
          ASSERT_EQ(expected, actual) << "pixel " << x << ", " << y;
        }
      }
    }
  }
}

// Verifies expanded, fading, half-collapsed and compact frames with a subtitle
// and centered title, including the exact typography-switch boundary.
TEST_F(MovingAppBarTest, CollapseFramesGolden) {
  install();
  bar_->setSubtitle("Solar heating");
  bar_->setTitleAlignment(AppBarTitleAlignment::kCentered);
  refresh();
  panel_->onDragStart(0, 0);
  const int steps[] = {0, -18, -17, -1, -18, -18};
  const char* names[] = {"expanded", "fading",    "before_switch",
                         "midpoint", "fading_in", "collapsed"};
  for (int i = 0; i < 6; ++i) {
    panel_->onDrag(0, 0, 0, steps[i]);
    refresh();
    EXPECT_TRUE(test::CompareOrUpdateGolden(
        test::CaptureRgb(offscreen_.raster(), 0, 0, 320, 240),
        std::string("test/goldens/app_bar_scroll/") + names[i] + ".ppm",
        std::string("app_bar_scroll_") + names[i]));
  }
  panel_->onDrag(0, 0, 0, 54);
  refresh();
  EXPECT_TRUE(test::CompareOrUpdateGolden(
      test::CaptureRgb(offscreen_.raster(), 0, 0, 320, 240),
      "test/goldens/app_bar_scroll/fading.ppm",
      "app_bar_scroll_fading_reverse"));
}

// Verifies the existing scaffold uses the same shrinking top-bar/body geometry.
TEST_F(AppBarScrollBehaviorTest, ScaffoldAndSearchBarGeometry) {
  auto scaffold = std::make_unique<LayoutScaffold>(context());
  auto bar = std::make_unique<SearchAppBar>(context());
  SearchAppBar* bar_ptr = bar.get();
  auto panel = std::make_unique<MotionPanel>(context());
  MotionPanel* panel_ptr = panel.get();
  panel->setContents(std::make_unique<ColorBoxWidget>(
      context(), roo_display::color::White, Dimensions(320, 600)));
  ASSERT_EQ(ScrollConnectionStatus::kSuccess,
            bar->setScrollBehavior(*panel, AppBarScrollBehavior::kEnterAlways));
  scaffold->setTopBar(std::move(bar));
  scaffold->setBody(std::move(panel));
  app_.add(std::move(scaffold), roo_display::Box(0, 0, 319, 239));
  refresh();
  EXPECT_EQ(64, bar_ptr->height());
  panel_ptr->onDragStart(0, 0);
  panel_ptr->onDrag(0, 0, 0, -32);
  refresh();
  EXPECT_EQ(32, bar_ptr->height());
  EXPECT_EQ(208, panel_ptr->height());
  std::vector<Widget*> path;
  EXPECT_FALSE(bar_ptr->fillTouchTargetPath(10, 40, path));
  panel_ptr->scrollToTop();
  refresh();
  EXPECT_EQ(64, bar_ptr->height());
}

// Verifies constrained geometry degrades safely and recovers after constraints
// are removed, without changing the chosen policy.
TEST_F(AppBarScrollBehaviorTest, FixedHeightAndRepeatedBinding) {
  AppBar bar(context(), AppBarVariant::kMediumFlexible);
  SimpleScrollablePanel panel(context());
  ASSERT_EQ(ScrollConnectionStatus::kSuccess,
            bar.setScrollBehavior(panel, AppBarScrollBehavior::kEnterAlways));
  auto connection = material3::internal::FindAppBarConnection(bar);
  connection->onPreScroll(-20);
  EXPECT_EQ(20, connection->collapse());
  EXPECT_EQ(ScrollConnectionStatus::kSuccess,
            bar.setScrollBehavior(panel, AppBarScrollBehavior::kEnterAlways));
  EXPECT_EQ(20, connection->collapse());
  bar.measure(WidthSpec::Exactly(320), HeightSpec::Exactly(112));
  EXPECT_EQ(0, connection->collapse());
  EXPECT_FALSE(connection->canScroll());
  bar.measure(WidthSpec::Exactly(320), HeightSpec::AtMost(240));
  EXPECT_TRUE(connection->canScroll());
}

// Verifies reverse input reaches the content top before expanding the bar,
// including a single delta split between content and bar.
TEST_F(MovingAppBarTest, ExitUntilCollapsedExpandsOnlyAtTop) {
  install(AppBarScrollBehavior::kExitUntilCollapsed);
  panel_->onDragStart(0, 0);
  panel_->onDrag(0, 0, 0, -60);
  EXPECT_EQ(64, bar_->height());
  EXPECT_EQ(-12, panel_->getScrollPosition().y);
  panel_->onDrag(0, 0, 0, 10);
  EXPECT_EQ(64, bar_->height());
  EXPECT_EQ(-2, panel_->getScrollPosition().y);
  panel_->onDrag(0, 0, 0, 10);
  EXPECT_EQ(72, bar_->height());
  EXPECT_EQ(0, panel_->getScrollPosition().y);
  panel_->onDragFinished(0, 0);
  refresh();
  delay(200);
  refresh();
  EXPECT_EQ(64, bar_->height());
  panel_->scrollToTop();
  refresh();
  EXPECT_EQ(112, bar_->height());
}

// Verifies the small exit-until-collapsed variant keeps its compact height.
TEST_F(MovingAppBarTest, SmallExitUntilCollapsedIsPinned) {
  install(AppBarScrollBehavior::kExitUntilCollapsed, AppBarVariant::kSmall);
  panel_->onDragStart(0, 0);
  panel_->onDrag(0, 0, 0, -60);
  refresh();
  EXPECT_EQ(64, bar_->height());
  EXPECT_EQ(-60, panel_->getScrollPosition().y);
}

// Verifies clearing a live binding restores geometry, stops fling and preserves
// the content's legal position, while unsupported rebind leaves it intact.
TEST_F(MovingAppBarTest, ClearDuringMotionAndUnsupportedSearchPolicy) {
  install();
  panel_->onDragStart(0, 0);
  panel_->onDrag(0, 0, 0, -60);
  panel_->onFling(0, 0, 0, -1200);
  EXPECT_EQ(ScrollConnectionStatus::kSuccess, bar_->clearScrollBehavior());
  refresh();
  EXPECT_EQ(112, bar_->height());
  EXPECT_FALSE(panel_->moving());
  SearchAppBar search(context());
  EXPECT_EQ(ScrollConnectionStatus::kSuccess,
            search.setScrollBehavior(*panel_, AppBarScrollBehavior::kPinned));
  EXPECT_EQ(ScrollConnectionStatus::kUnsupportedBehavior,
            search.setScrollBehavior(
                *panel_, AppBarScrollBehavior::kExitUntilCollapsed));
  EXPECT_TRUE(search.hasScrollBehavior());
}

// Verifies replacing wrapped content resets the bar even though the wrapper
// retains its identity, and callbacks can still remove their own registration.
TEST_F(AppBarScrollBehaviorTest, WrappedReplacementResetsBehavior) {
  ScrollablePanel panel(context());
  panel.setContents(std::make_unique<ColorBoxWidget>(
      context(), roo_display::color::White, Dimensions(320, 600)));
  AppBar bar(context(), AppBarVariant::kMediumFlexible);
  ASSERT_EQ(ScrollConnectionStatus::kSuccess,
            bar.setScrollBehavior(panel, AppBarScrollBehavior::kEnterAlways));
  auto connection = material3::internal::FindAppBarConnection(bar);
  connection->onPreScroll(-30);
  EXPECT_EQ(30, connection->collapse());
  panel.setContents(std::make_unique<ColorBoxWidget>(
      context(), roo_display::color::White, Dimensions(320, 100)));
  EXPECT_EQ(0, connection->collapse());
}

// Verifies app callbacks remain removable during delivery while binding
// mutations are rejected until the active scroll transaction finishes.
TEST_F(MovingAppBarTest, CallbackCanRemoveItselfButCannotRebindDuringDispatch) {
  install();
  ScrollConnectionStatus cleared = ScrollConnectionStatus::kSuccess;
  ScrollConnectionStatus rebound = ScrollConnectionStatus::kSuccess;
  panel_->setOnScrollPositionChanged([&](ScrollPosition, ScrollPosition) {
    cleared = bar_->clearScrollBehavior();
    rebound = bar_->setScrollBehavior(*panel_, AppBarScrollBehavior::kPinned);
    panel_->setOnScrollPositionChanged(nullptr);
  });
  panel_->onDragStart(0, 0);
  panel_->onDrag(0, 0, 0, -60);
  EXPECT_EQ(ScrollConnectionStatus::kBusy, cleared);
  EXPECT_EQ(ScrollConnectionStatus::kBusy, rebound);
  EXPECT_TRUE(bar_->hasScrollBehavior());
}

// Verifies a kinetic overshoot springs back without expanding the compact bar
// and releases the motion channel after the legal endpoint is applied.
TEST_F(MovingAppBarTest, FlingAndSpringReachExactEndpoint) {
  install();
  panel_->onDragStart(0, 0);
  panel_->onDrag(0, 0, 0, -60);
  panel_->onFling(0, 0, 0, -1200);
  panel_->onDragFinished(0, -1200);
  refresh();
  delay(800);
  refresh();
  // The replacement spring track anchors on its first sampled frame.
  delay(20);
  refresh();
  delay(600);
  refresh();
  EXPECT_EQ(64, bar_->height());
  EXPECT_FALSE(panel_->moving());
  Margins margins = panel_->contents()->getMargins();
  EXPECT_EQ(panel_->height() - margins.top() - margins.bottom() -
                panel_->contents()->height(),
            panel_->getScrollPosition().y);
}

// Verifies initial synchronization, independent callbacks and manual
// restoration.
TEST_F(AppBarScrollBehaviorTest, PinnedCoexistsWithPositionCallback) {
  SimpleScrollablePanel panel(context());
  ColorBoxWidget content(context(), roo_display::color::White,
                         Dimensions(100, 600));
  panel.setContents(content);
  panel.measure(WidthSpec::Exactly(100), HeightSpec::Exactly(200));
  panel.layout(Rect(0, 0, 99, 199));
  panel.scrollTo(0, -30);
  AppBar bar(context());
  bar.setSurfaceState(AppBarSurfaceState::kFlat);
  int calls = 0;
  panel.setOnScrollPositionChanged(
      [&](ScrollPosition, ScrollPosition) { ++calls; });
  ASSERT_EQ(ScrollConnectionStatus::kSuccess,
            bar.setScrollBehavior(panel, AppBarScrollBehavior::kPinned));
  EXPECT_EQ(AppBarSurfaceState::kScrolled, bar.surfaceState());
  panel.scrollToTop();
  EXPECT_EQ(AppBarSurfaceState::kFlat, bar.surfaceState());
  EXPECT_EQ(1, calls);
  bar.setSurfaceState(AppBarSurfaceState::kScrolled);
  EXPECT_EQ(AppBarSurfaceState::kFlat, bar.surfaceState());
  ASSERT_EQ(ScrollConnectionStatus::kSuccess, bar.clearScrollBehavior());
  EXPECT_EQ(AppBarSurfaceState::kScrolled, bar.surfaceState());
  EXPECT_FALSE(bar.hasScrollBehavior());
}

// Verifies duplicate binding, failed replacement and either endpoint lifetime.
TEST_F(AppBarScrollBehaviorTest, BindingIsExclusiveAndRestoresSurvivingBar) {
  AppBar bar(context());
  SearchAppBar other(context());
  {
    SimpleScrollablePanel panel(context());
    EXPECT_EQ(ScrollConnectionStatus::kSuccess,
              bar.setScrollBehavior(panel, AppBarScrollBehavior::kPinned));
    EXPECT_EQ(ScrollConnectionStatus::kSuccess,
              bar.setScrollBehavior(panel, AppBarScrollBehavior::kPinned));
    EXPECT_EQ(ScrollConnectionStatus::kAlreadyConnected,
              other.setScrollBehavior(panel, AppBarScrollBehavior::kPinned));
    EXPECT_TRUE(bar.hasScrollBehavior());
  }
  EXPECT_FALSE(bar.hasScrollBehavior());
  SimpleScrollablePanel panel(context());
  {
    SearchAppBar temporary(context());
    EXPECT_EQ(
        ScrollConnectionStatus::kSuccess,
        temporary.setScrollBehavior(panel, AppBarScrollBehavior::kPinned));
  }
  EXPECT_EQ(nullptr,
            roo_windows::internal::ScrollConnectionRegistry::Find(panel));
}
}  // namespace
}  // namespace roo_windows
