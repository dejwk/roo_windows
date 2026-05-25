#include "roo_windows/widgets/text_block.h"

#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace {

ApplicationContext MakeContext(Environment& context) {
  return ApplicationContext(context.scheduler(), context.theme(),
                            context.keyboardColorTheme());
}

constexpr char kOverhangText[] = "jQy/";

FlexLayout::Params FillWidthParams() {
  FlexLayout::Params params;
  params.flex_grow = 1;
  params.flex_basis = FlexBasis::kZero;
  return params;
}

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

// Verifies that with word-wrap enabled, measuring a multi-word string under a
// narrow AtMost width yields a measured height greater than a single line's
// height, confirming the wrap actually grew the layout vertically.
TEST(TextBlock, WordWrapIncreasesHeightWithNarrowWidth) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  TextBlock block(context, "word word word word", font_body2(),
                  roo_display::kLeft | roo_display::kTop);
  block.setWrapMode(TextWrapMode::kWordWrap);

  Dimensions dims =
      block.measure(WidthSpec::AtMost(32), HeightSpec::Unspecified(1000));
  EXPECT_GT(dims.height(), block.font().metrics().maxHeight());
}

// Verifies that with wrapping disabled, the measured height of multi-word
// text stays at exactly one line height even when the width spec is narrower
// than the text's natural advance.
TEST(TextBlock, NoWrapKeepsSingleLineHeight) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  TextBlock block(context, "word word word word", font_body2(),
                  roo_display::kLeft | roo_display::kTop);
  block.setWrapMode(TextWrapMode::kNoWrap);

  Dimensions dims =
      block.measure(WidthSpec::AtMost(32), HeightSpec::Unspecified(1000));
  EXPECT_EQ(block.font().metrics().maxHeight(), dims.height());
}

// Verifies that setMaxLines(N) caps the measured height to at most N line
// heights, even if the wrapped text would otherwise span more lines.
TEST(TextBlock, MaxLinesLimitsMeasuredHeight) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  TextBlock block(context, "word word word word word word", font_body2(),
                  roo_display::kLeft | roo_display::kTop);
  block.setWrapMode(TextWrapMode::kWordWrap);
  block.setMaxLines(2);

  Dimensions dims =
      block.measure(WidthSpec::AtMost(36), HeightSpec::Unspecified(1000));
  EXPECT_LE(dims.height(), 2 * block.font().metrics().maxHeight());
}

// Verifies that after laying out a non-wrapped single-line block, the
// content bounds reflect the actual glyph ink extents (clamped to include
// the bounding rectangle) rather than the nominal advance rectangle.
TEST(TextBlock, SingleLineContentBoundsReflectFontInk) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  const auto& font = font_body2();
  TextBlock block(context, "abc", font, roo_display::kLeft | roo_display::kTop);
  block.setPadding(PaddingSize::kNone);
  block.setWrapMode(TextWrapMode::kNoWrap);

  auto metrics = font.getHorizontalStringMetrics("abc");
  int16_t advance = metrics.advance();
  int16_t line_height = font.metrics().maxHeight();
  block.layout(Rect(0, 0, advance - 1, line_height - 1));

  EXPECT_EQ(Rect(std::min<int16_t>(0, metrics.glyphXMin()), 0,
                 std::max<int16_t>(advance - 1, metrics.glyphXMax()),
                 line_height - 1),
            block.getContentBounds());
}

// Verifies that with a pending re-layout after setText() the reported
// content bounds fall back to conservative font-level bounds (covering all
// glyphs and rsb overhang) instead of the previous laid-out extents.
TEST(TextBlock, PendingWrappedLayoutUsesConservativeFontBounds) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  const auto& font = font_body2();
  TextBlock block(context, "abc", font, roo_display::kLeft | roo_display::kTop);
  block.setPadding(PaddingSize::kNone);

  int16_t line_height = font.metrics().maxHeight();
  block.layout(Rect(0, 0, 79, 2 * line_height - 1));

  block.setText("ab");

  EXPECT_EQ(Rect(std::min<int16_t>(0, font.metrics().glyphXMin()), 0,
                 79 + std::max<int16_t>(0, -font.metrics().minRsb()),
                 2 * line_height - 1),
            block.getContentBounds());
}

