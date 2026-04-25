#include "roo_windows/widgets/text_block.h"

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "test/golden_image.h"
#include "roo_windows.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace {

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

class TextBlockGoldenTest : public testing::Test {
 protected:
  static constexpr int16_t kWidth = 360;
  static constexpr int16_t kHeight = 200;

  TextBlockGoldenTest()
      : offscreen_(kWidth, kHeight, raster_, roo_display::Argb4444()),
        display_(offscreen_),
        env_(scheduler_) {}

  roo_display::Offscreen<roo_display::Rgb888> RenderBlock(
      const std::string& text, TextAlign align, int16_t box_width,
      int16_t max_height = 0, uint16_t max_lines = 0,
      bool ellipsize = false) {
    TextBlock block(env_, text, font_body2(),
                    roo_display::kLeft | roo_display::kTop);
    block.setPadding(PaddingSize::kNone);
    block.setWrapMode(TextWrapMode::kWordWrap);
    block.setTextAlign(align);
    if (max_lines > 0) {
      block.setMaxLines(max_lines);
    }
    block.setEllipsize(ellipsize);

    Dimensions measured =
        block.measure(WidthSpec::Exactly(box_width), HeightSpec::Unspecified(kHeight));
    int16_t render_height = measured.height();
    if (max_height > 0) {
      render_height = std::min<int16_t>(render_height, max_height);
    }

    Application app(&env_, display_);
    app.add(WidgetRef(block),
            roo_display::Box(8, 8, 8 + box_width - 1, 8 + render_height - 1));
    EXPECT_TRUE(app.refresh());
    return test::CaptureRgb(offscreen_.raster(), 8, 8, box_width,
                            render_height);
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
      image, "lib/roo_windows/test/goldens/text_block/justify_paragraph.ppm",
      "text_block_justify_paragraph"));
}

TEST_F(TextBlockGoldenTest, JustifyExplicitParagraphBreakGolden) {
  auto image = RenderBlock(
      "Alpha beta gamma delta epsilon zeta eta theta iota kappa.\n"
      "Short final line.",
      TextAlign::kJustify, 300);

  EXPECT_TRUE(
      test::CompareOrUpdateGolden(
          image,
          "lib/roo_windows/test/goldens/text_block/justify_paragraph_break.ppm",
          "text_block_justify_paragraph_break"));
}

TEST_F(TextBlockGoldenTest, NonJustifiedMultilineGolden) {
  auto image = RenderBlock(
      "This is a regular multiline text block rendered with start alignment. "
      "It should wrap naturally across lines and render without truncation.",
      TextAlign::kStart, 300);

  EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "lib/roo_windows/test/goldens/text_block/non_justify_multiline.ppm",
      "text_block_non_justify_multiline"));
}

TEST_F(TextBlockGoldenTest, JustifyParagraphClippedBottomGolden) {
  auto image = RenderBlock(
      "Lorem ipsum dolor sit amet, consectetur adipiscing elit. "
      "Vestibulum luctus justo id sem pulvinar, non venenatis velit luctus.",
      TextAlign::kJustify, 120, 72);

  EXPECT_TRUE(test::CompareOrUpdateGolden(
    image,
    "lib/roo_windows/test/goldens/text_block/justify_paragraph_clipped.ppm",
    "text_block_justify_paragraph_clipped"));
}

  TEST_F(TextBlockGoldenTest, EllipsizeGolden) {
    auto image = RenderBlock(
      "This text should wrap to multiple lines but get truncated with an "
      "ellipsis once the max lines limit is reached.",
      TextAlign::kStart, 220, 0, 2, true);

    EXPECT_TRUE(test::CompareOrUpdateGolden(
      image, "lib/roo_windows/test/goldens/text_block/ellipsize.ppm",
      "text_block_ellipsize"));
  }

}  // namespace
}  // namespace roo_windows
