#include "roo_windows/widgets/text_label.h"

#include <algorithm>

#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_display/ui/text_label.h"
#include "roo_fonts/NotoSerif_Italic/40.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/containers/flex_layout.h"

using namespace roo_display;
using namespace roo_windows;

namespace roo_windows {
namespace {

ApplicationContext MakeContext(Environment& context) {
  return ApplicationContext(context.scheduler(), context.theme(),
                            context.keyboardColorTheme());
}

constexpr char kOverhangText[] = "jQy/";

Color QuantizeToArgb4444(Color color) {
  Argb4444 mode;
  return mode.toArgbColor(mode.fromArgbColor(color));
}

FlexLayout::Params FillWidthParams() {
  FlexLayout::Params params;
  params.flex_grow = 1;
  params.flex_basis = FlexBasis::kZero;
  return params;
}

struct PixelBounds {
  bool found;
  int16_t x_min;
  int16_t y_min;
  int16_t x_max;
  int16_t y_max;
};

class TextLabelRenderTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 96;
  static constexpr int16_t kHeight = 64;

  TextLabelRenderTest()
      : offscreen_(kWidth, kHeight, raster_, Argb4444()),
        display_(offscreen_),
        env_(scheduler_),
        app_(&env_, display_) {}

  ApplicationContext& context() { return app_.context(); }

  void refresh() { app_.refresh(); }

  Color pixelAt(int16_t x, int16_t y) const {
    int16_t px[] = {x};
    int16_t py[] = {y};
    Color color[1];
    offscreen_.raster().readColors(px, py, 1, color);
    return color[0];
  }

  PixelBounds findNonBackground(int16_t x0, int16_t y0, int16_t x1, int16_t y1,
                                Color background) const {
    PixelBounds out = {false, 0, 0, 0, 0};
    for (int16_t y = y0; y <= y1; ++y) {
      for (int16_t x = x0; x <= x1; ++x) {
        if (pixelAt(x, y) == background) continue;
        if (!out.found) {
          out.found = true;
          out.x_min = out.x_max = x;
          out.y_min = out.y_max = y;
        } else {
          out.x_min = std::min(out.x_min, x);
          out.y_min = std::min(out.y_min, y);
          out.x_max = std::max(out.x_max, x);
          out.y_max = std::max(out.y_max, y);
        }
      }
    }
    return out;
  }

  roo::byte raster_[kWidth * kHeight * 2];
  OffscreenDevice<Argb4444> offscreen_;
  Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Environment env_;
  Application app_;
};

class TextLabelGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 128;
  static constexpr int16_t kHeight = 80;
  static constexpr int16_t kLayoutX = 8;
  static constexpr int16_t kLayoutY = 8;
  static constexpr int16_t kLayoutWidth = 96;
  static constexpr int16_t kLayoutHeight = 40;

  TextLabelGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, Argb4444()),
        display_(offscreen_),
        env_(scheduler_) {}

  Offscreen<Rgb888> RenderFlexLabel(Gravity gravity,
                                    const FlexLayout::Params& params,
                                    PaddingSize padding = PaddingSize::kNone) {
    Application app(&env_, display_);

    FlexLayout layout(app.context(), FlexDirection::kRow);
    layout.setPadding(Padding(12, 8));
    layout.setJustifyContent(JustifyContent::kCenter);
    layout.setAlignItems(AlignItems::kCenter);

    TextLabel label(app.context(), kOverhangText, material2::text_style_body2(),
                    gravity);
    label.setPadding(padding);
    layout.add(label, params);

    app.add(layout, Box(kLayoutX, kLayoutY, kLayoutX + kLayoutWidth - 1,
                        kLayoutY + kLayoutHeight - 1));
    app.refresh();
    return test::CaptureRgb(offscreen_.raster(), kLayoutX, kLayoutY,
                            kLayoutWidth, kLayoutHeight);
  }

  roo::byte raster_[kWidth * kHeight * 2];
  OffscreenDevice<Argb4444> offscreen_;
  Display display_;
  roo_scheduler::SchedulingService scheduler_;
  Environment env_;
};

class RecordingPanel : public Panel {
 public:
  explicit RecordingPanel(ApplicationContext& context) : Panel(context) {}

  using Panel::add;

  std::vector<Rect> invalidated_regions;

 protected:
  void childShown(const Widget* child) override { (void)child; }

