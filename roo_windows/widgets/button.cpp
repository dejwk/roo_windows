#include "button.h"

#include "roo_display/core/raster.h"
#include "roo_display/shape/basic_shapes.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"

using namespace roo_display;

namespace roo_windows {

namespace {

struct BorderSpec {
  int8_t top_width;
  int8_t top_height;
  int8_t left_width;
  int8_t left_height;
  int8_t right_width;
  int8_t right_height;
  int8_t bottom_width;
  int8_t bottom_height;
  const uint8_t* PROGMEM data_top_left;
  const uint8_t* PROGMEM data_top;
  const uint8_t* PROGMEM data_top_right;
  const uint8_t* PROGMEM data_left_top;
  const uint8_t* PROGMEM data_left;
  const uint8_t* PROGMEM data_left_bottom;
  const uint8_t* PROGMEM data_right_top;
  const uint8_t* PROGMEM data_right;
  const uint8_t* PROGMEM data_right_bottom;
  const uint8_t* PROGMEM data_bottom_left;
  const uint8_t* PROGMEM data_bottom;
  const uint8_t* PROGMEM data_bottom_right;
};

static const PROGMEM uint8_t kContainedLeftData[] = {0x04, 0xAE};
static const PROGMEM uint8_t kContainedRightData[] = {0xEA, 0x40};
static const PROGMEM uint8_t kContainedFillData[] = {0xFF};
static const PROGMEM uint8_t kContainedTopData[] = {0x4A, 0xE0};
static const PROGMEM uint8_t kContainedBottomData[] = {0xEA, 0x40};

static const PROGMEM BorderSpec kContainedBorder = {
    .top_width = 4,
    .top_height = 1,
    .left_width = 1,
    .left_height = 3,
    .right_width = 1,
    .right_height = 3,
    .bottom_width = 4,
    .bottom_height = 1,
    .data_top_left = kContainedLeftData,
    .data_top = kContainedFillData,
    .data_top_right = kContainedRightData,
    .data_left_top = kContainedTopData,
    .data_left = kContainedFillData,
    .data_left_bottom = kContainedBottomData,
    .data_right_top = kContainedTopData,
    .data_right = kContainedFillData,
    .data_right_bottom = kContainedBottomData,
    .data_bottom_left = kContainedLeftData,
    .data_bottom = kContainedFillData,
    .data_bottom_right = kContainedRightData};

static const PROGMEM uint8_t kOutlinedTopLeftData[] = {0x01, 0x57, 0x27, 0x52};
static const PROGMEM uint8_t kOutlinedTopData[] = {0x71};
static const PROGMEM uint8_t kOutlinedTopRightData[] = {0x75, 0x10, 0x25, 0x71};
static const PROGMEM uint8_t kOutlinedBottomLeftData[] = {0x17, 0x52, 0x01,
                                                          0x57};
static const PROGMEM uint8_t kOutlinedBottomData[] = {0x17};
static const PROGMEM uint8_t kOutlinedBottomRightData[] = {0x25, 0x71, 0x75,
                                                           0x10};
static const PROGMEM uint8_t kOutlinedLeftTopData[] = {0x55, 0x72};
static const PROGMEM uint8_t kOutlinedLeftData[] = {0x71};
static const PROGMEM uint8_t kOutlinedLeftBottomData[] = {0x72, 0x55};
static const PROGMEM uint8_t kOutlinedRightTopData[] = {0x55, 0x27};
static const PROGMEM uint8_t kOutlinedRightData[] = {0x17};
static const PROGMEM uint8_t kOutlinedRightBottomData[] = {0x27, 0x55};

static const PROGMEM BorderSpec kOutlinedBorder = {
    .top_width = 4,
    .top_height = 2,
    .left_width = 2,
    .left_height = 2,
    .right_width = 2,
    .right_height = 2,
    .bottom_width = 4,
    .bottom_height = 2,
    .data_top_left = kOutlinedTopLeftData,
    .data_top = kOutlinedTopData,
    .data_top_right = kOutlinedTopRightData,
    .data_left_top = kOutlinedLeftTopData,
    .data_left = kOutlinedLeftData,
    .data_left_bottom = kOutlinedLeftBottomData,
    .data_right_top = kOutlinedRightTopData,
    .data_right = kOutlinedRightData,
    .data_right_bottom = kOutlinedRightBottomData,
    .data_bottom_left = kOutlinedBottomLeftData,
    .data_bottom = kOutlinedBottomData,
    .data_bottom_right = kOutlinedBottomRightData};

class Interior : public Drawable {
 public:
  Interior(const Button& button, const Box& bounds)
      : button_(button),
        bounds_(bounds),
        label_(*button.theme().font.button, button.label(), button.textColor(),
               roo_display::FILL_MODE_RECTANGLE) {}

