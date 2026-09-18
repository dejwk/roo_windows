#include "roo_windows/keyboard/keyboard.h"

#include <algorithm>
#include <memory>
#include <new>

#include "roo_display/shape/smooth.h"
#include "roo_display/ui/text_label.h"
#include "roo_display/ui/tile.h"
#include "roo_icons/outlined/action.h"
#include "roo_icons/outlined/content.h"
#include "roo_io/text/unicode.h"
#include "roo_scheduler.h"
#include "roo_windows/config.h"
#include "roo_windows/core/dimensions.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/surface_widget.h"
#include "roo_windows/core/task.h"
#include "roo_windows/decoration/decoration.h"

namespace roo_windows {

using namespace roo_display;

// Image file shift_24 24x24, 4-bit Alpha,  RLE, 121 bytes.
static const uint8_t shift_24_data[] PROGMEM = {
    0x80, 0xE7, 0x00, 0x30, 0x47, 0x77, 0x82, 0x3E, 0xE5, 0x77, 0x50,
    0x40, 0xEB, 0x05, 0x77, 0x38, 0x34, 0xEF, 0x76, 0xA0, 0x57, 0x71,
    0x82, 0x4E, 0xF7, 0x20, 0x6A, 0x05, 0x76, 0x82, 0x4E, 0xF7, 0x40,
    0x6A, 0x05, 0x74, 0x82, 0x5E, 0xF6, 0x60, 0x5A, 0x06, 0x72, 0x05,
    0xA0, 0x67, 0x10, 0x5A, 0x06, 0x70, 0x5A, 0x06, 0x73, 0x05, 0xA0,
    0x65, 0x05, 0xA8, 0x2E, 0xAA, 0x46, 0x82, 0x3A, 0xAD, 0xA0, 0x63,
    0x05, 0xE0, 0x66, 0x05, 0xE0, 0x62, 0x80, 0x11, 0x81, 0x8F, 0x66,
    0x86, 0x5F, 0x91, 0x11, 0x12, 0x78, 0x17, 0xF6, 0x68, 0x15, 0xF8,
    0x75, 0x81, 0x7F, 0x66, 0x81, 0x5F, 0x87, 0x58, 0x17, 0xF6, 0x68,
    0x15, 0xF8, 0x75, 0x81, 0x7F, 0xC8, 0x02, 0xA8, 0x1C, 0xF8, 0x75,
    0x07, 0xFB, 0x08, 0x75, 0x01, 0x80, 0x62, 0x01, 0x80, 0x9C, 0x20,
};

const RleImage4bppxBiased<Alpha4, ProgMemPtr>& shift_24() {
  static RleImage4bppxBiased<Alpha4, ProgMemPtr> value(24, 24, shift_24_data,
                                                       Alpha4(color::Black));
  return value;
}

// Image file shift_filled_24 24x24, 4-bit Alpha,  RLE, 86 bytes.
static const uint8_t shift_filled_24_data[] PROGMEM = {
    0x80, 0xE7, 0x00, 0x30, 0x47, 0x77, 0x82, 0x3E, 0xE5, 0x77, 0x50,
    0x40, 0xEB, 0x05, 0x77, 0x30, 0x40, 0xED, 0x05, 0x77, 0x10, 0x40,
    0xEF, 0x05, 0x76, 0x04, 0x0E, 0xFA, 0x05, 0x74, 0x05, 0x0E, 0xFC,
    0x06, 0x72, 0x05, 0xFF, 0x06, 0x70, 0x5F, 0xFA, 0x06, 0x50, 0x5F,
    0xFC, 0x06, 0x30, 0x5F, 0xFE, 0x06, 0x28, 0x01, 0x10, 0x8F, 0xB8,
    0x49, 0x11, 0x11, 0x27, 0x07, 0xFB, 0x08, 0x75, 0x07, 0xFB, 0x08,
    0x75, 0x07, 0xFB, 0x08, 0x75, 0x07, 0xFB, 0x08, 0x75, 0x07, 0xFB,
    0x08, 0x75, 0x01, 0x80, 0x62, 0x01, 0x80, 0x9C, 0x20,
};

const RleImage4bppxBiased<Alpha4, ProgMemPtr>& shift_filled_24() {
  static RleImage4bppxBiased<Alpha4, ProgMemPtr> value(
      24, 24, shift_filled_24_data, Alpha4(color::Black));
  return value;
}

// Image file caps_lock_filled_24 24x24, 4-bit Alpha,  RLE, 96 bytes.
static const uint8_t caps_lock_filled_24_data[] PROGMEM = {
    0x80, 0xE7, 0x00, 0x30, 0x47, 0x77, 0x82, 0x3E, 0xE5, 0x77, 0x50, 0x40,
    0xEB, 0x05, 0x77, 0x30, 0x40, 0xED, 0x05, 0x77, 0x10, 0x40, 0xEF, 0x05,
    0x76, 0x04, 0x0E, 0xFA, 0x05, 0x74, 0x05, 0x0E, 0xFC, 0x06, 0x72, 0x05,
    0xFF, 0x06, 0x70, 0x5F, 0xFA, 0x06, 0x50, 0x5F, 0xFC, 0x06, 0x30, 0x5F,
    0xFE, 0x06, 0x28, 0x01, 0x10, 0x8F, 0xB8, 0x49, 0x11, 0x11, 0x27, 0x07,
    0xFB, 0x08, 0x75, 0x07, 0xFB, 0x08, 0x75, 0x07, 0xFB, 0x08, 0x75, 0x07,
    0xFB, 0x08, 0x75, 0x07, 0xFB, 0x08, 0x75, 0x01, 0x80, 0x62, 0x01, 0x75,
    0x06, 0x80, 0x6C, 0x07, 0x75, 0x07, 0x80, 0x6E, 0x08, 0x80, 0xE2, 0x00,
};

const RleImage4bppxBiased<Alpha4, ProgMemPtr>& caps_lock_filled_24() {
  static RleImage4bppxBiased<Alpha4, ProgMemPtr> value(
      24, 24, caps_lock_filled_24_data, Alpha4(color::Black));
  return value;
}

static const int kPagePaddingPx = 6;
static const int kExtraTopPaddingPx = 2;
static const int kPressPreviewDiameter = Scaled(32);
static const int kPressPreviewGap = Scaled(2);
static const int kPressPreviewElevation = 1;

static const int kButtonMarginPercent = 10;
static const int kMinRowHeight = 10;
static const int kPreferredRowHeight = 10;
static const int kMinCellWidth = 5;
static const int kPreferredCellWidth = 15;
static const roo_time::Duration kDeleteRepeatDelay = roo_time::Millis(400);
static const roo_time::Duration kDeleteRepeatInterval = roo_time::Millis(60);

// All per-key information stays in the borrowed layout tables. Only the active
// key, one repeat timer, and pending paint damage use storage independent of
// the number of keys and pages.
class KeyboardWidget final : public SurfaceWidget {
 public:
  KeyboardWidget(ApplicationContext& context, KeyboardLayout layout,
                 TextInputEmitter& text_input)
      : SurfaceWidget(context),
        layout_(layout),
        text_input_(text_input),
        repeat_(context.scheduler(), [this]() { repeatDelete(); }) {}

