#include <memory>

#include "gtest/gtest.h"
#include "roo_testing/system/timer.h"
#include "roo_windows/core/task.h"
#include "roo_windows/keyboard_layout/accent_demo.h"
#include "roo_windows/keyboard_layout/en_us_binary.h"
#include "roo_windows/widgets/button.h"
#include "roo_windows/widgets/text_field.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace {

// Counts every output path used by keyboard painting; duplicate writes would
// expose a background prefill followed by foreground redraw.
class KeyboardDisplay
    : public roo_display::OffscreenDevice<roo_display::Argb4444> {
 public:
  explicit KeyboardDisplay(roo::byte* data)
      : OffscreenDevice(412, 320, data, roo_display::Argb4444()),
        writes(412 * 320) {}
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
      ++writes[y * 412 + x];
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

using test_support::QuantizeToArgb4444;
using test_support::RooWindowsRenderTestSized;

class KeyboardPresentationPinTest : public RooWindowsRenderTestSized<412, 320> {
 protected:
  KeyboardPresentationPinTest()
      : field_(context(), font_body1(), "", roo_display::kLeft,
               TextField::NONE) {}

  void SetUp() override {
    task_ = &app_.addTaskFullScreen(field_);
    task_->textFieldEditor().edit(&field_, false);
    app_.keyboard().show();
    ASSERT_TRUE(refresh());
    // A 20-pixel grid unit and 40-pixel row make key coordinates explicit.
    widget().layout(Rect(0, 0, 411, 173));
  }

  void TearDown() override {
    app_.keyboard().hide();
    task_->navigation().clear();
  }

  Widget& widget() { return app_.keyboard().getContents(); }

  void tap(int x, int y) {
    widget().onDown(x, y);
    widget().onSingleTapUp(x, y);
  }

  void advance(int millis) {
    system_time_delay_micros(millis * 1000);
    scheduler_.executeEligibleTasksUpToNow();
  }

  TextField field_;
  Task* task_ = nullptr;
};

// Verifies generated layouts deliver text and page actions through the same
// editor.
TEST_F(KeyboardPresentationPinTest,
       GeneratedKeyboardDeliversTextAndSwitchesPages) {
  app_.keyboard().hide();
  Keyboard binary(context(), kbEngUSLayout());
  Task& owner =
      app_.addTask(binary.getContents(), roo_display::Box(0, 146, 412, 174));
  binary.setTask(owner);
  binary.connect(app_);
  binary.show();
  ASSERT_TRUE(refresh());
  Widget& keys = binary.getContents();
  keys.layout(Rect(0, 0, 411, 173));
  task_->textFieldEditor().edit(&field_, false);
  keys.onDown(26, 28);
  keys.onSingleTapUp(26, 28);
  EXPECT_EQ("q", field_.content());
  binary.setPage(1);
  keys.layout(Rect(0, 0, 411, 173));
  keys.onDown(26, 28);
  keys.onSingleTapUp(26, 28);
  EXPECT_EQ("q1", field_.content());
  binary.setPage(-2);
  keys.onDown(26, 28);
  keys.onSingleTapUp(26, 28);
  EXPECT_EQ("q11", field_.content());
  binary.hide();
  owner.navigation().clear();
}

// Verifies a wide circular delete face leaves its allocation corners untouched,
// while those same corners remain valid delete touch targets.
TEST_F(KeyboardPresentationPinTest, CircularActionPaintAndTouchBoundsDiffer) {
  app_.keyboard().hide();
  Keyboard binary(context(), accentDemoLayout());
  Task& owner =
      app_.addTask(binary.getContents(), roo_display::Box(0, 146, 412, 174));
  binary.setTask(owner);
  binary.connect(app_);
  binary.show();
  ASSERT_TRUE(refresh());
  Widget& keys = binary.getContents();
  keys.layout(Rect(0, 0, 411, 173));
  keys.invalidateInterior();
  ASSERT_TRUE(refresh());
  XDim dx;
  YDim dy;
  keys.getAbsoluteOffset(dx, dy);
  EXPECT_EQ(QuantizeToArgb4444(context().keyboardColorTheme().background),
            pixelAt(dx + 220, dy + 25));
  EXPECT_EQ(QuantizeToArgb4444(context().keyboardColorTheme().modifierButton),
            pixelAt(dx + 286, dy + 20));
  field_.setContent("ab");
  task_->textFieldEditor().edit(&field_, false);
  keys.onDown(220, 25);
  keys.onSingleTapUp(220, 25);
  EXPECT_EQ("a", field_.content());
  binary.hide();
  owner.navigation().clear();
}

class BinaryPopupTest : public KeyboardPresentationPinTest {
 protected:
  BinaryPopupTest() : binary_(app_.keyboard()) {}

  void SetUp() override {
    KeyboardPresentationPinTest::SetUp();
    binary_.hide();
    binary_.setLayout(accentDemoLayout());
    task_->textFieldEditor().edit(&field_, false);
    binary_.show();
    ASSERT_TRUE(refresh());
  }

  Widget& keys() { return binary_.getContents(); }

  void hold() {
    keys().onDown(166, 48);
    keys().onShowPress(166, 48);
    keys().onLongPress(166, 48);
    ASSERT_TRUE(keys().hasPresentationPin());
  }

  Keyboard& binary_;
};

// Verifies slide selection, popup repaint, and exactly one semantic accent
// commit.
TEST_F(BinaryPopupTest, SlideSelectsAccentAndClearsPopup) {
  hold();
  ASSERT_TRUE(refresh());
  keys().onLongPressMove(232, -36);
  ASSERT_TRUE(refresh());
  keys().onLongPressFinished(232, -36);
  EXPECT_EQ(u8"é", field_.content());
  EXPECT_FALSE(keys().hasPresentationPin());
  keys().onLongPressFinished(232, -36);
  EXPECT_EQ(u8"é", field_.content());
  ASSERT_TRUE(refresh());
}

// Verifies release coordinates work without a preceding MOVE and consume
// one-shot caps.
TEST_F(BinaryPopupTest, ReleaseSelectsUppercaseAndBaseHoldStillWorks) {
  binary_.setCapsState(Keyboard::CAPS_STATE_HIGH);
  hold();
  keys().onLongPressFinished(232, -36);
  EXPECT_EQ(u8"É", field_.content());
  EXPECT_EQ(Keyboard::CAPS_STATE_LOW, binary_.caps_state());
  hold();
  keys().onLongPressFinished(166, 48);
  EXPECT_EQ(u8"Ée", field_.content());
  binary_.setCapsState(Keyboard::CAPS_STATE_HIGH_LOCKED);
  hold();
  keys().onLongPressFinished(376, -36);
  EXPECT_EQ(u8"ÉeĘ", field_.content());
  EXPECT_EQ(Keyboard::CAPS_STATE_HIGH_LOCKED, binary_.caps_state());
}

// Verifies cancellation does not fall through to base-letter delivery.
TEST_F(BinaryPopupTest, OutsideAndLifecycleChangesCancelSelection) {
  for (int reason = 0; reason < 6; ++reason) {
    binary_.show();
    ASSERT_TRUE(refresh());
    hold();
    if (reason == 0) keys().onLongPressMove(-100, -100);
    if (reason == 1) binary_.hide();
    if (reason == 2) binary_.connect(app_);
    if (reason == 3) binary_.setCapsState(Keyboard::CAPS_STATE_HIGH);
    if (reason == 4) keys().hidePresentationPin();
    if (reason == 5) keys().onCancel();
    keys().onLongPressFinished(-100, -100);
    EXPECT_TRUE(field_.content().empty()) << reason;
    EXPECT_FALSE(keys().hasPresentationPin());
  }
}

// Verifies returning from outside and traversing the gap retains the chosen
// accent.
TEST_F(BinaryPopupTest, CanReturnToStripAndCrossTheCorridor) {
  hold();
  keys().onLongPressMove(-100, -100);
  keys().onLongPressMove(232, -36);
  keys().onLongPressFinished(232, 5);
  EXPECT_EQ(u8"é", field_.content());
}

// Verifies replacing layout or resizing a live popup cannot commit its stale
// key.
TEST_F(BinaryPopupTest, LayoutAndPresentationChangesCancel) {
  hold();
  binary_.setLayout(kbEngUSLayout());
  keys().onLongPressFinished(232, -36);
  EXPECT_TRUE(field_.content().empty());
  binary_.setLayout(accentDemoLayout());
  ASSERT_TRUE(refresh());
  hold();
  keys().layout(Rect(0, 0, 399, 159));
  keys().onLongPressFinished(232, -36);
  EXPECT_TRUE(field_.content().empty());
  binary_.hide();
  binary_.show();
  ASSERT_TRUE(refresh());
  hold();
  keys().setVisibility(Visibility::kInvisible);
  keys().onLongPressFinished(232, -36);
  EXPECT_TRUE(field_.content().empty());
}

// Verifies a popup too tall for its viewport retains ordinary hold-to-commit
// behavior.
TEST_F(BinaryPopupTest, UnfittableStripFallsBackToBaseLetter) {
  keys().layout(Rect(0, 0, 411, 999));
  hold();
  keys().onLongPressFinished(166, 48);
  EXPECT_EQ("e", field_.content());
}

// Verifies popup highlights, circle faces and cleanup are single-pass and match
// complete invalidation, including changes while the strip covers the keyboard.
TEST(KeyboardPaint, AlternativePopupDirtyPaintMatchesFullPaint) {
  std::vector<roo::byte> pixels(412 * 320 * 2);
  KeyboardDisplay device(pixels.data());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  app.keyboard().setLayout(accentDemoLayout());
  app.keyboard().show();
  ASSERT_TRUE(app.refresh());
  Widget& keyboard = app.keyboard().getContents();
  for (int step = 0; step < 4; ++step) {
    if (step == 0) {
      keyboard.onDown(166, 48);
      keyboard.onShowPress(166, 48);
      keyboard.onLongPress(166, 48);
    }
    if (step == 1) keyboard.onLongPressMove(232, -36);
    if (step == 2) keyboard.onLongPressMove(376, -36);
    if (step == 3) keyboard.onCancel();
    device.reset();
    ASSERT_TRUE(app.refresh());
    EXPECT_LE(*std::max_element(device.writes.begin(), device.writes.end()), 1);
    const std::vector<roo::byte> partial = pixels;
    app.root().invalidateInterior();
    device.reset();
    ASSERT_TRUE(app.refresh());
    EXPECT_LE(*std::max_element(device.writes.begin(), device.writes.end()), 1);
    EXPECT_EQ(partial, pixels) << step;
  }
  app.keyboard().hide();
}

// Verifies the preview escapes the leaf keyboard while retaining task focus,
// and disappears on cancellation without committing the pressed character.
TEST_F(KeyboardPresentationPinTest,
       PreviewEscapesKeyboardTaskAndPreservesPopupFocus) {
  static_assert(sizeof(Keyboard) == sizeof(std::unique_ptr<Widget>) +
                                        sizeof(TextInputEmitter) +
                                        sizeof(Task*));
  EXPECT_EQ(ParentClipMode::kClipped, widget().getParentClipMode());
  Task* owner = widget().getTask();
  ASSERT_NE(nullptr, owner);
  ASSERT_TRUE(widget().requestFocus());
  EXPECT_EQ(&widget(), owner->focus().focused());
  std::vector<Widget*> path;
  EXPECT_TRUE(widget().fillTouchTargetPath(26, 28, path));
  ASSERT_EQ(1u, path.size());
  EXPECT_EQ(&widget(), path.back());

  XDim dx;
  YDim dy;
  widget().getAbsoluteOffset(dx, dy);
  const XDim preview_center_x = dx + 25;
  const YDim preview_center_y = dy + 12 - Scaled(2) - Scaled(32) / 2 - 1;
  ASSERT_LT(preview_center_y, dy);
  widget().onDown(26, 28);
  widget().onShowPress(26, 28);
  EXPECT_TRUE(widget().hasPresentationPin());
  EXPECT_EQ(&widget(), owner->focus().focused());
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(context().keyboardColorTheme().normalButton),
            pixelAt(preview_center_x + Scaled(12), preview_center_y));
  widget().onCancel();
  EXPECT_FALSE(widget().hasPresentationPin());
  EXPECT_TRUE(field_.content().empty());
  EXPECT_EQ(&widget(), owner->focus().focused());
}