  Box extents() const override { return bounds_; }

 private:
  bool hasIcon() const { return button_.hasIcon(); }
  bool hasText() const { return !label_.label().empty(); }

  void drawTo(const Surface& s) const override {
    if (!hasIcon() && !hasText()) {
      s.drawObject(roo_display::FilledRect(bounds_.xMin(), bounds_.yMin(),
                                           bounds_.xMax(), bounds_.yMax(),
                                           button_.interiorColor()));
      return;
    }
    if (!hasText()) {
      // Icon only.
      MonoIcon icon = button_.icon();
      icon.color_mode().setColor(button_.textColor());
      roo_display::Tile tile(&icon, bounds_, HAlign::Center(), VAlign::Middle(),
                             button_.interiorColor(), s.fill_mode());
      s.drawObject(tile);
      return;
    }
    if (!hasIcon()) {
      roo_display::Tile tile(&label_, bounds_, HAlign::Center(),
                             VAlign::Middle(), button_.interiorColor(),
                             s.fill_mode());
      s.drawObject(tile);
      return;
    }
    // Both text and icon.
    MonoIcon icon = button_.icon();
    icon.color_mode().setColor(button_.textColor());
    int16_t gap = (icon.extents().width() / 2) & 0xFFFC;
    int16_t x_offset = (bounds_.width() - icon.extents().width() -
                        label_.extents().width() - gap) /
                       2;
    int16_t x_cursor = bounds_.xMin();
    const int16_t yMin = bounds_.yMin();
    const int16_t yMax = bounds_.yMax();
    roo_display::Color c = button_.interiorColor();
    // Left border.
    if (x_offset > 0) {
      s.drawObject(roo_display::FilledRect(x_cursor, yMin,
                                           x_cursor + x_offset - 1, yMax, c));
    }
    x_cursor += x_offset;  // Note: x_offset may be negative.
    // Icon.
    {
      roo_display::Box box(x_cursor, yMin,
                           x_cursor + icon.extents().width() - 1, yMax);
      roo_display::Tile tile(&icon, box, HAlign::Center(), VAlign::Middle(),
                             button_.interiorColor(), s.fill_mode());
      s.drawObject(tile);
    }
    x_cursor += icon.extents().width();
    // Gap.
    s.drawObject(
        roo_display::FilledRect(x_cursor, yMin, x_cursor + gap - 1, yMax, c));
    x_cursor += gap;
    // Text.
    {
      roo_display::Box box(x_cursor, yMin,
                           x_cursor + label_.extents().width() - 1, yMax);
      roo_display::Tile tile(&label_, box, HAlign::Center(), VAlign::Middle(),
                             button_.interiorColor(), s.fill_mode());
      s.drawObject(tile);
    }
    x_cursor += label_.extents().width();
    // Right border.
    if (x_offset <= bounds_.xMax()) {
      s.drawObject(
          roo_display::FilledRect(x_cursor, yMin, bounds_.xMax(), yMax, c));
    }
  }