  ~KeyboardWidget() override { hidePresentationPin(); }

  Color background() const override { return colorTheme().background; }

  bool isClickable() const override { return true; }
  bool supportsLongPress() override { return true; }
  OverlayType getOverlayType() const override { return OVERLAY_NONE; }
  // Press feedback belongs to one key, not the whole keyboard surface.
  bool useOverlayOnPress() const override { return false; }
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions();
  }
  PreferredSize getPreferredSize() const override;
  Keyboard::CapsState caps_state() const {
    return static_cast<Keyboard::CapsState>(caps_state_);
  }
  void setCapsState(Keyboard::CapsState caps_state);
  void setPage(int idx);
  void setLayout(KeyboardLayout layout) {
    onCancel();
    page_idx_ = -1;
    layout_ = layout;
    caps_state_ = Keyboard::CAPS_STATE_LOW;
    if (isVisible()) setPage(0);
    invalidateInterior();
    requestLayout();
  }
  void onDown(XDim x, YDim y) override;
  void onShowPress(XDim x, YDim y) override;
  void onSingleTapUp(XDim x, YDim y) override;
  void onLongPress(XDim x, YDim y) override;
  void onLongPressFinished(XDim x, YDim y) override;
  void onLongPressMove(XDim x, YDim y) override;
  void onPresentationChanged(const PresentationChange& change) override {
    if (change.state != PresentationState::kPresented ||
        change.detached_since_delivery)
      onCancel();
  }
  void onCancel() override;

  const KeyboardColorTheme& colorTheme() const {
    return context().keyboardColorTheme();
  }
  Rect activeBounds() const { return keyBounds(active_row_, active_key_); }
  uint32_t activeRune() const { return rune(active_row_, active_key_); }

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override {
    if (changed) onCancel();
  }
  void invalidateDescending() override;
  void invalidateDescending(const Rect& rect) override;
  void paintWidgetContents(PaintContext& ctx) override;
  void paint(PaintContext& ctx) const override;

 private:
  // Recomputes the compact grid from final widget dimensions for both drawing
  // and hit testing; no rectangles or other state are stored per key.
  struct Grid {
    int cell_width, row_height, left, top;
  };