// Verifies quick taps, shifted text, caps lock, long-held text, and wide space
// use the original down key and share a single active press.
TEST_F(KeyboardPresentationPinTest, TextShiftCapsLockAndSpace) {
  tap(26, 28);
  EXPECT_EQ("q", field_.content());
  tap(36, 108);  // Shift.
  EXPECT_EQ(Keyboard::CAPS_STATE_HIGH, app_.keyboard().caps_state());
  tap(46, 68);  // A, on the staggered second row.
  EXPECT_EQ("qA", field_.content());
  EXPECT_EQ(Keyboard::CAPS_STATE_LOW, app_.keyboard().caps_state());
  widget().onDown(36, 108);
  widget().onShowPress(36, 108);
  widget().onLongPress(36, 108);
  widget().onLongPressFinished(36, 108);
  EXPECT_EQ(Keyboard::CAPS_STATE_HIGH_LOCKED, app_.keyboard().caps_state());
  widget().onDown(26, 28);
  widget().onShowPress(26, 28);
  widget().onLongPress(26, 28);
  widget().onLongPressFinished(28, 28);
  tap(260, 148);  // Right half of wide space.
  EXPECT_EQ("qAQ ", field_.content());
  EXPECT_EQ(Keyboard::CAPS_STATE_HIGH_LOCKED, app_.keyboard().caps_state());
  tap(36, 108);
  widget().onDown(26, 28);
  widget().onDown(66, 28);  // Replaces the earlier q press.
  widget().onSingleTapUp(67, 28);
  EXPECT_EQ("qAQ w", field_.content());
  tap(10, 68);  // Empty area before the staggered row.
  tap(26, 0);   // Top padding.
  EXPECT_EQ("qAQ w", field_.content());
}

