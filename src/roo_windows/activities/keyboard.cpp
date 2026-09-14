#include "roo_windows/activities/keyboard.h"

#include <algorithm>
#include <memory>
#include <new>

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
  KeyboardWidget(ApplicationContext& context, const KeyboardSpec* spec,
                 TextInputEmitter& text_input)
      : SurfaceWidget(context),
        spec_(spec),
        text_input_(text_input),
        repeat_(context.scheduler(), [this]() { repeatDelete(); }) {}

  KeyboardWidget(ApplicationContext& context, KeyboardLayoutView layout,
                 TextInputEmitter& text_input)
      : SurfaceWidget(context),
        layout_(layout),
        text_input_(text_input),
        repeat_(context.scheduler(), [this]() { repeatDelete(); }) {}

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
  void onDown(XDim x, YDim y) override;
  void onShowPress(XDim x, YDim y) override;
  void onSingleTapUp(XDim x, YDim y) override;
  void onLongPress(XDim x, YDim y) override;
  void onLongPressFinished(XDim x, YDim y) override;
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

  KeyboardLayoutView::Page page() const;
  KeyboardLayoutView::Key keyAt(int row, int key) const;
  int rowKeyCount(int row) const;

  const KeyboardSpec* spec_ = nullptr;
  KeyboardLayoutView layout_;
  int16_t page_idx_ = -1;
  TextInputEmitter& text_input_;
  roo_scheduler::SingletonTask repeat_;
  uint8_t caps_state_ = Keyboard::CAPS_STATE_LOW;
  int16_t active_row_ = -1;
  int16_t active_key_ = -1;
  Rect pending_paint_ = Rect(0, 0, -1, -1);
};

namespace {

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
  const int cell_width =
      std::max(kMinCellWidth, (width() - 2 * kPagePaddingPx) / page().width);
  const int row_height = std::max(
      kMinRowHeight,
      (height() - 2 * kPagePaddingPx - kExtraTopPaddingPx) / page().row_count);
  return {cell_width, row_height,
          std::max(0, (width() - page().width * cell_width) / 2),
          std::max(0, (height() - kExtraTopPaddingPx -
                       page().row_count * row_height) /
                          2) +
              kExtraTopPaddingPx};
}

KeyboardLayoutView::Page KeyboardWidget::page() const {
  KeyboardLayoutView::Page result;
  if (page_idx_ < 0) return result;
  if (spec_ != nullptr) {
    result.width = spec_->pages[page_idx_].row_width;
    result.row_count = spec_->pages[page_idx_].row_count;
  } else {
    layout_.readPage(page_idx_, result);
  }
  return result;
}

int KeyboardWidget::rowKeyCount(int row) const {
  if (spec_ != nullptr) return spec_->pages[page_idx_].rows[row].key_count;
  KeyboardLayoutView::Row result;
  layout_.readRow(page_idx_, row, result);
  return result.key_count;
}

// Decode legacy layouts only during migration; generated records need no scan.
KeyboardLayoutView::Key KeyboardWidget::keyAt(int row_idx, int key_idx) const {
  KeyboardLayoutView::Key result;
  if (page_idx_ < 0 || row_idx < 0 || key_idx < 0) return result;
  if (spec_ == nullptr) {
    layout_.readKey(page_idx_, row_idx, key_idx, result);
    return result;
  }
  const KeyboardRowSpec& row = spec_->pages[page_idx_].rows[row_idx];
  const KeySpec& key = row.keys[key_idx];
  result.start = row.start_offset;
  for (int i = 0; i < key_idx; ++i) result.start += row.keys[i].width;
  result.width = key.width;
  result.function = static_cast<KeyboardLayoutView::Function>(key.function);
  result.character = {key.data, row.keys_caps[key_idx].data};
  result.target_page = key.data & 255;
  result.label_bytes = key.data >> 16;
  return result;
}

Rect KeyboardWidget::keyBounds(int row_idx, int key_idx) const {
  const Grid g = grid();
  const KeyboardLayoutView::Key key = keyAt(row_idx, key_idx);
  return keyBounds(g, g.left + key.start * g.cell_width, row_idx, key.width);
}

