#include "roo_windows/material2/text_field.h"

#include <functional>

#include "gtest/gtest.h"
#include "roo_fonts/NotoSans_Regular/14.h"
#include "roo_windows/core/text_field_editor.h"
#include "roo_windows/material2/typography.h"
#include "roo_windows_render_test_support.h"
namespace roo_windows {
namespace {
// Counts every output path used by keyboard painting; duplicate writes would
// expose a background prefill followed by foreground redraw.
class TextFieldDisplay
    : public roo_display::OffscreenDevice<roo_display::Argb4444> {
 public:
  explicit TextFieldDisplay(roo::byte* data)
      : OffscreenDevice(180, 80, data, roo_display::Argb4444()),
        writes(180 * 80) {}
  void reset() { std::fill(writes.begin(), writes.end(), 0); }
  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  roo_display::BlendingMode mode) override {
    left = x = x0;
    right = x1;
    y = y0;
    OffscreenDevice::setAddress(x0, y0, x1, y1, mode);
  }
  void count(uint32_t n) {
    while (n--) {
      ++writes[y * 180 + x];
      if (++x > right) {
        x = left;
        ++y;
      }
    }
  }
  void write(Color* colors, uint32_t n) override {
    count(n);
    OffscreenDevice::write(colors, n);
  }
  void fill(Color color, uint32_t n) override {
    count(n);
    OffscreenDevice::fill(color, n);
  }
  void writePixels(roo_display::BlendingMode mode, Color* colors, int16_t* xs,
                   int16_t* ys, uint16_t n) override {
    for (int i = 0; i < n; ++i) {
      setAddress(xs[i], ys[i], xs[i], ys[i], mode);
      write(colors + i, 1);
    }
  }
  void fillPixels(roo_display::BlendingMode mode, Color color, int16_t* xs,
                  int16_t* ys, uint16_t n) override {
    for (int i = 0; i < n; ++i) {
      setAddress(xs[i], ys[i], xs[i], ys[i], mode);
      fill(color, 1);
    }
  }
  void writeRects(roo_display::BlendingMode mode, Color* colors, int16_t* x0,
                  int16_t* y0, int16_t* x1, int16_t* y1, uint16_t n) override {
    for (int i = 0; i < n; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(colors[i], (x1[i] - x0[i] + 1) * (y1[i] - y0[i] + 1));
    }
  }
  void fillRects(roo_display::BlendingMode mode, Color color, int16_t* x0,
                 int16_t* y0, int16_t* x1, int16_t* y1, uint16_t n) override {
    for (int i = 0; i < n; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(color, (x1[i] - x0[i] + 1) * (y1[i] - y0[i] + 1));
    }
  }
  std::vector<uint8_t> writes;