  friend class AlternativesPin;
  void showAlternatives();
  void selectAlternative(XDim x, YDim y);
  int alternativeColumns() const { return alternatives_.columns; }
  Rect alternativeGrid() const;
  uint32_t alternativeRune(int choice) const;
  Rect activeAllocation() const;

  struct AlternativeSelection {
    Rect strip;
    int8_t selected = -1;
    uint8_t caps = 0;
    uint8_t columns = 0;
    bool active = false;
  } alternatives_;

  Grid grid() const;
  Rect keyBounds(int row, int key) const;
  Rect keyBounds(const Grid& grid, int x, int row, int key_width) const;
  // Assigns inter-key gutters to their grid cell; staggered row gaps stay
  // empty.
  void findKey(XDim x, YDim y);
  uint32_t rune(int row, int key) const;
  void paintKey(PaintContext& ctx, int row, int key, const Rect& bounds) const;
  void repeatDelete();
  void dirty(const Rect& bounds);
  void includeDamage(const Rect& bounds);

  KeyboardLayout::Page page() const;
  KeyboardLayout::Key keyAt(int row, int key) const;

  KeyboardLayout layout_;
  int16_t page_idx_ = -1;
  TextInputEmitter& text_input_;
  roo_scheduler::SingletonTask repeat_;
  uint8_t caps_state_ = Keyboard::CAPS_STATE_LOW;
  int16_t active_row_ = -1;
  int16_t active_key_ = -1;
  Rect pending_paint_ = Rect(0, 0, -1, -1);
};

namespace {

// With the baseline half an ascent below center, the lower extent is
// ascent/2 - descent. Mirror that extent and add scaled clearance.
int AlternativeRowHeight() {
  const FontMetrics& metrics = font_body1().metrics();
  return metrics.ascent() - 2 * metrics.descent() + Scaled(8);
}

// Draws glyphs without clearing the rounded surface around them.
void PaintKeyContent(PaintContext& ctx, const Drawable& content,
                     const Rect& bounds) {
  ctx.drawTiled(content, bounds, kCenter | kMiddle);
}

}  // namespace

/// Active-only popup-layer preview for the single pressed text key.
class PressHighlighterPin final : public PresentationPin {
 public:
  explicit PressHighlighterPin(const KeyboardWidget& target)
      : target_(target) {}

 protected:
  Rect boundsInWindow() const override {
    XDim dx;
    YDim dy;
    target_.getAbsoluteOffset(dx, dy);
    return CalculateShadowExtents(previewBounds(), kPressPreviewElevation)
        .translate(dx, dy);
  }

  void paint(PaintContext& ctx) const override {
    XDim dx;
    YDim dy;
    target_.getAbsoluteOffset(dx, dy);
    PaintContext local = ctx.translated(dx, dy);
    const Rect preview = previewBounds();
    const Color background = target_.colorTheme().normalButton;
    char utf8[4];
    int size = roo_io::WriteUtf8Char(utf8, target_.activeRune());
    StringViewLabel label(roo::string_view(utf8, size), font_body1(),
                          target_.colorTheme().text);
    const Offset offset =
        (kCenter | kMiddle)
            .resolveOffset(preview.asBox(), label.anchorExtents());
    const Rect glyph_bounds =
        Rect(label.extents()).translate(offset.dx, offset.dy);
    PaintContext label_ctx = local.clipped(glyph_bounds);
    label_ctx.setBgcolor(background);
    label_ctx.drawTiled(label, preview, kCenter | kMiddle);
    local.addExclusion(glyph_bounds);

    PaintDecoration decoration;
    decoration.bounds = preview;
    decoration.background = background;
    const uint8_t radius = preview.width() / 2;
    decoration.corner_radii = {radius, radius, radius, radius};
    decoration.elevation = kPressPreviewElevation;
    local.addDecoration(decoration);
  }

 private:
  Rect previewBounds() const {
    const Rect bounds = target_.activeBounds();
    // Preserve the between-pixel center of even-width keys.
    const XDim x_min =
        (bounds.xMin() + bounds.xMax() - (kPressPreviewDiameter - 1)) / 2;
    const YDim y_max = bounds.yMin() - kPressPreviewGap - 1;
    return Rect(x_min, y_max - kPressPreviewDiameter + 1,
                x_min + kPressPreviewDiameter - 1, y_max);
  }

  const KeyboardWidget& target_;
};

// A single paint-only grid keeps gesture and editor ownership on the keyboard.
class AlternativesPin final : public PresentationPin {
 public:
  explicit AlternativesPin(const KeyboardWidget& target) : target_(target) {}

 protected:
  Rect boundsInWindow() const override {
    return CalculateShadowExtents(target_.alternatives_.strip,
                                  kPressPreviewElevation);
  }

