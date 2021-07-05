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

// static const PROGMEM uint8_t kOutlinedTopLeftData[] = {0x02, 0x9E, 0x3E, 0x93};
// static const PROGMEM uint8_t kOutlinedTopData[] = {0xF0};
// static const PROGMEM uint8_t kOutlinedTopRightData[] = {0xE9, 0x20, 0x39, 0xE2};
// static const PROGMEM uint8_t kOutlinedBottomLeftData[] = {0x2E, 0x93, 0x02,
//                                                           0x9E};
// static const PROGMEM uint8_t kOutlinedBottomData[] = {0x0F};
// static const PROGMEM uint8_t kOutlinedBottomRightData[] = {0x39, 0xE2, 0xE9,
//                                                            0x20};
// static const PROGMEM uint8_t kOutlinedLeftTopData[] = {0x99, 0xE3};
// static const PROGMEM uint8_t kOutlinedLeftData[] = {0xF0};
// static const PROGMEM uint8_t kOutlinedLeftBottomData[] = {0xE3, 0x99};
// static const PROGMEM uint8_t kOutlinedRightTopData[] = {0x99, 0x3E};
// static const PROGMEM uint8_t kOutlinedRightData[] = {0x0F};
// static const PROGMEM uint8_t kOutlinedRightBottomData[] = {0x3E, 0x99};

static const PROGMEM uint8_t kOutlinedTopLeftData[] = {0x01, 0x57, 0x27, 0x52};
static const PROGMEM uint8_t kOutlinedTopData[] = {0x71};
static const PROGMEM uint8_t kOutlinedTopRightData[] = {0x75, 0x10, 0x25, 0x71};
static const PROGMEM uint8_t kOutlinedBottomLeftData[] = {0x17, 0x52, 0x01,
                                                          0x57};
static const PROGMEM uint8_t kOutlinedBottomData[] = {0x17};
static const PROGMEM uint8_t kOutlinedBottomRightData[] = {0x2A, 0x71, 0x75,
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

void paintInterior(const Surface& s, const Box& bounds, const Button& button) {
  s.drawObject(MakeTileOf(TextLabel(*button.theme().font.button, button.label(),
                                    button.textColor(), s.fill_mode()),
                          bounds, HAlign::Center(), VAlign::Middle(),
                          button.interiorColor(), s.fill_mode()));
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

Button::Button(Panel* parent, const Box& bounds, std::string label, Style style)
    : Widget(parent, bounds),
      style_(style),
      label_(std::move(label)),
      outlineColor_(style == CONTAINED  ? theme().color.primary
                    : style == OUTLINED ? theme().color.onBackground
                                        : color::Transparent),
      interiorColor_(style == CONTAINED ? theme().color.primary
                                        : theme().color.background),
      textColor_(style == CONTAINED ? theme().color.onPrimary
                                    : theme().color.primary) {}


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

void Button::defaultPaint(const Surface& s) {
  if (style() == TEXT) {
    paintInterior(s, bounds(), *this);
    return;
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
          spec.left_width - 1, full_height - 1),
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
    printHorizStripes(s, full_width - spec.right_width, spec.top_height + spec.left_height,
          full_height - spec.bottom_height - spec.left_height - 1,
          spec.right_width, spec.data_right, outlineColor());
  }
  // Right bottom.
  RasterAlpha4<const uint8_t * PROGMEM> rb(
      Box(full_width - spec.right_width,
          full_height - spec.bottom_height - spec.left_height, full_width - 1,
          full_height - 1),
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
}

}  // namespace roo_windows
