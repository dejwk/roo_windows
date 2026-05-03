#include "roo_windows/widgets/text_block.h"

#include "golden_image.h"
#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace {

constexpr char kOverhangText[] = "jQy/";

FlexLayout::Params FillWidthParams() {
  FlexLayout::Params params;
  params.flex_grow = 1;
  params.flex_basis = FlexBasis::kZero;
  return params;
}

class RecordingPanel : public Panel {
 public:
  explicit RecordingPanel(const Environment& env) : Panel(env) {}

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

TEST(TextBlock, WordWrapIncreasesHeightWithNarrowWidth) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  TextBlock block(env, "word word word word", font_body2(),
                  roo_display::kLeft | roo_display::kTop);
  block.setWrapMode(TextWrapMode::kWordWrap);

  Dimensions dims =
      block.measure(WidthSpec::AtMost(32), HeightSpec::Unspecified(1000));
  EXPECT_GT(dims.height(), block.font().metrics().maxHeight());
}

TEST(TextBlock, NoWrapKeepsSingleLineHeight) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  TextBlock block(env, "word word word word", font_body2(),
                  roo_display::kLeft | roo_display::kTop);
  block.setWrapMode(TextWrapMode::kNoWrap);

  Dimensions dims =
      block.measure(WidthSpec::AtMost(32), HeightSpec::Unspecified(1000));
  EXPECT_EQ(block.font().metrics().maxHeight(), dims.height());
}

TEST(TextBlock, MaxLinesLimitsMeasuredHeight) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  TextBlock block(env, "word word word word word word", font_body2(),
                  roo_display::kLeft | roo_display::kTop);
  block.setWrapMode(TextWrapMode::kWordWrap);
  block.setMaxLines(2);

  Dimensions dims =
      block.measure(WidthSpec::AtMost(36), HeightSpec::Unspecified(1000));
  EXPECT_LE(dims.height(), 2 * block.font().metrics().maxHeight());
}

TEST(TextBlock, SingleLineContentBoundsReflectFontInk) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  const auto& font = font_body2();
  TextBlock block(env, "abc", font, roo_display::kLeft | roo_display::kTop);
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

TEST(TextBlock, PendingWrappedLayoutUsesConservativeFontBounds) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  const auto& font = font_body2();
  TextBlock block(env, "abc", font, roo_display::kLeft | roo_display::kTop);
  block.setPadding(PaddingSize::kNone);

  int16_t line_height = font.metrics().maxHeight();
  block.layout(Rect(0, 0, 79, 2 * line_height - 1));

  block.setText("ab");

  EXPECT_EQ(Rect(std::min<int16_t>(0, font.metrics().glyphXMin()), 0,
                 79 + std::max<int16_t>(0, -font.metrics().minRsb()),
                 2 * line_height - 1),
            block.getContentBounds());
}

TEST(TextBlock, SetPaddingRequestsLayoutAndUpdatesContentBounds) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  const auto& font = font_body2();
  TextBlock block(env, "abc", font, roo_display::kLeft | roo_display::kTop);
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