  Rect clipBoundsInWindow() const override {
    return target_.getMainWindow()->bounds();
  }

  void paint(PaintContext& ctx) const override {
    const Rect strip = target_.alternatives_.strip;
    const int count = target_.keyAt(target_.active_row_, target_.active_key_)
                          .alternative_count;
    const int columns = target_.alternativeColumns();
    const int rows = (count + columns - 1) / columns;
    const Rect grid = target_.alternativeGrid();
    const int cell = grid.width() / columns;
    const int height = grid.height() / rows;
    const uint8_t radius = height / 2 + Scaled(4);
    const int inset = BorderStyle(radius, 0).getThickness();
    const Rect inner(strip.xMin() + inset, strip.yMin() + inset,
                     strip.xMax() - inset, strip.yMax() - inset);
    const Color background = target_.colorTheme().normalButton;
    // The circle fits its square cell and the padded popup. Register it once
    // above the glyphs and the shared background decoration.
    const int selected = target_.alternatives_.selected;
    const float highlight_radius = height * 0.5f;
    if (selected >= 0 && highlight_radius > 0) {
      ctx.addOverlayShape(roo_display::SmoothFilledCircle(
          {grid.xMin() + (selected % columns) * cell + (cell - 1) * 0.5f,
           grid.yMin() + (selected / columns) * height + (height - 1) * 0.5f},
          highlight_radius,
          target_.theme().material3Theme().state.resolve(
              material3::ColorToken::kPrimary, InteractionState::kPressed)));
    }
    for (int i = 0; i < count; ++i) {
      const int x = grid.xMin() + (i % columns) * cell;
      const int y = grid.yMin() + (i / columns) * height;
      const Rect box(x, y, x + cell - 1, y + height - 1);
      PaintContext content = ctx.clipped(box).clipped(inner);
      if (content.empty()) continue;
      content.setBgcolor(background);
      char text[4];
      int bytes = roo_io::WriteUtf8Char(text, target_.alternativeRune(i));
      content.drawTiled(
          StringViewLabel(roo::string_view(text, bytes), font_body1(),
                          target_.colorTheme().text),
          box,
          kCenter | kBaseline.toMiddle().shiftBy(
                        font_body1().metrics().ascent() / 2));
      content.addExclusion(inner);
    }
    PaintDecoration decoration;
    decoration.bounds = strip;
    decoration.background = background;
    decoration.corner_radii = {radius, radius, radius, radius};
    decoration.elevation = kPressPreviewElevation;
    ctx.addDecoration(decoration);
  }

 private:
  const KeyboardWidget& target_;
};

Rect KeyboardWidget::activeAllocation() const {
  const Grid g = grid();
  const KeyboardLayout::Key key = keyAt(active_row_, active_key_);
  const int left = g.left + key.start * g.cell_width;
  const int top = g.top + active_row_ * g.row_height;
  XDim dx;
  YDim dy;
  getAbsoluteOffset(dx, dy);
  return Rect(left + dx, top + dy, left + key.width * g.cell_width - 1 + dx,
              top + g.row_height - 1 + dy);
}

uint32_t KeyboardWidget::alternativeRune(int choice) const {
  KeyboardLayout::Character ch;
  layout_.readAlternative(page_idx_, active_row_, active_key_, choice, ch);
  return alternatives_.caps == Keyboard::CAPS_STATE_LOW ? ch.lower : ch.upper;
}

// Only the outer pin has padding; adjacent grid cells share their edges.
Rect KeyboardWidget::alternativeGrid() const {
  const Rect& pin = alternatives_.strip;
  const int padding = Scaled(4);
  return Rect(pin.xMin() + padding, pin.yMin() + padding, pin.xMax() - padding,
              pin.yMax() - padding);
}