Rect KeyboardWidget::keyBounds(const Grid& g, int x, int row_idx,
                               int key_width) const {
  const int y = g.top + row_idx * g.row_height;
  const int mx = std::max(1, g.cell_width * kButtonMarginPercent / 100);
  const int my = std::max(1, g.row_height * kButtonMarginPercent / 100);
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
  int key_idx = -1;
  if (spec_ == nullptr) {
    key_idx = layout_.findKey(page_idx_, row_idx, col);
  } else {
    const KeyboardRowSpec& row = spec_->pages[page_idx_].rows[row_idx];
    int left = row.start_offset;
    for (int i = 0; i < row.key_count; ++i) {
      if (col >= left && col < left + row.keys[i].width) {
        key_idx = i;
        break;
      }
      left += row.keys[i].width;
    }
  }
  if (key_idx >= 0) {
    active_row_ = row_idx;
    active_key_ = key_idx;
  }
}

uint32_t KeyboardWidget::rune(int row_idx, int key_idx) const {
  const KeyboardLayoutView::Character ch = keyAt(row_idx, key_idx).character;
  return caps_state_ == Keyboard::CAPS_STATE_LOW ? ch.lower : ch.upper;
}

void KeyboardWidget::paintKey(PaintContext& ctx, int row_idx, int key_idx,
                              const Rect& bounds) const {
  const KeyboardLayoutView::Key key = keyAt(row_idx, key_idx);
  PaintContext local = ctx.clipped(bounds);
  if (local.empty()) return;
  Color background = colorTheme().modifierButton;
  if (key.function == KeyboardLayoutView::Function::kText ||
      key.function == KeyboardLayoutView::Function::kSpace) {
    background = colorTheme().normalButton;
  } else if (key.function == KeyboardLayoutView::Function::kEnter) {
    background = colorTheme().acceptButton;
  }
  if (isPressed() && active_row_ == row_idx && active_key_ == key_idx) {
    background = AlphaBlend(background, theme().material3Theme().state.resolve(
                                            material3::ColorToken::kPrimary,
                                            InteractionState::kPressed));
  }
  // Resolve the flat interior first, including transparent glyph pixels.
  // The clipper composes rounded edges over the keyboard surface afterward.
  const uint8_t radius = std::min<int>(
      Scaled(3), std::min<int>(bounds.width(), bounds.height()) / 2);
  const int inset = BorderStyle(radius, 0).getThickness();
  const Rect inner(bounds.xMin() + inset, bounds.yMin() + inset,
                   bounds.xMax() - inset, bounds.yMax() - inset);
  PaintContext content = local.clipped(inner);
  content.setBgcolor(background);
  const MonoIcon* icon = nullptr;
  switch (key.function) {
    case KeyboardLayoutView::Function::kText: {
      char utf8[4];
      int size = roo_io::WriteUtf8Char(utf8, rune(row_idx, key_idx));
      PaintKeyContent(content,
                      StringViewLabel(roo::string_view(utf8, size),
                                      font_body1(), colorTheme().text),
                      bounds);
      break;
    }
    case KeyboardLayoutView::Function::kSwitchPage: {
      char label[255];
      size_t length = 0;
      if (spec_ != nullptr) {
        const KeyboardRowSpec& row = spec_->pages[page_idx_].rows[row_idx];
        const uint32_t data = row.keys[key_idx].data;
        length = data >> 16;
        std::copy_n(row.pageswitch_key_labels + ((data >> 8) & 255), length,
                    label);
      } else {
        layout_.copyLabel(page_idx_, row_idx, key_idx, label, sizeof(label),
                          length);
      }
      PaintKeyContent(content,
                      StringViewLabel(roo::string_view(label, length),
                                      font_button(), colorTheme().text),
                      bounds);
      break;
    }
    case KeyboardLayoutView::Function::kEnter:
      icon = &ic_outlined_24_action_done();
      break;
    case KeyboardLayoutView::Function::kDelete:
      icon = &ic_outlined_24_content_backspace();
      break;
    case KeyboardLayoutView::Function::kShift:
      icon = caps_state_ == Keyboard::CAPS_STATE_LOW ? &shift_24()
             : caps_state_ == Keyboard::CAPS_STATE_HIGH
                 ? &shift_filled_24()
                 : &caps_lock_filled_24();
      break;
    case KeyboardLayoutView::Function::kSpace:
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
  const int first_row = std::max(0, (clip.yMin() - g.top) / g.row_height);
  const int last_row =
      std::min<int>(page().row_count - 1, (clip.yMax() - g.top) / g.row_height);
  const int left = std::max(g.left, static_cast<int>(clip.xMin()));
  const int past = std::min(g.left + page().width * g.cell_width,
                            static_cast<int>(clip.xMax()) + 1);
  if (left < past) {
    for (int row_idx = first_row; row_idx <= last_row; ++row_idx) {
      KeyboardLayoutView::KeyRange range{
          0, static_cast<uint16_t>(rowKeyCount(row_idx))};
      if (spec_ == nullptr) {
        range = layout_.findKeyRange(
            page_idx_, row_idx, (left - g.left) / g.cell_width,
            (past - g.left + g.cell_width - 1) / g.cell_width);
      }
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
  caps_state_ = caps_state;
  dirty(bounds());
  if (hasPresentationPin()) setPresentationPinDirty();
}

void KeyboardWidget::setPage(int idx) {
  const int count = spec_ != nullptr ? spec_->page_count : layout_.pageCount();
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
  const KeyboardLayoutView::Key key = keyAt(active_row_, active_key_);
  switch (key.function) {
    case KeyboardLayoutView::Function::kText:
      showPresentationPin(std::unique_ptr<PresentationPin>(
          new (std::nothrow) PressHighlighterPin(*this)));
      break;
    case KeyboardLayoutView::Function::kShift:
      setCapsState(caps_state_ == Keyboard::CAPS_STATE_LOW
                       ? Keyboard::CAPS_STATE_HIGH
                       : Keyboard::CAPS_STATE_LOW);
      break;
    case KeyboardLayoutView::Function::kDelete:
      // Schedule before synchronous delivery, which may hide this keyboard.
      repeat_.scheduleAfter(kDeleteRepeatDelay);
      text_input_.deleteBackward();
      break;
    case KeyboardLayoutView::Function::kSwitchPage:
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
  const KeyboardLayoutView::Function function =
      keyAt(active_row_, active_key_).function;
  const uint32_t ch =
      function == KeyboardLayoutView::Function::kText ? activeRune() : ' ';
  const Keyboard::CapsState caps = caps_state();
  onCancel();
  if (function == KeyboardLayoutView::Function::kText) {
    if (caps == Keyboard::CAPS_STATE_HIGH)
      setCapsState(Keyboard::CAPS_STATE_LOW);
    text_input_.commitRune(ch);
  } else if (function == KeyboardLayoutView::Function::kSpace) {
    text_input_.commitRune(' ');
  } else if (function == KeyboardLayoutView::Function::kEnter) {
    text_input_.performAction(TextInputAction::kDone);
  }
}

void KeyboardWidget::onLongPress(XDim x, YDim y) {
  if (active_key_ < 0) return;
  if (keyAt(active_row_, active_key_).function ==
          KeyboardLayoutView::Function::kShift &&
      caps_state_ == Keyboard::CAPS_STATE_HIGH) {
    setCapsState(Keyboard::CAPS_STATE_HIGH_LOCKED);
  }
}

void KeyboardWidget::onLongPressFinished(XDim x, YDim y) {
  // Text keys still commit on release after a hold; only shift and delete
  // have additional long-press semantics.
  onSingleTapUp(x, y);
}

void KeyboardWidget::onCancel() {
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

Keyboard::Keyboard(ApplicationContext& context, const KeyboardSpec* spec)
    : contents_(new KeyboardWidget(context, spec, text_input_)) {
  contents_->setVisibility(Visibility::kGone);
}

Keyboard::Keyboard(ApplicationContext& context, KeyboardLayoutView layout)
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
  text_input_.disconnect();
  text_input_.connect(destination);
}

void Keyboard::setPage(int idx) { contents()->setPage(idx); }

Keyboard::CapsState Keyboard::caps_state() const {
  return contents()->caps_state();
}

void Keyboard::setCapsState(CapsState caps_state) {
  contents()->setCapsState(caps_state);
}

}  // namespace roo_windows