TEST(TextBlock, EmptyToNonEmptyDoesNotInvalidateParentBeneath) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  RecordingPanel panel(env);

  auto block = std::make_unique<TextBlock>(env, "", font_body2(),
                                           roo_display::kCenter |
                                               roo_display::kMiddle);
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
    TextBlock block(env_, text, font_body2(),
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

    Application app(&env_, display_);
    app.add(block,
            roo_display::Box(8, 8, 8 + box_width - 1, 8 + render_height - 1));
    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 8, 8, box_width,
                            render_height);
  }

  roo_display::Offscreen<roo_display::Rgb888> RenderFlexBlock(
      roo_display::Alignment alignment, const FlexLayout::Params& params,
      PaddingSize padding = PaddingSize::kNone) {
    TextBlock block(env_, kOverhangText, font_body2(), alignment);
    block.setPadding(padding);
    block.setWrapMode(TextWrapMode::kNoWrap);

    FlexLayout layout(env_, FlexDirection::kRow);
    layout.setPadding(Padding(12, 8));
    layout.setJustifyContent(JustifyContent::kCenter);
    layout.setAlignItems(AlignItems::kCenter);
    layout.add(block, params);

    Application app(&env_, display_);
    app.add(layout,
            roo_display::Box(kFlexLayoutX, kFlexLayoutY,
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

TEST_F(TextBlockGoldenTest, JustifyParagraphGolden) {
  auto image = RenderBlock(
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
      "Vestibulum luctus justo id sem pulvinar, non venenatis velit luctus.",
      TextAlign::kJustify, 300);

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/justify_paragraph.ppm",
      "text_block_justify_paragraph"));
}

TEST_F(TextBlockGoldenTest, JustifyExplicitParagraphBreakGolden) {
  auto image = RenderBlock(
      "Alpha beta gamma delta epsilon zeta eta theta iota kappa.\n"
      "Short final line.",
      TextAlign::kJustify, 300);

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/justify_paragraph_break.ppm",
      "text_block_justify_paragraph_break"));
}

TEST_F(TextBlockGoldenTest, NonJustifiedMultilineGolden) {
  auto image = RenderBlock(
      "This is a regular multiline text block rendered with start alignment. "
      "It should wrap naturally across lines and render without truncation.",
      TextAlign::kStart, 300);

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/non_justify_multiline.ppm",
      "text_block_non_justify_multiline"));
}

TEST_F(TextBlockGoldenTest, JustifyParagraphClippedBottomGolden) {
  auto image = RenderBlock(
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
      "Vestibulum luctus justo id sem pulvinar, non venenatis velit luctus.",
      TextAlign::kJustify, 120, 72);

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/justify_paragraph_clipped.ppm",
      "text_block_justify_paragraph_clipped"));
}

TEST_F(TextBlockGoldenTest, EllipsizeGolden) {
  auto image = RenderBlock(
      "This text should wrap to multiple lines but get truncated with an "
      "ellipsis once the max lines limit is reached.",
      TextAlign::kStart, 220, 0, 2, true);

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/ellipsize.ppm", "text_block_ellipsize"));
}

TEST_F(TextBlockGoldenTest, CenteredNaturalSizeOverhangGolden) {
  auto image = RenderFlexBlock(roo_display::kCenter | roo_display::kMiddle,
                               FlexLayout::Params{});

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/flex_center_natural.ppm",
      "text_block_flex_center_natural"));
}

TEST_F(TextBlockGoldenTest, FillWidthLeftGravityOverhangGolden) {
  auto image =
      RenderFlexBlock(roo_display::kLeft | roo_display::kMiddle,
                      FillWidthParams());

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/flex_fill_left.ppm",
      "text_block_flex_fill_left"));
}

TEST_F(TextBlockGoldenTest, FillWidthCenterGravityOverhangGolden) {
  auto image =
      RenderFlexBlock(roo_display::kCenter | roo_display::kMiddle,
                      FillWidthParams());

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/flex_fill_center.ppm",
      "text_block_flex_fill_center"));
}

TEST_F(TextBlockGoldenTest, FillWidthRightGravityOverhangGolden) {
  auto image =
      RenderFlexBlock(roo_display::kRight | roo_display::kMiddle,
                      FillWidthParams());

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/flex_fill_right.ppm",
      "text_block_flex_fill_right"));
}

TEST_F(TextBlockGoldenTest, FillWidthLeftGravityWithWidgetPaddingGolden) {
  auto image = RenderFlexBlock(roo_display::kLeft | roo_display::kMiddle,
                               FillWidthParams(), PaddingSize::kSmall);

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "test/goldens/text_block/flex_fill_left_widget_padding.ppm",
      "text_block_flex_fill_left_widget_padding"));
}

}  // namespace
}  // namespace roo_windows