  void propagateDirty(const Widget* child, const Rect& rect) override {
    (void)child;
    (void)rect;
  }

  void childInvalidatedRegion(const Widget* child, Rect rect) override {
    (void)child;
    invalidated_regions.push_back(rect);
  }
};

}  // namespace

// Verifies that the natural minimum size of a TextLabel matches the font's
// advance for the configured text horizontally and the font's full line
// height (ascent - descent + linegap) vertically.
TEST(TextLabel, SuggestedMinimumDimensionsMatchFontMetrics) {
  roo_scheduler::SchedulingService scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  const auto& font = material2::text_style_body2();
  TextLabel label(context, "abc", font);

  auto metrics =
      font.font().getHorizontalStringMetrics("abc", font.fontOptions());
  Dimensions dims = label.getSuggestedMinimumDimensions();
  EXPECT_EQ(metrics.advance(), dims.width());
  EXPECT_EQ(font.lineHeight(), dims.height());
}

// Verifies that getContentBounds() reflects the glyphs' actual ink extents
// translated by the gravity-resolved alignment offset, rather than the full
// layout rectangle.
TEST(TextLabel, ContentBoundsFollowDrawableInkExtents) {
  roo_scheduler::SchedulingService scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  const auto& font = material2::text_style_body2();
  TextLabel label(context, "abc", font, kGravityLeft | kGravityMiddle);
  label.setPadding(PaddingSize::kNone);

  Dimensions dims = label.getSuggestedMinimumDimensions();
  constexpr int16_t kLabelHeight = 40;
  label.layout(Rect(0, 0, dims.width() - 1, kLabelHeight - 1));

  roo_display::StringViewLabel drawable("abc", font.font(), color::Black,
                                        font.fontOptions());
  auto offset =
      ResolveAlignmentOffset(label.bounds(), Rect(drawable.anchorExtents()),
                             roo_display::kLeft | roo_display::kMiddle);
  Rect expected =
      Rect(drawable.extents()).translate(offset.first, offset.second);

  EXPECT_EQ(expected, label.getContentBounds());
}

// Verifies that vertical-middle gravity centers the above-baseline font body
// for both owning and string-view labels. Descent does not pull the requested
// center toward the baseline.
TEST(TextLabel, MiddleGravityCentersAscentForBothLabelKinds) {
  roo_scheduler::SchedulingService scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  const TextStyle& style = material2::text_style_subtitle1();
  constexpr int16_t kWidth = 80;
  constexpr int16_t kHeight = 48;

  TextLabel owned(context, "Wi-Fi", style, kGravityLeft | kGravityMiddle);
  StringViewLabel viewed(context, "Wi-Fi", style,
                         kGravityLeft | kGravityMiddle);
  owned.layout(Rect(0, 0, kWidth - 1, kHeight - 1));
  viewed.layout(Rect(0, 0, kWidth - 1, kHeight - 1));

  auto metrics =
      style.font().getHorizontalStringMetrics("Wi-Fi", style.fontOptions());
  const int16_t baseline = (kHeight + style.ascent()) / 2;
  Rect expected = Rect(metrics.screen_extents()).translate(0, baseline);

  EXPECT_EQ(expected, owned.getContentBounds());
  EXPECT_EQ(expected, viewed.getContentBounds());
}

// Verifies both label kinds align using drawable anchors but retain actual
// italic ink overhangs outside widget bounds. Style leading still
// controls minimum height; tracking affects both alignment and ink metrics.
TEST(TextLabel, OverhangingInkUsesDrawableExtentsForAllGravities) {
  roo_scheduler::SchedulingService scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  for (int16_t leading : {0, 7}) {
    TextStyle style(font_NotoSerif_Italic_40(), leading, 2);
    roo_display::StringViewLabel drawable(kOverhangText, style.font(),
                                          color::Black, style.fontOptions());
    ASSERT_LT(drawable.extents().xMin(), drawable.anchorExtents().xMin());
    for (HorizontalGravity horizontal :
         {kGravityLeft, kGravityCenter, kGravityRight}) {
      for (VerticalGravity vertical :
           {kGravityTop, kGravityMiddle, kGravityBottom}) {
        Gravity gravity = horizontal | vertical;
        TextLabel owned(context, kOverhangText, style, gravity);
        StringViewLabel viewed(context, kOverhangText, style, gravity);
        owned.setPadding(PaddingSize::kNone);
        viewed.setPadding(PaddingSize::kNone);
        EXPECT_EQ(owned.getSuggestedMinimumDimensions().height(),
                  style.lineHeight());
        for (int width :
             {static_cast<int>(drawable.anchorExtents().width()), 140}) {
          Rect bounds(0, 0, width - 1, 79);
          owned.layout(bounds);
          viewed.layout(bounds);
          auto offset = ResolveAlignmentOffset(
              bounds, Rect(drawable.anchorExtents()), gravity.asAlignment());
          Rect expected =
              Rect(drawable.extents()).translate(offset.first, offset.second);
          EXPECT_EQ(expected, owned.getContentBounds());
          EXPECT_EQ(expected, viewed.getContentBounds());
        }
      }
    }
  }
}