// Verifies page switching consumes the current gesture, Unicode is emitted
// directly from the layout table, and hidden/page-changed previews are removed.
TEST_F(KeyboardPresentationPinTest, PageSwitchUnicodeAndPreviewCleanup) {
  widget().onDown(26, 28);
  widget().onShowPress(26, 28);
  ASSERT_TRUE(widget().hasPresentationPin());
  app_.keyboard().setPage(1);
  EXPECT_FALSE(widget().hasPresentationPin());
  widget().onSingleTapUp(26, 28);
  EXPECT_TRUE(field_.content().empty());
  tap(26, 28);
  EXPECT_EQ("1", field_.content());
  tap(36, 108);  // Symbols.
  tap(186, 28);  // Greek mu.
  EXPECT_EQ(u8"1μ", field_.content());
  tap(36, 148);  // Letters.
  EXPECT_EQ(u8"1μ", field_.content());
  tap(26, 28);
  EXPECT_EQ(u8"1μq", field_.content());
  widget().onDown(26, 28);
  widget().onShowPress(26, 28);
  app_.keyboard().hide();
  EXPECT_FALSE(widget().hasPresentationPin());
  widget().onSingleTapUp(26, 28);
  EXPECT_EQ(u8"1μq", field_.content());
}

// Verifies delete fires immediately, repeats after 400 ms then every 60 ms,
// and stops on release, cancellation, page change, and hide.
TEST_F(KeyboardPresentationPinTest, DeleteRepeatStopsAtEveryTerminalPath) {
  for (int terminal = 0; terminal < 4; ++terminal) {
    app_.keyboard().show();
    field_.setContent("abcdefghij");
    task_->textFieldEditor().moveEnd(false);
    widget().onDown(376, 108);
    widget().onShowPress(376, 108);
    EXPECT_EQ("abcdefghi", field_.content());
    advance(399);
    EXPECT_EQ("abcdefghi", field_.content());
    advance(1);
    EXPECT_EQ("abcdefgh", field_.content());
    advance(60);
    EXPECT_EQ("abcdefg", field_.content());
    switch (terminal) {
      case 0:
        widget().onLongPressFinished(376, 108);
        break;
      case 1:
        widget().onCancel();
        break;
      case 2:
        app_.keyboard().setPage(1);
        break;
      case 3:
        app_.keyboard().hide();
        break;
    }
    advance(500);
    EXPECT_EQ("abcdefg", field_.content());
  }
}