  const Button& button_;
  roo_display::Box bounds_;
  roo_display::TextLabel label_;
};

void paintInterior(const Surface& s, const Box& bounds, const Button& button) {
  Interior interior(button, bounds);
  s.drawObject(interior);
}

void printVertStripes(const Surface& s, int16_t xMin, int16_t yMin,
                      int16_t xMax, uint8_t count,
                      const uint8_t* PROGMEM alpha4, Color base_color) {
  Alpha4 color_mode(base_color);
  typename Raster<const uint8_t * PROGMEM, Alpha4>::StreamType stream(
      alpha4, color_mode);
  for (int i = 0; i < count; ++i) {
    Color c = stream.next();
    s.drawObject(Line(xMin, yMin + i, xMax, yMin + i, c));
  }
}

void printHorizStripes(const Surface& s, int16_t xMin, int16_t yMin,
                       int16_t yMax, uint8_t count,
                       const uint8_t* PROGMEM alpha4, Color base_color) {
  Alpha4 color_mode(base_color);
  typename Raster<const uint8_t * PROGMEM, Alpha4>::StreamType stream(
      alpha4, color_mode);
  for (int i = 0; i < count; ++i) {
    Color c = stream.next();
    s.drawObject(Line(xMin + i, yMin, xMin + i, yMax, c));
  }
}

}  // namespace

Button::Button(const Environment& env, const MonoIcon* icon, std::string label,
               Style style)
    : Widget(env),
      style_(style),
      label_(std::move(label)),
      icon_(icon),
      outlineColor_(style == CONTAINED  ? env.theme().color.primary
                    : style == OUTLINED ? env.theme().color.onBackground
                                        : color::Transparent),
      interiorColor_(style == CONTAINED ? env.theme().color.primary
                                        : env.theme().color.background),
      textColor_(style == CONTAINED ? env.theme().color.onPrimary
                                    : env.theme().color.primary) {}

// 111111_2_2_2_2_333333
// 111111_2_2_2_2_333333
// 444___7___7___7___888
// 444_7___7___7___7_888
// 5_5___7___7___7___9_9
// 5_5_7___7___7___7_9_9
// 666___7___7___7___AAA
// 666_7___7___7___7_AAA
// BBBBBB_C_C_C_C_DDDDDD
// BBBBBB_C_C_C_C_DDDDDD

bool Button::paint(const Surface& s) {
  if (style() == TEXT) {
    paintInterior(s, bounds(), *this);
    return true;
  }

  const BorderSpec& spec =
      (style() == CONTAINED ? kContainedBorder : kOutlinedBorder);
  int16_t min_width = std::max(2 * spec.top_width, 2 * spec.bottom_width);
  int16_t full_width = std::max(width(), min_width);
  int16_t min_height = std::max(2 * spec.left_height, 2 * spec.right_height);
  int16_t full_height = std::max(height(), min_height);

  // Top left.
  RasterAlpha4<const uint8_t * PROGMEM> tl(
      Box(0, 0, spec.top_width - 1, spec.top_height - 1), spec.data_top_left,
      Alpha4(outlineColor()));
  s.drawObject(tl);
  // Top bar.
  int16_t top_bar_width = full_width - 2 * spec.top_width;
  if (top_bar_width > 0) {
    printVertStripes(s, spec.top_width, 0, spec.top_width + top_bar_width - 1,
                     spec.top_height, spec.data_top, outlineColor());
  }
  // Top right.
  RasterAlpha4<const uint8_t * PROGMEM> tr(
      Box(spec.top_width + top_bar_width, 0,
          2 * spec.top_width + top_bar_width - 1, spec.top_height - 1),
      spec.data_top_right, Alpha4(outlineColor()));
  s.drawObject(tr);

  // Left top.
  RasterAlpha4<const uint8_t * PROGMEM> lt(
      Box(0, spec.top_height, spec.left_width - 1,
          spec.top_height + spec.left_height - 1),
      spec.data_left_top, Alpha4(outlineColor()));
  s.drawObject(lt);
  // Left bar.
  int16_t left_bar_height = full_height - 2 * spec.left_height;
  if (left_bar_height > 0) {
    printHorizStripes(s, 0, spec.top_height + spec.left_height,
                      full_height - spec.bottom_height - spec.left_height - 1,
                      spec.left_width, spec.data_left, outlineColor());
  }
  // Left bottom.
  RasterAlpha4<const uint8_t * PROGMEM> lb(
      Box(0, full_height - spec.bottom_height - spec.left_height,
          spec.left_width - 1, full_height - spec.bottom_height - 1),
      spec.data_left_bottom, Alpha4(outlineColor()));
  s.drawObject(lb);

  // Main content.
  Box extents(spec.left_width, spec.top_height,
              full_width - spec.right_width - 1,
              full_height - spec.bottom_height - 1);
  paintInterior(s, extents, *this);

  // Right top.
  RasterAlpha4<const uint8_t * PROGMEM> rt(
      Box(full_width - spec.right_width, spec.top_height, full_width - 1,
          spec.top_height + spec.right_height - 1),
      spec.data_right_top, Alpha4(outlineColor()));
  s.drawObject(rt);
  // Right bar.
  int16_t right_bar_height = full_height - 2 * spec.right_height;
  if (right_bar_height > 0) {
    printHorizStripes(s, full_width - spec.right_width,
                      spec.top_height + spec.left_height,
                      full_height - spec.bottom_height - spec.left_height - 1,
                      spec.right_width, spec.data_right, outlineColor());
  }
  // Right bottom.
  RasterAlpha4<const uint8_t * PROGMEM> rb(
      Box(full_width - spec.right_width,
          full_height - spec.bottom_height - spec.left_height, full_width - 1,
          full_height - spec.bottom_height - 1),
      spec.data_right_bottom, Alpha4(outlineColor()));
  s.drawObject(rb);

  // Bottom left.
  RasterAlpha4<const uint8_t * PROGMEM> bl(
      Box(0, full_height - spec.bottom_height, spec.top_width - 1,
          full_height - 1),
      spec.data_bottom_left, Alpha4(outlineColor()));
  s.drawObject(bl);
  // Bottom bar.
  int16_t bottom_bar_width = full_width - 2 * spec.bottom_width;
  if (bottom_bar_width > 0) {
    printVertStripes(s, spec.bottom_width, full_height - spec.bottom_height,
                     spec.bottom_width + top_bar_width - 1, spec.bottom_height,
                     spec.data_bottom, outlineColor());
  }
  // Bottom right.
  RasterAlpha4<const uint8_t * PROGMEM> br(
      Box(spec.bottom_width + bottom_bar_width,
          full_height - spec.bottom_height,
          2 * spec.bottom_width + bottom_bar_width - 1, full_height - 1),
      spec.data_bottom_right, Alpha4(outlineColor()));
  s.drawObject(br);
  return true;
}

Dimensions Button::getSuggestedMinimumDimensions() const {
  const BorderSpec& spec =
      (style() == CONTAINED ? kContainedBorder : kOutlinedBorder);
  int16_t border_width = std::max(2 * spec.top_width, 2 * spec.bottom_width);
  int16_t border_height = std::max(2 * spec.left_height, 2 * spec.right_height);

  if (!hasLabel()) {
    if (!hasIcon()) return Dimensions(0, 0);
    // Icon only.
    return Dimensions(border_width + icon().extents().width(),
                      border_height + icon().extents().height());
  }
  // We must measure the text.
  auto metrics = theme().font.button->getHorizontalStringMetrics(
      (const uint8_t*)label().c_str(), label().size());
  if (!hasIcon()) {
    // Label only.
    return Dimensions(border_width + metrics.width(),
                      border_height + metrics.height());
  }
  // Both icon and label.
  int16_t gap = (icon().extents().width() / 2) & 0xFFFC;
  return Dimensions(
      border_width + icon().extents().width() + gap + metrics.width(),
      border_height + std::max(icon().extents().height(), (int16_t)metrics.height()));
}

}  // namespace roo_windows