// Verifies both label kinds clip long text at each horizontal gravity, so
// their ink and direct-paint exclusions cannot reach adjacent layout slots.
TEST(TextLabel, ConstrainedTextClipsHorizontalInkForBothLabelKinds) {
  roo_scheduler::SchedulingService scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  TextStyle style(font_NotoSerif_Italic_40(), 0, 2);
  for (HorizontalGravity horizontal :
       {kGravityLeft, kGravityCenter, kGravityRight}) {
    TextLabel owned(context, kOverhangText, style, horizontal | kGravityMiddle);
    StringViewLabel viewed(context, kOverhangText, style,
                           horizontal | kGravityMiddle);
    ASSERT_GT(owned.getSuggestedMinimumDimensions().width(), 32);
    for (Widget* label :
         {static_cast<Widget*>(&owned), static_cast<Widget*>(&viewed)}) {
      label->layout(Rect(0, 0, 31, 79));
      Rect ink = label->getContentBounds();
      EXPECT_FALSE(ink.empty());
      EXPECT_GE(ink.xMin(), 0);
      EXPECT_LE(ink.xMax(), 31);
    }
  }
}

// Verifies actual painting of constrained owned and borrowed labels leaves
// the background on both sides intact, including after clearing the text.
TEST_F(TextLabelRenderTest, ConstrainedLabelsDoNotPaintOutsideTheirSlots) {
  const TextStyle& style = material2::text_style_body2();
  auto owned =
      std::make_unique<TextLabel>(context(), "Long label text", style,
                                  color::Black, kGravityLeft | kGravityMiddle);
  auto viewed = std::make_unique<StringViewLabel>(
      context(), "Long label text", style, color::Black,
      kGravityRight | kGravityMiddle);
  TextLabel* owned_ptr = owned.get();
  StringViewLabel* viewed_ptr = viewed.get();
  app_.add(std::move(owned), Box(30, 0, 49, 29));
  app_.add(std::move(viewed), Box(30, 32, 49, 61));
  refresh();
  const Color background =
      QuantizeToArgb4444(context().theme().material3Theme().color.background);
  EXPECT_TRUE(findNonBackground(30, 0, 49, 29, background).found);
  EXPECT_TRUE(findNonBackground(30, 32, 49, 61, background).found);
  EXPECT_FALSE(findNonBackground(0, 0, 29, 63, background).found);
  EXPECT_FALSE(findNonBackground(50, 0, 95, 63, background).found);
  owned_ptr->clearText();
  viewed_ptr->clearText();
  refresh();
  EXPECT_FALSE(findNonBackground(0, 0, 95, 63, background).found);
}

// Verifies logical sizing uses advance and line height while reported ink
// retains negative bearings and descenders outside those logical bounds.
TEST(TextLabel, LogicalSizeAndInkBoundsAreIndependent) {
  roo_scheduler::SchedulingService scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  TextStyle style(font_NotoSerif_Italic_40(), 0, 2);
  const auto metrics = style.font().getHorizontalStringMetrics(
      kOverhangText, style.fontOptions());
  for (HorizontalGravity h : {kGravityLeft, kGravityCenter, kGravityRight}) {
    for (VerticalGravity v : {kGravityTop, kGravityMiddle, kGravityBottom}) {
      TextLabel owned(context, kOverhangText, style, h | v);
      StringViewLabel viewed(context, kOverhangText, style, h | v);
      const Dimensions size = owned.getSuggestedMinimumDimensions();
      EXPECT_EQ(metrics.advance(), size.width());
      EXPECT_EQ(style.lineHeight(), size.height());
      const Rect bounds(0, 0, size.width() - 1, size.height() - 1);
      owned.layout(bounds);
      viewed.layout(bounds);
      EXPECT_EQ(metrics.width(), owned.getContentBounds().width());
      EXPECT_EQ(metrics.height(), owned.getContentBounds().height());
      EXPECT_EQ(owned.getContentBounds(), viewed.getContentBounds());
    }
  }
}