// Verifies Done completes editing and clears any active keyboard state.
TEST_F(KeyboardPresentationPinTest, EnterFinishesEditing) {
  tap(26, 28);
  tap(376, 148);
  EXPECT_EQ("q", field_.content());
  EXPECT_FALSE(task_->textFieldEditor().isEdited(&field_));
  EXPECT_FALSE(widget().hasPresentationPin());
}

// Verifies width and height are not transposed, including preferred size.
TEST_F(KeyboardPresentationPinTest, MeasuresWidthAndHeightIndependently) {
  Dimensions size =
      widget().measure(WidthSpec::Exactly(412), HeightSpec::Exactly(174));
  EXPECT_EQ(412, size.width());
  EXPECT_EQ(174, size.height());
  size =
      widget().measure(WidthSpec::Unspecified(0), HeightSpec::Unspecified(0));
  EXPECT_EQ(312, size.width());
  EXPECT_EQ(54, size.height());
}

// Verifies final pixels are emitted once, dirty presses stay local, and partial
// updates match full invalidation for keys, caps changes, pages, and previews.
TEST(KeyboardPaint, SinglePassAndDirtyPaintMatchesFullPaint) {
  std::vector<roo::byte> pixels(412 * 320 * 2);
  KeyboardDisplay device(pixels.data());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  ASSERT_TRUE(app.refresh());
  app.keyboard().show();
  Widget& keyboard = app.keyboard().getContents();
  for (int step = 0; step < 9; ++step) {
    if (step == 1) {
      keyboard.onDown(206, 135);
      keyboard.onShowPress(206, 135);
    }
    if (step == 2 || step == 7) keyboard.onCancel();
    if (step == 3) {
      keyboard.onDown(36, 99);
      keyboard.onShowPress(36, 99);
    }
    if (step == 4) keyboard.onLongPress(36, 99);
    if (step == 5) keyboard.onLongPressFinished(36, 99);
    if (step == 6) {
      keyboard.onDown(26, 27);
      keyboard.onShowPress(26, 27);
    }
    if (step == 8) app.keyboard().setPage(1);
    device.reset();
    ASSERT_TRUE(app.refresh());
    EXPECT_LE(*std::max_element(device.writes.begin(), device.writes.end()), 1)
        << "dirty step " << step;
    if (step == 1 || step == 2) {
      int touched = std::count(device.writes.begin(), device.writes.end(), 1);
      EXPECT_GT(touched, 0);
      EXPECT_LE(touched, 6000) << "Only the space key should repaint";
    }
    const std::vector<roo::byte> partial = pixels;
    keyboard.invalidateInterior();
    device.reset();
    ASSERT_TRUE(app.refresh());
    EXPECT_LE(*std::max_element(device.writes.begin(), device.writes.end()), 1)
        << "full step " << step;
    EXPECT_EQ(partial, pixels) << "step " << step;
  }
  app.keyboard().hide();
}