void KeyboardWidget::showAlternatives() {
  if (alternatives_.active || !isPresented()) return;
  const KeyboardLayout::Key key = keyAt(active_row_, active_key_);
  if (key.alternative_count == 0) return;
  const Rect viewport = getMainWindow()->bounds();
  const Rect allocation = activeAllocation();
  const int count = key.alternative_count;
  const int padding = Scaled(4);
  const int row_height = AlternativeRowHeight();
  const int cell = row_height;
  const int rows = key.alternative_rows;
  const int columns = (count + rows - 1) / rows;
  const int height = rows * row_height + 2 * padding;
  const int top = allocation.yMin() - height;
  if (row_height <= 0 || row_height / 2 + padding > 255 ||
      top < viewport.yMin() || allocation.yMin() > viewport.yMax() + 1)
    return;
  const int width = cell * columns + 2 * padding;
  if (width > viewport.width()) return;
  const int desired_left =
      (allocation.xMin() + allocation.xMax() + 1 - cell) / 2 -
      (key.default_alternative % columns) * cell - padding;
  const int left =
      std::max<int>(viewport.xMin(),
                    std::min<int>(desired_left, viewport.xMax() + 1 - width));
  std::unique_ptr<PresentationPin> pin(new (std::nothrow)
                                           AlternativesPin(*this));
  if (!pin) return;  // Preserve the ordinary preview and hold behavior.
  alternatives_.strip = Rect(left, top, left + width - 1, top + height - 1);
  alternatives_.caps = caps_state_;
  alternatives_.columns = columns;
  alternatives_.selected = key.default_alternative;
  hidePresentationPin();
  if (showPresentationPin(std::move(pin)) !=
      PresentationPinShowResult::kShown) {
    showPresentationPin(std::unique_ptr<PresentationPin>(
        new (std::nothrow) PressHighlighterPin(*this)));
    return;
  }
  alternatives_.active = true;
  context().presentations().observe(*this);
}

void KeyboardWidget::selectAlternative(XDim x, YDim y) {
  XDim dx;
  YDim dy;
  getAbsoluteOffset(dx, dy);
  const int wx = x + dx, wy = y + dy;
  const Rect strip = alternatives_.strip;
  const Rect base = activeAllocation();
  if (wy < strip.yMin() || wy > base.yMax()) {
    onCancel();
    return;
  }
  const Rect grid = alternativeGrid();
  const int count = keyAt(active_row_, active_key_).alternative_count;
  const int columns = alternativeColumns();
  const int rows = (count + columns - 1) / columns;
  int selected = -1;
  if (wy > strip.yMax()) {
    // Below the popup, anchor horizontal movement to the authored default,
    // even when the visible pin had to shift at a viewport edge.
    const int cell = grid.width() / columns;
    const int default_column =
        keyAt(active_row_, active_key_).default_alternative % columns;
    const int virtual_left =
        (base.xMin() + base.xMax() + 1 - cell) / 2 - default_column * cell;
    if (wx >= virtual_left && wx < virtual_left + columns * cell)
      selected = (rows - 1) * columns + (wx - virtual_left) / cell;
  } else if (wx >= grid.xMin() && wx <= grid.xMax()) {
    const int row = std::min<int>(
        rows - 1, std::max<int>(0, wy - grid.yMin()) / (grid.height() / rows));
    selected = row * columns + (wx - grid.xMin()) / (grid.width() / columns);
  }
  if (selected >= count) selected = -1;
  if (selected == alternatives_.selected) return;
  alternatives_.selected = selected;
  setPresentationPinDirty();
}

void KeyboardWidget::onLongPressMove(XDim x, YDim y) {
  if (!alternatives_.active) return;
  if (!hasPresentationPin() || !isPresented()) {
    onCancel();
    return;
  }
  selectAlternative(x, y);
}

PreferredSize KeyboardWidget::getPreferredSize() const {
  if (page_idx_ < 0) {
    return PreferredSize(PreferredSize::ExactWidth(0),
                         PreferredSize::ExactHeight(0));
  }
  return PreferredSize(
      PreferredSize::MatchParentWidth(),
      PreferredSize::ExactHeight(page().row_count * kPreferredRowHeight +
                                 2 * kPagePaddingPx + kExtraTopPaddingPx));
}

Dimensions KeyboardWidget::onMeasure(WidthSpec width, HeightSpec height) {
  if (page_idx_ < 0) return Dimensions();
  return Dimensions(
      width.kind() == UNSPECIFIED
          ? page().width * kPreferredCellWidth + 2 * kPagePaddingPx
          : width.value(),
      height.kind() == UNSPECIFIED ? page().row_count * kPreferredRowHeight +
                                         2 * kPagePaddingPx + kExtraTopPaddingPx
                                   : height.value());
}

KeyboardWidget::Grid KeyboardWidget::grid() const {
  const int cell_width = std::max<int>(
      kMinCellWidth, (width() - 2 * kPagePaddingPx) / page().width);
  const int row_height = std::max<int>(
      kMinRowHeight,
      (height() - 2 * kPagePaddingPx - kExtraTopPaddingPx) / page().row_count);
  return {cell_width, row_height,
          std::max<int>(0, (width() - page().width * cell_width) / 2),
          std::max<int>(0, (height() - kExtraTopPaddingPx -
                            page().row_count * row_height) /
                               2) +
              kExtraTopPaddingPx};
}

KeyboardLayout::Page KeyboardWidget::page() const {
  KeyboardLayout::Page result;
  layout_.readPage(page_idx_, result);
  return result;
}