// Verifies that an empty TextLabel (both std::string and string_view flavors)
// reports empty ink bounds, so it excludes no background pixels.
TEST(TextLabel, EmptyTextHasNoInkBounds) {
  roo_scheduler::SchedulingService scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  const auto& font = material2::text_style_body2();

  TextLabel label(context, "", font, kGravityLeft | kGravityMiddle);
  StringViewLabel string_view_label(context, roo::string_view(), font,
                                    kGravityLeft | kGravityMiddle);

  label.layout(Rect(0, 0, 79, 39));
  string_view_label.layout(Rect(0, 0, 79, 39));
  EXPECT_TRUE(label.getContentBounds().empty());
  EXPECT_TRUE(string_view_label.getContentBounds().empty());
}

// Verifies that transitioning from empty to non-empty text avoids invalidating
// the parent: only the new ink region needs painting and there is nothing to
// erase beneath, so the panel records no invalidation regions.
TEST(TextLabel, EmptyToNonEmptyDoesNotInvalidateParentBeneath) {
  roo_scheduler::SchedulingService scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  RecordingPanel panel(context);

  auto label =
      std::make_unique<TextLabel>(context, "", material2::text_style_body2(),
                                  kGravityCenter | kGravityMiddle);
  TextLabel* label_ptr = label.get();
  panel.add(std::move(label), Rect(0, 0, 119, 19));
  panel.invalidated_regions.clear();

  label_ptr->setText("42.0");

  EXPECT_TRUE(panel.invalidated_regions.empty());
}

// Verifies that changing text from a small string to a larger one invalidates
// exactly the previous (smaller) visual rect on the parent; the new larger
// rect is repainted by the label itself.
TEST(TextLabel, TextChangeInvalidatesOnlyOldVisualBounds) {
  roo_scheduler::SchedulingService scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  RecordingPanel panel(context);

  auto label =
      std::make_unique<TextLabel>(context, "A", material2::text_style_body2(),
                                  kGravityCenter | kGravityMiddle);
  TextLabel* label_ptr = label.get();
  panel.add(std::move(label), Rect(0, 0, 119, 19));
  Rect old_bounds = label_ptr->maxParentBounds();
  panel.invalidated_regions.clear();

  label_ptr->setText("MMMMMMMM");

  ASSERT_EQ(1u, panel.invalidated_regions.size());
  EXPECT_EQ(old_bounds, panel.invalidated_regions.front());
}

// Verifies that swapping the text for another value of identical measured
// dimensions does not trigger a layout request, avoiding needless layout
// passes for incremental value updates (e.g., '72%' -> '73%').
TEST(TextLabel, SameMeasuredSizeTextChangeDoesNotRequestLayout) {
  roo_scheduler::SchedulingService scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  const auto& font = material2::text_style_body2();
  TextLabel label(context, "72%", font, kGravityLeft | kGravityMiddle);
  TextLabel comparison(context, "73%", font, kGravityLeft | kGravityMiddle);

  Dimensions initial = label.getSuggestedMinimumDimensions();
  Dimensions updated = comparison.getSuggestedMinimumDimensions();
  ASSERT_EQ(initial.width(), updated.width());
  ASSERT_EQ(initial.height(), updated.height());

  label.measure(WidthSpec::Exactly(64), HeightSpec::Exactly(24));
  label.layout(Rect(0, 0, 63, 23));

  label.setText("73%");

  EXPECT_FALSE(label.isLayoutRequested());
}