// Models the old first-row button layout independently of KeyboardWidget.
class LegacyKeyboardFirstRow : public Panel {
 public:
  LegacyKeyboardFirstRow(ApplicationContext& context, const char* keys)
      : Panel(context) {
    for (int i = 0; i < 10; ++i) {
      auto button =
          std::make_unique<SimpleButton>(context, std::string(1, keys[i]));
      button->setFont(font_body1());
      button->setCornerRadius(Scaled(3));
      button->setInteriorColor(context.keyboardColorTheme().normalButton);
      button->setOutlineColor(context.keyboardColorTheme().normalButton);
      button->setContentColor(context.keyboardColorTheme().text);
      add(std::move(button));
    }
  }

  Color background() const override {
    return context().keyboardColorTheme().background;
  }

  void onLayout(bool, const Rect&) override {
    const int cell_width = std::max(5, (width() - 12) / 20);
    const int row_height = std::max(10, (height() - 14) / 4);
    const int x = std::max(0, (width() - 20 * cell_width) / 2);
    const int y = std::max(0, (height() - 2 - 4 * row_height) / 2) + 2;
    const int mx = std::max(1, cell_width / 10);
    const int my = std::max(1, row_height / 10);
    for (int i = 0; i < 10; ++i) {
      child_at(i).layout(Rect(x + i * 2 * cell_width + mx, y + my,
                              x + (i + 1) * 2 * cell_width - mx - 1,
                              y + row_height - my - 1));
    }
  }
};