KeyboardLayout::Key KeyboardWidget::keyAt(int row_idx, int key_idx) const {
  KeyboardLayout::Key result;
  layout_.readKey(page_idx_, row_idx, key_idx, result);
  return result;
}

Rect KeyboardWidget::keyBounds(int row_idx, int key_idx) const {
  const Grid g = grid();
  const KeyboardLayout::Key key = keyAt(row_idx, key_idx);
  return keyBounds(g, g.left + key.start * g.cell_width, row_idx, key.width);
}

Rect KeyboardWidget::keyBounds(const Grid& g, int x, int row_idx,
                               int key_width) const {
  const int y = g.top + row_idx * g.row_height;
  const int mx = std::max<int>(1, g.cell_width * kButtonMarginPercent / 100);
  const int my = std::max<int>(1, g.row_height * kButtonMarginPercent / 100);
  return Rect(x + mx, y + my, x + key_width * g.cell_width - mx - 1,
              y + g.row_height - my - 1);
}

void KeyboardWidget::findKey(XDim x, YDim y) {
  if (page_idx_ < 0 || !bounds().contains(x, y)) return;
  const Grid g = grid();
  if (y < g.top || x < g.left) return;
  const int row_idx = (y - g.top) / g.row_height;
  const int col = (x - g.left) / g.cell_width;
  if (row_idx >= page().row_count || col >= page().width) return;
  const int key_idx = layout_.findKey(page_idx_, row_idx, col);
  if (key_idx >= 0) {
    active_row_ = row_idx;
    active_key_ = key_idx;
  }
}

uint32_t KeyboardWidget::rune(int row_idx, int key_idx) const {
  const KeyboardLayout::Character ch = keyAt(row_idx, key_idx).character;
  return caps_state_ == Keyboard::CAPS_STATE_LOW ? ch.lower : ch.upper;
}

void KeyboardWidget::paintKey(PaintContext& ctx, int row_idx, int key_idx,
                              const Rect& face) const {
  const KeyboardLayout::Key key = keyAt(row_idx, key_idx);
  Rect bounds = face;
  const bool circle = key.shape == KeyboardLayout::Shape::kCircle;
  if (circle) {
    // Decoration radii are byte-sized; keep the face circular at large scales.
    const int side =
        std::min<int>(510, std::min<int>(face.width(), face.height()));
    if (side <= 0) return;
    const int left = face.xMin() + (face.width() - side) / 2;
    const int top = face.yMin() + (face.height() - side) / 2;
    bounds = Rect(left, top, left + side - 1, top + side - 1);
  }
  PaintContext local = ctx.clipped(bounds);
  if (local.empty()) return;
  Color background = colorTheme().modifierButton;
  if (key.function == KeyboardLayout::Function::kText ||
      key.function == KeyboardLayout::Function::kSpace) {
    background = colorTheme().normalButton;
  } else if (key.function == KeyboardLayout::Function::kEnter) {
    background = colorTheme().acceptButton;
  }
  if (isPressed() && active_row_ == row_idx && active_key_ == key_idx) {
    background = AlphaBlend(background, theme().material3Theme().state.resolve(
                                            material3::ColorToken::kPrimary,
                                            InteractionState::kPressed));
  }
  // Resolve the flat interior first, including transparent glyph pixels.
  // The clipper composes rounded edges over the keyboard surface afterward.
  const uint8_t radius =
      std::min<int>(circle ? 255 : Scaled(3),
                    std::min<int>(bounds.width(), bounds.height()) / 2);
  const int inset = BorderStyle(radius, 0).getThickness();
  const Rect inner(bounds.xMin() + inset, bounds.yMin() + inset,
                   bounds.xMax() - inset, bounds.yMax() - inset);
  PaintContext content = local.clipped(inner);
  content.setBgcolor(background);
  const MonoIcon* icon = nullptr;
  switch (key.function) {
    case KeyboardLayout::Function::kText: {
      char utf8[4];
      int size = roo_io::WriteUtf8Char(utf8, rune(row_idx, key_idx));
      PaintKeyContent(content,
                      StringViewLabel(roo::string_view(utf8, size),
                                      font_body1(), colorTheme().text),
                      bounds);
      break;
    }
    case KeyboardLayout::Function::kSwitchPage: {
      char label[255];
      size_t length = 0;
      layout_.copyLabel(page_idx_, row_idx, key_idx, label, sizeof(label),
                        length);
      PaintKeyContent(content,
                      StringViewLabel(roo::string_view(label, length),
                                      font_button(), colorTheme().text),
                      bounds);
      break;
    }
    case KeyboardLayout::Function::kEnter:
      icon = &ic_outlined_24_action_done();
      break;
    case KeyboardLayout::Function::kDelete:
      icon = &ic_outlined_24_content_backspace();
      break;
    case KeyboardLayout::Function::kShift:
      icon = caps_state_ == Keyboard::CAPS_STATE_LOW ? &shift_24()
             : caps_state_ == Keyboard::CAPS_STATE_HIGH
                 ? &shift_filled_24()
                 : &caps_lock_filled_24();
      break;
    case KeyboardLayout::Function::kSpace:
      content.clear();
      break;
  }
  if (icon != nullptr) {
    MonoIcon tinted = *icon;
    tinted.color_mode().setColor(colorTheme().text);
    PaintKeyContent(content, tinted, bounds);
  }
  PaintDecoration decoration;
  decoration.bounds = bounds;
  decoration.background = background;
  decoration.corner_radii = {radius, radius, radius, radius};
  local.addDecoration(decoration);
  local.addExclusion(inner);
}