 private:
  int left = 0, right = 0, x = 0, y = 0;
};

class Target : public Widget, public internal::TextEditTarget {
 public:
  using Widget::Widget;
  Dimensions getSuggestedMinimumDimensions() const override {
    return {100, 40};
  }
  Widget& editWidget() override { return *this; }
  std::string& textBuffer() override { return buffer; }
  roo::string_view value() const override { return buffer; }
  const roo_display::Font& textFont() const override {
    return roo_display::font_NotoSans_Regular_14();
  }
  bool obscureText() const override { return masked; }
  void notifyEditVisualChange() override { setDirty(); }
  void notifyTextChanged() override { ++changes; }
  void onEditFinished(bool confirmed) override {
    result = confirmed ? 1 : 0;
    if (finished) finished();
  }
  std::function<void()> finished;
  std::string buffer;
  bool masked = false;
  int changes = 0, result = -1;
};
class TextFieldTest : public test_support::RooWindowsRenderTestSized<180, 80> {
};
// Verifies overflowing text and underline padding never prefill settled pixels,
// including dirty cursor redraws.
TEST_F(TextFieldTest, OverflowPaintWritesEachPixelOnce) {
  roo::byte pixels[180 * 80 * 2] = {};
  TextFieldDisplay device(pixels);
  roo_display::Display display(device);
  Application app(&env_, display);
  material2::TextField field(app.context(), material2::font_body1(), "",
                             roo_display::kLeft | roo_display::kMiddle,
                             material2::TextField::UNDERLINE);
  Task& task = app.addTaskFullScreen(field);
  field.setContent(std::string(80, 'W'));
  app.refresh();
  task.textFieldEditor().edit(&field, false);
  for (int pass = 0; pass < 2; ++pass) {
    device.reset();
    field.notifyEditVisualChange();
    app.refresh();
    EXPECT_LE(*std::max_element(device.writes.begin(), device.writes.end()), 1);
  }
  task.navigation().clear();
}

// Verifies abstract target unicode selection and completion.
TEST_F(TextFieldTest, AbstractTargetUnicodeSelectionAndCompletion) {
  Target target(context());
  Task& task = app_.addTaskFullScreen(target);
  auto& editor = task.textFieldEditor();
  target.masked = true;
  target.buffer = u8"aé猫";
  editor.edit(&target, false);
  ASSERT_EQ(3u, editor.glyphs().size());
  editor.setSelection(1, 2);
  editor.rune(U'ß');
  EXPECT_EQ(u8"aß猫", target.buffer);
  editor.forwardDelete();
  EXPECT_EQ(u8"aß", target.buffer);
  editor.del();
  EXPECT_EQ("a", target.buffer);
  EXPECT_EQ(3, target.changes);
  editor.cancel();
  EXPECT_EQ(0, target.result);
  EXPECT_EQ("a", target.buffer);
  editor.edit(&target, false);
  editor.enter();
  EXPECT_EQ(1, target.result);
  task.navigation().clear();
}
// Verifies empty legacy hint does not enter editor metrics.
TEST_F(TextFieldTest, EmptyLegacyHintDoesNotEnterEditorMetrics) {
  material2::TextField field(context(), roo_windows::material2::font_body1(),
                             "Hint", roo_display::kLeft | roo_display::kMiddle,
                             material2::TextField::UNDERLINE);
  Task& task = app_.addTaskFullScreen(field);
  auto& editor = task.textFieldEditor();
  editor.edit(&field, false);
  EXPECT_TRUE(editor.glyphs().empty());
  editor.moveEnd(true);
  editor.rune(U'x');
  EXPECT_EQ("x", field.content());
  task.navigation().clear();
}
// Verifies completion may reenter and destroy old target.
TEST_F(TextFieldTest, CompletionMayReenterAndDestroyOldTarget) {
  auto first = std::make_unique<Target>(context());
  Target next(context());
  Task& first_task = app_.addTaskFullScreen(*first);
  Task& next_task = app_.addTaskFullScreen(next);
  first_task.textFieldEditor().edit(first.get(), false);
  first->finished = [&] {
    first_task.navigation().clear();
    // Defer destroying the std::function until its active invocation returns.
    next_task.textFieldEditor().edit(&next, false);
  };
  first_task.textFieldEditor().enter();
  first.reset();
  EXPECT_TRUE(next_task.textFieldEditor().isEdited(&next));
  next_task.navigation().clear();
}
// Verifies cross task activation respects reentrant editor.
TEST_F(TextFieldTest, CrossTaskActivationRespectsReentrantEditor) {
  Target first(context()), second(context()), third(context());
  Task& a = app_.addTaskFullScreen(first);
  Task& b = app_.addTaskFullScreen(second);
  Task& c = app_.addTaskFullScreen(third);
  a.textFieldEditor().edit(&first, false);
  first.finished = [&] { c.textFieldEditor().edit(&third, false); };
  b.textFieldEditor().edit(&second, false);
  EXPECT_FALSE(b.textFieldEditor().isEdited(&second));
  EXPECT_TRUE(c.textFieldEditor().isEdited(&third));
  TextInputEmitter input;
  input.connect(app_);
  EXPECT_TRUE(input.commitRune(U'x'));
  EXPECT_EQ("x", third.buffer);
  first.finished = {};
  a.navigation().clear();
  b.navigation().clear();
  c.navigation().clear();
}
// Verifies mask expiry preserves selection and filters controls.
TEST_F(TextFieldTest, MaskExpiryPreservesSelectionAndFiltersControls) {
  Target target(context());
  target.masked = true;
  Task& task = app_.addTaskFullScreen(target);
  auto& editor = task.textFieldEditor();
  editor.edit(&target, false);
  editor.rune(U'é');
  editor.rune(U'猫');
  editor.setSelection(0, 1);
  delay(1600);
  scheduler_.executeEligibleTasksUpToNow();
  refresh();
  EXPECT_FALSE(editor.lastGlyphRecentlyEntered());
  EXPECT_EQ(0, editor.selection_begin());
  EXPECT_EQ(1, editor.selection_end());
  editor.rune(U'\n');
  editor.rune(0xd800);
  EXPECT_EQ(u8"é猫", target.buffer);
  editor.rune(U'ß');
  EXPECT_EQ(u8"ß猫", target.buffer);
  task.navigation().clear();
}
// Verifies mask changes preserve Unicode selection, while replacing the buffer
// clears selection and cancels the recent-glyph deadline.
TEST_F(TextFieldTest, LegacyMaskAndBufferChangesRefreshActiveMetrics) {
  material2::TextField field(context(), roo_windows::material2::font_body1(),
                             "Hint", roo_display::kLeft | roo_display::kMiddle,
                             material2::TextField::UNDERLINE);
  Task& task = app_.addTaskFullScreen(field);
  field.setContent(u8"aé猫");
  auto& editor = task.textFieldEditor();
  editor.edit(&field, false);
  editor.setSelection(1, 2);
  field.setStarred(true);
  EXPECT_EQ(1, editor.selection_begin());
  EXPECT_EQ(2, editor.selection_end());
  editor.rune(U'ß');
  EXPECT_EQ(u8"aß猫", field.content());
  editor.moveEnd();
  editor.rune(U'x');
  EXPECT_TRUE(editor.lastGlyphRecentlyEntered());
  field.setContent("new");
  EXPECT_FALSE(editor.lastGlyphRecentlyEntered());
  EXPECT_FALSE(editor.has_selection());
  EXPECT_EQ(3, editor.cursor_position());
  editor.del();
  EXPECT_FALSE(editor.lastGlyphRecentlyEntered());
  EXPECT_EQ("ne", field.content());
  task.navigation().clear();
}
}  // namespace
}  // namespace roo_windows