// Verifies the same equal-measured-size invariant for StringViewLabel: a
// text swap to a string of the same measured dimensions does not request a
// new layout pass.
TEST(StringViewLabel, SameMeasuredSizeTextChangeDoesNotRequestLayout) {
  roo_scheduler::SchedulingService scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  const auto& font = material2::text_style_body2();
  StringViewLabel label(context, roo::string_view("72%"), font,
                        kGravityLeft | kGravityMiddle);
  StringViewLabel comparison(context, roo::string_view("73%"), font,
                             kGravityLeft | kGravityMiddle);

  Dimensions initial = label.getSuggestedMinimumDimensions();
  Dimensions updated = comparison.getSuggestedMinimumDimensions();
  ASSERT_EQ(initial.width(), updated.width());
  ASSERT_EQ(initial.height(), updated.height());

  label.measure(WidthSpec::Exactly(64), HeightSpec::Exactly(24));
  label.layout(Rect(0, 0, 63, 23));

  label.setText(roo::string_view("73%"));

  EXPECT_FALSE(label.isLayoutRequested());
}

// Verifies overhanging ink is painted and excluded from the ancestor's
// background pass, then fully erased when the label is cleared.
TEST_F(TextLabelRenderTest, PaintAndClearInkOutsideLogicalBounds) {
  TextStyle style(font_NotoSerif_Italic_40(), 0, 0);
  auto label = std::make_unique<TextLabel>(context(), "j", style, color::Black,
                                           kGravityLeft | kGravityMiddle);
  TextLabel* label_ptr = label.get();
  const Dimensions size = label->getSuggestedMinimumDimensions();
  const Box logical(30, 4, 30 + size.width() - 1, 4 + size.height() - 1);
  app_.add(std::move(label), logical);
  refresh();
  ASSERT_LT(label_ptr->getContentBounds().xMin(), 0);

  roo::byte reference_pixels[kWidth * kHeight * 2];
  OffscreenDevice<Argb4444> reference_device(kWidth, kHeight, reference_pixels,
                                             Argb4444());
  Display reference_display(reference_device);
  const Color background = context().theme().material3Theme().color.background;
  reference_display.init(background);
  {
    DrawingContext dc(reference_display);
    dc.setBackgroundColor(background);
    dc.draw(roo_display::StringViewLabel("j", style.font(), color::Black), 30,
            4 + (size.height() + style.ascent()) / 2);
  }
  for (int16_t y = 0; y < kHeight; ++y) {
    for (int16_t x = 0; x < kWidth; ++x) {
      Color expected;
      reference_device.raster().readColors(&x, &y, 1, &expected);
      ASSERT_EQ(expected, pixelAt(x, y)) << "pixel " << x << ", " << y;
    }
  }
  label_ptr->clearText();
  refresh();
  for (int16_t y = 0; y < kHeight; ++y) {
    for (int16_t x = 0; x < kWidth; ++x) {
      ASSERT_EQ(QuantizeToArgb4444(background), pixelAt(x, y))
          << "cleared pixel " << x << ", " << y;
    }
  }
}

// Verifies that setText() and clearText() actually change rendered pixels:
// the painted ink region grows when text grows, and disappears entirely
// when the text is cleared.
TEST_F(TextLabelRenderTest, SetTextAndClearTextAffectRenderedPixels) {
  auto label = std::make_unique<TextLabel>(context(), "A",
                                           material2::text_style_body2());
  TextLabel* label_ptr = label.get();
  app_.add(std::move(label), Box(8, 8, 80, 32));

  refresh();
  Color bg =
      QuantizeToArgb4444(context().theme().material3Theme().color.background);
  PixelBounds initial = findNonBackground(8, 8, 80, 32, bg);
  ASSERT_TRUE(initial.found);

  label_ptr->setText("MMMMMMMM");
  refresh();
  PixelBounds expanded = findNonBackground(8, 8, 80, 32, bg);
  ASSERT_TRUE(expanded.found);
  EXPECT_GT(expanded.x_max - expanded.x_min, initial.x_max - initial.x_min);

  label_ptr->clearText();
  refresh();
  PixelBounds cleared = findNonBackground(8, 8, 80, 32, bg);
  EXPECT_FALSE(cleared.found);
}