void KeyboardWidget::includeDamage(const Rect& bounds) {
  if (bounds.empty()) return;
  pending_paint_ =
      pending_paint_.empty() ? bounds : Rect::Extent(pending_paint_, bounds);
}

void KeyboardWidget::invalidateDescending() {
  Widget::invalidateDescending();
  pending_paint_ = bounds();
}

void KeyboardWidget::invalidateDescending(const Rect& rect) {
  Widget::invalidateDescending(rect);
  // A preview can expose only a strip of this surface while unrelated damage
  // elsewhere expands MainWindow's combined redraw rectangle.
  includeDamage(Rect::Intersect(rect, bounds()));
}

// Preserve pending damage until the entire local pass succeeds, including
// when the framework defers painting because its deadline has expired.
void KeyboardWidget::paintWidgetContents(PaintContext& ctx) {
  const Rect damage = pending_paint_.empty() ? bounds() : pending_paint_;
  PaintContext local = ctx.clipped(damage);
  Widget::paintWidgetContents(local);
  if (!isDirty()) pending_paint_ = Rect(0, 0, -1, -1);
}

void KeyboardWidget::dirty(const Rect& bounds) {
  includeDamage(bounds);
  setDirty(bounds);
}

void KeyboardWidget::paint(PaintContext& ctx) const {
  if (page_idx_ < 0) {
    ctx.clear();
    return;
  }
  const Grid g = grid();
  const Rect clip = ctx.localClip();
  if (clip.empty()) return;
  const int first_row = std::max<int>(0, (clip.yMin() - g.top) / g.row_height);
  const int last_row =
      std::min<int>(page().row_count - 1, (clip.yMax() - g.top) / g.row_height);
  const int left = std::max<int>(g.left, static_cast<int>(clip.xMin()));
  const int past = std::min<int>(g.left + page().width * g.cell_width,
                                 static_cast<int>(clip.xMax()) + 1);
  if (left < past) {
    for (int row_idx = first_row; row_idx <= last_row; ++row_idx) {
      const KeyboardLayout::KeyRange range = layout_.findKeyRange(
          page_idx_, row_idx, (left - g.left) / g.cell_width,
          (past - g.left + g.cell_width - 1) / g.cell_width);
      for (int key = range.first; key < range.past_last; ++key) {
        const Rect face = keyBounds(row_idx, key);
        if (face.intersects(clip)) paintKey(ctx, row_idx, key, face);
      }
    }
  }
  ctx.clear();
}

void KeyboardWidget::setCapsState(Keyboard::CapsState caps_state) {
  if (caps_state == caps_state_) return;
  if (alternatives_.active) onCancel();
  caps_state_ = caps_state;
  dirty(bounds());
  if (hasPresentationPin()) setPresentationPinDirty();
}

void KeyboardWidget::setPage(int idx) {
  const int count = layout_.pageCount();
  if (idx < -1 || idx >= count || idx == page_idx_) return;
  onCancel();
  page_idx_ = idx;
  setCapsState(Keyboard::CAPS_STATE_LOW);
  invalidateInterior();
  requestLayout();
}

void KeyboardWidget::onDown(XDim x, YDim y) {
  onCancel();
  findKey(x, y);
}

void KeyboardWidget::onShowPress(XDim x, YDim y) {
  if (active_key_ < 0 || isPressed()) return;
  setPressed(true);
  dirty(activeBounds());
  const KeyboardLayout::Key key = keyAt(active_row_, active_key_);
  switch (key.function) {
    case KeyboardLayout::Function::kText:
      showPresentationPin(std::unique_ptr<PresentationPin>(
          new (std::nothrow) PressHighlighterPin(*this)));
      break;
    case KeyboardLayout::Function::kShift:
      setCapsState(caps_state_ == Keyboard::CAPS_STATE_LOW
                       ? Keyboard::CAPS_STATE_HIGH
                       : Keyboard::CAPS_STATE_LOW);
      break;
    case KeyboardLayout::Function::kDelete:
      // Schedule before synchronous delivery, which may hide this keyboard.
      repeat_.scheduleAfter(kDeleteRepeatDelay);
      text_input_.deleteBackward();
      break;
    case KeyboardLayout::Function::kSwitchPage:
      setPage(key.target_page);
      break;
    default:
      break;
  }
}

