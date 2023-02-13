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

void printVertStripes(const Canvas& canvas, int16_t xMin, int16_t yMin,
                      int16_t xMax, uint8_t count,
                      const uint8_t* PROGMEM alpha4, Color base_color) {
  Alpha4 color_mode(base_color);
  typename Raster<const uint8_t * PROGMEM, Alpha4>::StreamType stream(
      alpha4, color_mode);
  for (int i = 0; i < count; ++i) {
    Color c = stream.next();
    canvas.drawHLine(xMin, yMin + i, xMax, c);
  }
}

void printHorizStripes(const Canvas& canvas, int16_t xMin, int16_t yMin,
                       int16_t yMax, uint8_t count,
                       const uint8_t* PROGMEM alpha4, Color base_color) {
  Alpha4 color_mode(base_color);
  typename Raster<const uint8_t * PROGMEM, Alpha4>::StreamType stream(
      alpha4, color_mode);
  for (int i = 0; i < count; ++i) {
    Color c = stream.next();
    canvas.drawVLine(xMin + i, yMin, yMax, c);
  }
}

}  // namespace

ButtonBase::ButtonBase(const Environment& env, Style style)
    : BasicWidget(env),
      style_(style),
      outline_color_(style == CONTAINED  ? env.theme().color.primary
                     : style == OUTLINED ? env.theme().color.onBackground
                                         : color::Transparent),
      interior_color_(style == CONTAINED ? env.theme().color.primary
                                         : env.theme().color.background),
      content_color_(style == CONTAINED ? env.theme().color.onPrimary
                                        : env.theme().color.primary),
      elevation_resting_(0),
      elevation_pressed_(0) {}

Button::Button(const Environment& env, const MonoIcon* icon, std::string label,
               Style style)
    : ButtonBase(env, style),
      font_(env.theme().font.button),
      label_(std::move(label)),
      icon_(icon) {}

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

void ButtonBase::setElevation(uint8_t resting, uint8_t pressed) {
  if (elevation_resting_ != resting) {
    uint8_t larger = std::max(elevation_resting_, resting);
    elevation_resting_ = resting;
    if (!isPressed()) {
      elevationChanged(larger);
    }
  }
  if (elevation_pressed_ != pressed) {
    uint8_t larger = std::max(elevation_pressed_, pressed);
    elevation_pressed_ = pressed;
    if (isPressed()) {
      elevationChanged(larger);
    }
  }
}

void ButtonBase::paint(const Canvas& canvas) const {
  Canvas interior_canvas = canvas;
  if (style() == TEXT) {
    paintInterior(interior_canvas, bounds());
    return;
  }

  const BorderSpec& spec =
      (style() == CONTAINED ? kContainedBorder : kOutlinedBorder);
  Color outline = (style() == CONTAINED ? background() : getOutlineColor());
  int16_t min_width = std::max(2 * spec.top_width, 2 * spec.bottom_width);
  int16_t full_width = std::max(width(), min_width);
  int16_t min_height = std::max(2 * spec.left_height, 2 * spec.right_height);
  int16_t full_height = std::max<YDim>(height(), min_height);

  int16_t top_bar_width = full_width - 2 * spec.top_width;

  if (isInvalidated()) {
    // Top left.
    RasterAlpha4<const uint8_t * PROGMEM> tl(
        Box(0, 0, spec.top_width - 1, spec.top_height - 1), spec.data_top_left,
        Alpha4(getOutlineColor()));
    canvas.drawObject(tl);
    // Top bar.
    if (top_bar_width > 0) {
      printVertStripes(canvas, spec.top_width, 0,
                       spec.top_width + top_bar_width - 1, spec.top_height,
                       spec.data_top, outline);
    }
    // Top right.
    RasterAlpha4<const uint8_t * PROGMEM> tr(
        Box(spec.top_width + top_bar_width, 0,
            2 * spec.top_width + top_bar_width - 1, spec.top_height - 1),
        spec.data_top_right, Alpha4(outline));
    canvas.drawObject(tr);

    // Left top.
    RasterAlpha4<const uint8_t * PROGMEM> lt(
        Box(0, spec.top_height, spec.left_width - 1,
            spec.top_height + spec.left_height - 1),
        spec.data_left_top, Alpha4(outline));
    canvas.drawObject(lt);
    // Left bar.
    int16_t left_bar_height = full_height - 2 * spec.left_height;
    if (left_bar_height > 0) {
      printHorizStripes(canvas, 0, spec.top_height + spec.left_height,
                        full_height - spec.bottom_height - spec.left_height - 1,
                        spec.left_width, spec.data_left, outline);
    }
    // Left bottom.
    RasterAlpha4<const uint8_t * PROGMEM> lb(
        Box(0, full_height - spec.bottom_height - spec.left_height,
            spec.left_width - 1, full_height - spec.bottom_height - 1),
        spec.data_left_bottom, Alpha4(outline));
    canvas.drawObject(lb);
  }

  // Main content.
  Rect extents(spec.left_width, spec.top_height,
               full_width - spec.right_width - 1,
               full_height - spec.bottom_height - 1);
  interior_canvas.clipToExtents(extents);
  paintInterior(interior_canvas, extents);

  if (isInvalidated()) {
    // Right top.
    RasterAlpha4<const uint8_t * PROGMEM> rt(
        Box(full_width - spec.right_width, spec.top_height, full_width - 1,
            spec.top_height + spec.right_height - 1),
        spec.data_right_top, Alpha4(outline));
    canvas.drawObject(rt);
    // Right bar.
    int16_t right_bar_height = full_height - 2 * spec.right_height;
    if (right_bar_height > 0) {
      printHorizStripes(canvas, full_width - spec.right_width,
                        spec.top_height + spec.left_height,
                        full_height - spec.bottom_height - spec.left_height - 1,
                        spec.right_width, spec.data_right, outline);
    }
    // Right bottom.
    RasterAlpha4<const uint8_t * PROGMEM> rb(
        Box(full_width - spec.right_width,
            full_height - spec.bottom_height - spec.left_height, full_width - 1,
            full_height - spec.bottom_height - 1),
        spec.data_right_bottom, Alpha4(outline));
    canvas.drawObject(rb);

    // Bottom left.
    RasterAlpha4<const uint8_t * PROGMEM> bl(
        Box(0, full_height - spec.bottom_height, spec.top_width - 1,
            full_height - 1),
        spec.data_bottom_left, Alpha4(outline));
    canvas.drawObject(bl);
    // Bottom bar.
    int16_t bottom_bar_width = full_width - 2 * spec.bottom_width;
    if (bottom_bar_width > 0) {
      printVertStripes(canvas, spec.bottom_width,
                       full_height - spec.bottom_height,
                       spec.bottom_width + top_bar_width - 1,
                       spec.bottom_height, spec.data_bottom, outline);
    }
    // Bottom right.
    RasterAlpha4<const uint8_t * PROGMEM> br(
        Box(spec.bottom_width + bottom_bar_width,
            full_height - spec.bottom_height,
            2 * spec.bottom_width + bottom_bar_width - 1, full_height - 1),
        spec.data_bottom_right, Alpha4(outline));
    canvas.drawObject(br);
  }
}