// Verifies that horizontal gravity is honored at paint time: a right-gravity
// label paints its ink farther right than a left-gravity label of the same
// text within an equally-sized box.
TEST_F(TextLabelRenderTest, GravityChangesHorizontalPlacement) {
  auto left = std::make_unique<TextLabel>(context(), "WWW",
                                          material2::text_style_body2(),
                                          kGravityLeft | kGravityMiddle);
  auto right = std::make_unique<TextLabel>(context(), "WWW",
                                           material2::text_style_body2(),
                                           kGravityRight | kGravityMiddle);
  left->setPadding(PaddingSize::kNone);
  right->setPadding(PaddingSize::kNone);

  app_.add(std::move(left), Box(4, 6, 44, 28));
  app_.add(std::move(right), Box(4, 34, 44, 56));

  refresh();
  Color bg =
      QuantizeToArgb4444(context().theme().material3Theme().color.background);
  PixelBounds left_bounds = findNonBackground(4, 6, 44, 28, bg);
  PixelBounds right_bounds = findNonBackground(4, 34, 44, 56, bg);

  ASSERT_TRUE(left_bounds.found);
  ASSERT_TRUE(right_bounds.found);
  EXPECT_GT(right_bounds.x_min, left_bounds.x_min);
}

// Verifies that omitting the color argument (Color::Transparent default)
// produces pixels identical to passing the theme's onBackground color
// explicitly, confirming the transparent sentinel resolves to the theme.
TEST_F(TextLabelRenderTest, TransparentColorMatchesExplicitDefaultColor) {
  auto implicit = std::make_unique<TextLabel>(context(), "Hi",
                                              material2::text_style_body2(),
                                              kGravityLeft | kGravityMiddle);
  auto explicit_default = std::make_unique<TextLabel>(
      context(), "Hi", material2::text_style_body2(),
      context().theme().material3Theme().color.onBackground,
      kGravityLeft | kGravityMiddle);

  app_.add(std::move(implicit), Box(4, 6, 44, 28));
  app_.add(std::move(explicit_default), Box(4, 34, 44, 56));

  refresh();
  Color bg =
      QuantizeToArgb4444(context().theme().material3Theme().color.background);
  PixelBounds top = findNonBackground(4, 6, 44, 28, bg);
  PixelBounds bottom = findNonBackground(4, 34, 44, 56, bg);
  ASSERT_TRUE(top.found);
  ASSERT_TRUE(bottom.found);

  int16_t dx = top.x_min - 4;
  int16_t dy = top.y_min - 6;
  EXPECT_EQ(pixelAt(top.x_min, top.y_min), pixelAt(4 + dx, 34 + dy));
}

// Verifies the golden image for a centered, natural-size label placed in a
// FlexLayout: the ink overhang renders correctly within the parent layout.
TEST_F(TextLabelGoldenTest, CenteredNaturalSizeOverhangGolden) {
  auto image =
      RenderFlexLabel(kGravityCenter | kGravityMiddle, FlexLayout::Params{});

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_label/flex_center_natural.ppm",
      "text_label_flex_center_natural"));
}

// Verifies the golden image for a fill-width, left-gravity label in a
// FlexLayout, including any horizontal ink overhang at the leading edge.
TEST_F(TextLabelGoldenTest, FillWidthLeftGravityOverhangGolden) {
  auto image =
      RenderFlexLabel(kGravityLeft | kGravityMiddle, FillWidthParams());

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_label/flex_fill_left.ppm",
      "text_label_flex_fill_left"));
}

// Verifies the golden image for a fill-width, center-gravity label in a
// FlexLayout, exercising symmetric ink overhang on both sides.
TEST_F(TextLabelGoldenTest, FillWidthCenterGravityOverhangGolden) {
  auto image =
      RenderFlexLabel(kGravityCenter | kGravityMiddle, FillWidthParams());

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_label/flex_fill_center.ppm",
      "text_label_flex_fill_center"));
}

// Verifies the golden image for a fill-width, right-gravity label in a
// FlexLayout, including any horizontal ink overhang at the trailing edge.
TEST_F(TextLabelGoldenTest, FillWidthRightGravityOverhangGolden) {
  auto image =
      RenderFlexLabel(kGravityRight | kGravityMiddle, FillWidthParams());

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_label/flex_fill_right.ppm",
      "text_label_flex_fill_right"));
}

// Verifies the golden image for a fill-width left-gravity label that has
// small widget-level padding applied, so the ink starts inside (not at) the
// allocated content rect's edge.
TEST_F(TextLabelGoldenTest, FillWidthLeftGravityWithWidgetPaddingGolden) {
  auto image = RenderFlexLabel(kGravityLeft | kGravityMiddle, FillWidthParams(),
                               PaddingSize::kSmall);

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_label/flex_fill_left_widget_padding.ppm",
      "text_label_flex_fill_left_widget_padding"));
}

}  // namespace roo_windows