// Verifies that setPadding() flags the block as needing a new layout pass
// and, once re-laid-out, shifts the content bounds inward by the padding on
// both the left and top edges.
TEST(TextBlock, SetPaddingRequestsLayoutAndUpdatesContentBounds) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  const auto& font = font_body2();
  TextBlock block(context, "abc", font, roo_display::kLeft | roo_display::kTop);
  block.setWrapMode(TextWrapMode::kNoWrap);
  block.setPadding(PaddingSize::kNone);

  auto metrics = font.getHorizontalStringMetrics("abc");
  int16_t advance = metrics.advance();
  int16_t line_height = font.metrics().maxHeight();
  block.layout(Rect(0, 0, advance + 15, line_height + 11));

  Rect without_padding = block.getContentBounds();

  block.setPadding(PaddingSize::kSmall);
  EXPECT_TRUE(block.isLayoutRequested());
  block.measure(WidthSpec::Exactly(advance + 16),
                HeightSpec::Exactly(line_height + 12));
  block.layout(block.parent_bounds());

  EXPECT_NE(without_padding, block.getContentBounds());
  EXPECT_GT(block.getContentBounds().xMin(), without_padding.xMin());
  EXPECT_GT(block.getContentBounds().yMin(), without_padding.yMin());
}

// Verifies that transitioning a TextBlock from empty to non-empty text does
// not request invalidation from the parent panel: the new ink is repainted
// by the block itself with nothing to erase beneath.
TEST(TextBlock, EmptyToNonEmptyDoesNotInvalidateParentBeneath) {
  roo_scheduler::Scheduler scheduler;
  Environment bootstrap(scheduler);
  ApplicationContext context = MakeContext(bootstrap);
  RecordingPanel panel(context);

  auto block = std::make_unique<TextBlock>(
      context, "", font_body2(), roo_display::kCenter | roo_display::kMiddle);
  TextBlock* block_ptr = block.get();
  panel.add(std::move(block), Rect(0, 0, 159, 39));
  panel.invalidated_regions.clear();

  block_ptr->setText("42.0 C");

  EXPECT_TRUE(panel.invalidated_regions.empty());
}

class TextBlockGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 360;
  static constexpr int16_t kHeight = 200;
  static constexpr int16_t kFlexLayoutX = 8;
  static constexpr int16_t kFlexLayoutY = 8;
  static constexpr int16_t kFlexLayoutWidth = 128;
  static constexpr int16_t kFlexLayoutHeight = 48;

  TextBlockGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_) {}

  roo_display::Offscreen<roo_display::Rgb888> RenderBlock(
      const std::string& text, TextAlign align, int16_t box_width,
      int16_t max_height = 0, uint16_t max_lines = 0, bool ellipsize = false) {
    Application app(&env_, display_);

    TextBlock block(app.context(), text, font_body2(),
                    roo_display::kLeft | roo_display::kTop);
    block.setPadding(PaddingSize::kNone);
    block.setWrapMode(TextWrapMode::kWordWrap);
    block.setTextAlign(align);
    if (max_lines > 0) {
      block.setMaxLines(max_lines);
    }
    block.setEllipsize(ellipsize);

    Dimensions measured = block.measure(WidthSpec::Exactly(box_width),
                                        HeightSpec::Unspecified(kHeight));
    int16_t render_height = measured.height();
    if (max_height > 0) {
      render_height = std::min<int16_t>(render_height, max_height);
    }

    app.add(block,
            roo_display::Box(8, 8, 8 + box_width - 1, 8 + render_height - 1));
    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 8, 8, box_width,
                            render_height);
  }

  roo_display::Offscreen<roo_display::Rgb888> RenderFlexBlock(
      roo_display::Alignment alignment, const FlexLayout::Params& params,
      PaddingSize padding = PaddingSize::kNone) {
    Application app(&env_, display_);

    TextBlock block(app.context(), kOverhangText, font_body2(), alignment);
    block.setPadding(padding);
    block.setWrapMode(TextWrapMode::kNoWrap);

    FlexLayout layout(app.context(), FlexDirection::kRow);
    layout.setPadding(Padding(12, 8));
    layout.setJustifyContent(JustifyContent::kCenter);
    layout.setAlignItems(AlignItems::kCenter);
    layout.add(block, params);

    app.add(layout, roo_display::Box(kFlexLayoutX, kFlexLayoutY,
                                     kFlexLayoutX + kFlexLayoutWidth - 1,
                                     kFlexLayoutY + kFlexLayoutHeight - 1));
    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), kFlexLayoutX, kFlexLayoutY,
                            kFlexLayoutWidth, kFlexLayoutHeight);
  }

  roo::byte raster_[kWidth * kHeight * 2];
  roo_display::OffscreenDevice<roo_display::Argb4444> offscreen_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
};