void KeyboardWidget::onSingleTapUp(XDim x, YDim y) {
  if (active_key_ < 0) return;
  if (!isPressed()) onShowPress(x, y);
  if (active_key_ < 0) return;  // Page switches cancel the old key.
  const KeyboardLayout::Function function =
      keyAt(active_row_, active_key_).function;
  const uint32_t ch =
      function == KeyboardLayout::Function::kText ? activeRune() : ' ';
  const Keyboard::CapsState caps = caps_state();
  onCancel();
  if (function == KeyboardLayout::Function::kText) {
    if (caps == Keyboard::CAPS_STATE_HIGH)
      setCapsState(Keyboard::CAPS_STATE_LOW);
    text_input_.commitRune(ch);
  } else if (function == KeyboardLayout::Function::kSpace) {
    text_input_.commitRune(' ');
  } else if (function == KeyboardLayout::Function::kEnter) {
    text_input_.performAction(TextInputAction::kDone);
  }
}

void KeyboardWidget::onLongPress(XDim x, YDim y) {
  if (active_key_ < 0) return;
  if (!isPressed()) onShowPress(x, y);
  if (active_key_ < 0) return;
  if (keyAt(active_row_, active_key_).function ==
      KeyboardLayout::Function::kText) {
    showAlternatives();
    return;
  }
  if (keyAt(active_row_, active_key_).function ==
          KeyboardLayout::Function::kShift &&
      caps_state_ == Keyboard::CAPS_STATE_HIGH) {
    setCapsState(Keyboard::CAPS_STATE_HIGH_LOCKED);
  }
}

void KeyboardWidget::onLongPressFinished(XDim x, YDim y) {
  if (!alternatives_.active) {
    onSingleTapUp(x, y);
    return;
  }
  if (!hasPresentationPin() || !isPresented()) {
    onCancel();
    return;
  }
  selectAlternative(x, y);
  const int selected = alternatives_.selected;
  const uint32_t rune = selected >= 0 ? alternativeRune(selected) : 0;
  const uint8_t caps = alternatives_.caps;
  onCancel();
  if (selected < 0) return;
  if (caps == Keyboard::CAPS_STATE_HIGH) setCapsState(Keyboard::CAPS_STATE_LOW);
  text_input_.commitRune(rune);
}

void KeyboardWidget::onCancel() {
  if (alternatives_.active) context().presentations().unobserve(*this);
  alternatives_.active = false;
  alternatives_.selected = -1;
  repeat_.cancel();
  hidePresentationPin();
  if (active_key_ >= 0) dirty(activeBounds());
  active_row_ = active_key_ = -1;
  setPressed(false);
}

void KeyboardWidget::repeatDelete() {
  if (!isPressed() || active_key_ < 0 || !isPresented()) return;
  repeat_.scheduleAfter(kDeleteRepeatInterval);
  text_input_.deleteBackward();
}

KeyboardWidget* Keyboard::contents() {
  return (KeyboardWidget*)contents_.get();
}

const KeyboardWidget* Keyboard::contents() const {
  return (KeyboardWidget*)contents_.get();
}

Keyboard::Keyboard(ApplicationContext& context, KeyboardLayout layout)
    : contents_(new KeyboardWidget(context, layout, text_input_)) {
  contents_->setVisibility(Visibility::kGone);
}

Widget& Keyboard::getContents() { return *contents_; }

const Widget& Keyboard::getContents() const { return *contents_; }

void Keyboard::show() {
  task_->setVisible(true);
  contents()->setPage(0);
  contents()->setVisibility(Visibility::kVisible);
}

void Keyboard::hide() {
  contents()->setVisibility(Visibility::kGone);
  contents()->setPage(-1);
  contents()->setCapsState(CapsState::CAPS_STATE_LOW);
  task_->setVisible(false);
}

void Keyboard::connect(Application& destination) {
  contents()->onCancel();
  text_input_.disconnect();
  text_input_.connect(destination);
}

void Keyboard::setLayout(KeyboardLayout layout) {
  contents()->setLayout(layout);
}

void Keyboard::setPage(int idx) { contents()->setPage(idx); }

Keyboard::CapsState Keyboard::caps_state() const {
  return contents()->caps_state();
}

void Keyboard::setCapsState(CapsState caps_state) {
  contents()->setCapsState(caps_state);
}

}  // namespace roo_windows