Padding ButtonBase::getDefaultPadding() const { return Padding(14, 4); }

Dimensions Button::getSuggestedMinimumDimensions() const {
  if (!hasLabel()) {
    if (!hasIcon()) return Dimensions(0, 0);
    // Icon only.
    return Dimensions(4 + icon().extents().width(),
                      4 + icon().extents().height());
  }
  // We must measure the text.
  auto metrics = font_->getHorizontalStringMetrics(label_);
  int16_t text_height = ((font_->metrics().maxHeight()) + 1) & ~1;
  if (!hasIcon()) {
    // Label only.
    return Dimensions(4 + metrics.width(), 4 + text_height);
  }
  // Both icon and label.
  int16_t gap = (icon().extents().width() / 2) & 0xFFFC;
  return Dimensions(4 + icon().extents().width() + gap + metrics.width(),
                    4 + std::max(icon().extents().height(), text_height));
}

void Button::paintInterior(const Canvas& canvas, Rect bounds) const {
  if (!hasIcon() && label().empty()) {
    canvas.clearRect(bounds);
    return;
  }
  if (label().empty()) {
    // Icon only.
    MonoIcon ic = icon();
    ic.color_mode().setColor(contentColor());
    canvas.drawTiled(ic, bounds, kCenter | kMiddle);
    return;
  }
  if (!hasIcon()) {
    canvas.drawTiled(StringViewLabel(label_, *font_, contentColor()), bounds,
                     kCenter | kMiddle);
    return;
  }
  // Both text and icon.
  MonoIcon ic = icon();
  StringViewLabel l(label_, *font_, contentColor());
  ic.color_mode().setColor(contentColor());
  int16_t gap = (ic.extents().width() / 2) & 0xFFFC;
  int16_t x_offset =
      (bounds.width() - ic.extents().width() - l.extents().width() - gap) / 2;
  int16_t x_cursor = bounds.xMin();
  const int16_t yMin = bounds.yMin();
  const int16_t yMax = bounds.yMax();
  // Left border.
  if (x_offset > 0) {
    canvas.clearRect(x_cursor, yMin, x_cursor + x_offset - 1, yMax);
  }
  x_cursor += x_offset;  // Note: x_offset may be negative.
  // Icon.
  {
    Rect r(x_cursor, yMin, x_cursor + ic.extents().width() - 1, yMax);
    canvas.drawTiled(ic, r, kCenter | kMiddle);
  }
  x_cursor += ic.extents().width();
  // Gap.
  canvas.clearRect(x_cursor, yMin, x_cursor + gap - 1, yMax);
  x_cursor += gap;
  // Text.
  {
    Rect r(x_cursor, yMin, x_cursor + l.extents().width() - 1, yMax);
    canvas.drawTiled(l, r, kCenter | kMiddle);
  }
  x_cursor += l.extents().width();
  // Right border.
  if (x_offset <= bounds.xMax()) {
    canvas.clearRect(x_cursor, yMin, bounds.xMax(), yMax);
  }
}

}  // namespace roo_windows