// Verifies the golden image for a justified, word-wrapped paragraph: lines
// expand to flush with both edges except the final line.
TEST_F(TextBlockGoldenTest, JustifyParagraphGolden) {
  auto image = RenderBlock(
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
      "Vestibulum luctus justo id sem pulvinar, non venenatis velit luctus.",
      TextAlign::kJustify, 300);

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/justify_paragraph.ppm",
      "text_block_justify_paragraph"));
}

// Verifies the golden image for a justified paragraph that contains an
// explicit '\n' break: justification stops at the line preceding the break,
// and the post-break short line renders unjustified.
TEST_F(TextBlockGoldenTest, JustifyExplicitParagraphBreakGolden) {
  auto image = RenderBlock(
      "Alpha beta gamma delta epsilon zeta eta theta iota kappa.\n"
      "Short final line.",
      TextAlign::kJustify, 300);

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/justify_paragraph_break.ppm",
      "text_block_justify_paragraph_break"));
}

// Verifies the golden image for a non-justified (start-aligned) multi-line
// paragraph rendered without truncation.
TEST_F(TextBlockGoldenTest, NonJustifiedMultilineGolden) {
  auto image = RenderBlock(
      "This is a regular multiline text block rendered with start alignment. "
      "It should wrap naturally across lines and render without truncation.",
      TextAlign::kStart, 300);

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/non_justify_multiline.ppm",
      "text_block_non_justify_multiline"));
}

// Verifies the golden image for a justified paragraph rendered into a box
// whose height clips the bottom of the text: clipping happens cleanly
// without disturbing the justified line layout above.
TEST_F(TextBlockGoldenTest, JustifyParagraphClippedBottomGolden) {
  auto image = RenderBlock(
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
      "Vestibulum luctus justo id sem pulvinar, non venenatis velit luctus.",
      TextAlign::kJustify, 120, 72);

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/justify_paragraph_clipped.ppm",
      "text_block_justify_paragraph_clipped"));
}

// Verifies the golden image for ellipsization: text that would wrap to more
// than maxLines is truncated and terminated with an ellipsis on the final
// allowed line.
TEST_F(TextBlockGoldenTest, EllipsizeGolden) {
  auto image = RenderBlock(
      "This text should wrap to multiple lines but get truncated with an "
      "ellipsis once the max lines limit is reached.",
      TextAlign::kStart, 220, 0, 2, true);

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/ellipsize.ppm", "text_block_ellipsize"));
}

// Verifies the golden image for a centered, natural-size TextBlock placed
// in a FlexLayout, including any ink overhang on either side.
TEST_F(TextBlockGoldenTest, CenteredNaturalSizeOverhangGolden) {
  auto image = RenderFlexBlock(roo_display::kCenter | roo_display::kMiddle,
                               FlexLayout::Params{});

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/flex_center_natural.ppm",
      "text_block_flex_center_natural"));
}

// Verifies the golden image for a fill-width, left-gravity TextBlock in a
// FlexLayout, including any leading-edge ink overhang.
TEST_F(TextBlockGoldenTest, FillWidthLeftGravityOverhangGolden) {
  auto image = RenderFlexBlock(roo_display::kLeft | roo_display::kMiddle,
                               FillWidthParams());

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/flex_fill_left.ppm",
      "text_block_flex_fill_left"));
}

// Verifies the golden image for a fill-width, center-gravity TextBlock in a
// FlexLayout, exercising symmetric ink overhang on both sides.
TEST_F(TextBlockGoldenTest, FillWidthCenterGravityOverhangGolden) {
  auto image = RenderFlexBlock(roo_display::kCenter | roo_display::kMiddle,
                               FillWidthParams());

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/flex_fill_center.ppm",
      "text_block_flex_fill_center"));
}

// Verifies the golden image for a fill-width, right-gravity TextBlock in a
// FlexLayout, including any trailing-edge ink overhang.
TEST_F(TextBlockGoldenTest, FillWidthRightGravityOverhangGolden) {
  auto image = RenderFlexBlock(roo_display::kRight | roo_display::kMiddle,
                               FillWidthParams());

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/flex_fill_right.ppm",
      "text_block_flex_fill_right"));
}

// Verifies the golden image for a fill-width left-gravity TextBlock with
// small widget-level padding applied, so ink starts inside the allocated
// content area's edge.
TEST_F(TextBlockGoldenTest, FillWidthLeftGravityWithWidgetPaddingGolden) {
  auto image = RenderFlexBlock(roo_display::kLeft | roo_display::kMiddle,
                               FillWidthParams(), PaddingSize::kSmall);

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/flex_fill_left_widget_padding.ppm",
      "text_block_flex_fill_left_widget_padding"));
}

}  // namespace
}  // namespace roo_windows