// Verifies key widths, spacing, corners, and glyph clipping match the previous
// SimpleButton rendering on a 320-pixel display, including wide capital glyphs.
TEST(KeyboardPaint, FirstRowMatchesOriginalButtons) {
  std::vector<roo::byte> pixels(320 * 240 * 2);
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      320, 240, pixels.data(), roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  app.keyboard().show();
  ASSERT_TRUE(app.refresh());
  Widget& keyboard = app.keyboard().getContents();
  EXPECT_EQ(320, keyboard.width());
  EXPECT_EQ(120, keyboard.height());

  for (int caps = 0; caps < 2; ++caps) {
    app.keyboard().setCapsState(caps ? Keyboard::CAPS_STATE_HIGH
                                     : Keyboard::CAPS_STATE_LOW);
    keyboard.invalidateInterior();
    ASSERT_TRUE(app.refresh());
    std::vector<roo::byte> reference_pixels(320 * 120 * 2);
    roo_display::OffscreenDevice<roo_display::Argb4444> reference_device(
        320, 120, reference_pixels.data(), roo_display::Argb4444());
    roo_display::Display reference_display(reference_device);
    Application reference_app(&env, reference_display);
    LegacyKeyboardFirstRow reference(reference_app.context(),
                                     caps ? "QWERTYUIOP" : "qwertyuiop");
    Task& reference_task = reference_app.addTaskFullScreen(reference);
    ASSERT_TRUE(reference_app.refresh());
    // Compare the entire first row, including the outer keyboard margins.
    for (int y = 0; y < 35; ++y) {
      for (int x = 0; x < 320 * 2; ++x) {
        ASSERT_EQ(reference_pixels[y * 640 + x], pixels[(120 + y) * 640 + x])
            << "caps " << caps << ", pixel " << x / 2 << ", " << y;
      }
    }
    reference_task.navigation().clear();
  }
}

// Verifies lower-row previews and simultaneous editor damage do not widen a
// key press/release into a whole-keyboard repaint. Invalidated repaint still
// produces exactly the same image as the partial repaint.
TEST(KeyboardPaint, TextPressAndReleaseWithEditorDamageStayLocal) {
  std::vector<roo::byte> pixels(412 * 320 * 2);
  KeyboardDisplay device(pixels.data());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  TextField field(app.context(), font_body1(), "", roo_display::kLeft,
                  TextField::NONE);
  Task& task = app.addTaskFullScreen(field);
  task.textFieldEditor().edit(&field, false);
  app.keyboard().show();
  ASSERT_TRUE(app.refresh());
  Widget& keyboard = app.keyboard().getContents();
  for (int step = 0; step < 4; ++step) {
    if ((step % 2) == 0) {
      keyboard.onDown(206, 99);  // V, with its preview inside the keyboard.
      keyboard.onShowPress(206, 99);
    } else {
      keyboard.onSingleTapUp(206, 99);
    }
    // MainWindow now has a much wider aggregate redraw bound than the key.
    field.invalidateInterior();
    device.reset();
    ASSERT_TRUE(app.refresh());
    int keyboard_pixels = 0;
    for (int y = 160; y < 320; ++y) {
      for (int x = 0; x < 412; ++x) {
        int writes = device.writes[y * 412 + x];
        ASSERT_LE(writes, 1);
        if (writes == 0) continue;
        ++keyboard_pixels;
        ASSERT_GE(x, 164) << "step " << step;
        ASSERT_LE(x, 248) << "step " << step;
        ASSERT_LT(y, 277) << "step " << step;
      }
    }
    EXPECT_GT(keyboard_pixels, 0);
    EXPECT_LT(keyboard_pixels, 8500);
    const std::vector<roo::byte> partial = pixels;
    keyboard.invalidateInterior();
    ASSERT_TRUE(app.refresh());
    EXPECT_EQ(partial, pixels) << "step " << step;
  }
  EXPECT_EQ("vv", field.content());
  app.keyboard().hide();
  task.navigation().clear();
}

}  // namespace
}  // namespace roo_windows
