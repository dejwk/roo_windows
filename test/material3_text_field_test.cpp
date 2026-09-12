#include <algorithm>

#include "gtest/gtest.h"
#include "roo_windows/material3/text_field/secure_text_field.h"
#include "roo_windows/material3/text_field/text_field.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows::material3 {
namespace {
class Field : public TextField {
 public:
  using TextField::TextField;
  int changes = 0, finished = 0;
  bool confirmed = false, handled = false;
  void onTextChanged() override { ++changes; }
  void onEditFinished(bool value) override {
    ++finished;
    confirmed = value;
  }
  bool onTrailingAffordanceClicked() override { return handled; }
};
class Material3TextFieldTest
    : public test_support::RooWindowsRenderTestSized<240, 120> {};
// Verifies defaults and assistive precedence.
TEST_F(Material3TextFieldTest, DefaultsAndAssistivePrecedence) {
  Field field(context(), "Label");
  EXPECT_EQ(TextFieldVariant::kFilled, field.variant());
  EXPECT_FALSE(field.hasError());
  EXPECT_FALSE(field.readOnly());
  auto h = field.getSuggestedMinimumDimensions().height();
  field.setSupportingText("Help");
  EXPECT_GT(field.getSuggestedMinimumDimensions().height(), h);
  field.setErrorText("");
  EXPECT_TRUE(field.hasError());
  EXPECT_EQ(h, field.getSuggestedMinimumDimensions().height());
  field.clearError();
  EXPECT_GT(field.getSuggestedMinimumDimensions().height(), h);
  field.setText("value");
  field.setText("value");
  EXPECT_EQ(1, field.changes);
  field.edit();
  EXPECT_FALSE(field.isEdited());
}
// Verifies focus is idle and activation starts hardware editing.
TEST_F(Material3TextFieldTest, FocusIsIdleAndActivationStartsHardwareEditing) {
  Field field(context(), "Label");
  Task& task = app_.addTaskFullScreen(field);
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(field.requestFocus());
  EXPECT_FALSE(field.isEdited());
  KeyEvent key;
  key.code = KeyCode::kEnter;
  key.phase = KeyPhase::kDown;
  EXPECT_TRUE(field.onKeyEvent(key));
  EXPECT_TRUE(field.isEdited());
  key.phase = KeyPhase::kUp;
  EXPECT_TRUE(field.onKeyEvent(key));
  key.code = KeyCode::kCharacter;
  key.phase = KeyPhase::kDown;
  key.rune = U'é';
  EXPECT_TRUE(field.onKeyEvent(key));
  EXPECT_EQ(u8"é", field.text());
  EXPECT_EQ(1, field.changes);
  key.code = KeyCode::kEnter;
  EXPECT_TRUE(field.onKeyEvent(key));
  EXPECT_FALSE(field.isEdited());
  EXPECT_TRUE(field.confirmed);
  EXPECT_EQ(1, field.finished);
  task.navigation().clear();
}
// Verifies read only disable and live cancel.
TEST_F(Material3TextFieldTest, ReadOnlyDisableAndLiveCancel) {
  Field field(context(), "Label", TextFieldVariant::kOutlined);
  Task& task = app_.addTaskFullScreen(field);
  ASSERT_TRUE(refresh());
  field.setReadOnly(true);
  field.edit();
  EXPECT_FALSE(field.isEdited());
  field.setReadOnly(false);
  field.edit();
  ASSERT_TRUE(field.isEdited());
  task.textFieldEditor().rune(U'x');
  field.setReadOnly(true);
  EXPECT_FALSE(field.isEdited());
  EXPECT_EQ("x", field.text());
  EXPECT_FALSE(field.confirmed);
  field.setReadOnly(false);
  field.edit();
  field.setEnabled(false);
  EXPECT_FALSE(field.isEdited());
  task.navigation().clear();
}
// Verifies disabled slot background matches container.
TEST_F(Material3TextFieldTest, DisabledSlotBackgroundMatchesContainer) {
  Field field(context(), "Label");
  field.setText("value");
  field.setEnabled(false);
  Task& task = app_.addTaskFullScreen(field);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(pixelAt(8, 36), pixelAt(210, 36));
  task.navigation().clear();
}
// Verifies scroll and dirty caret equal full repaint.
TEST_F(Material3TextFieldTest, ScrollAndDirtyCaretEqualFullRepaint) {
  Field field(context(), "Label", TextFieldVariant::kOutlined);
  Task& task = app_.addTaskFullScreen(field);
  field.setPrefixText("USD ");
  field.setSuffixText(" / day");
  field.setText("A long editable line which must scroll beyond the viewport");
  ASSERT_TRUE(refresh());
  field.edit();
  ASSERT_TRUE(refresh());
  EXPECT_LT(task.textFieldEditor().draw_xoffset(), 0);
  task.textFieldEditor().moveHome();
  ASSERT_TRUE(refresh());
  EXPECT_EQ(0, task.textFieldEditor().draw_xoffset());
  std::vector<Color> dirty;
  for (int y = 0; y < 120; ++y)
    for (int x = 0; x < 240; ++x) dirty.push_back(pixelAt(x, y));
  field.invalidateInterior();
  ASSERT_TRUE(refresh());
  for (int y = 0; y < 120; ++y)
    for (int x = 0; x < 240; ++x) EXPECT_EQ(dirty[y * 240 + x], pixelAt(x, y));
  task.navigation().clear();
}
}  // namespace
// Verifies secure reveal keeps selection and trailing tap does not edit.
TEST_F(Material3TextFieldTest,
       SecureRevealKeepsSelectionAndTrailingTapDoesNotEdit) {
  SecureTextField field(context(), "Password");
  Task& task = app_.addTaskFullScreen(field);
  field.setText(u8"aé猫");
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(field.revealed());
  field.onSingleTapUp(220, 28);
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(field.revealed());
  EXPECT_FALSE(field.isEdited());
  field.edit();
  ASSERT_TRUE(field.isEdited());
  auto& editor = task.textFieldEditor();
  editor.setSelection(1, 2);
  field.setRevealed(false);
  EXPECT_EQ(1, editor.selection_begin());
  EXPECT_EQ(2, editor.selection_end());
  editor.rune(U'ß');
  EXPECT_EQ(u8"aß猫", field.text());
  field.setRevealed(true);
  editor.forwardDelete();
  EXPECT_EQ(u8"aß", field.text());
  task.navigation().clear();
}
// Verifies error affordance falls back and assistive tap does nothing.
TEST_F(Material3TextFieldTest,
       ErrorAffordanceFallsBackAndAssistiveTapDoesNothing) {
  Field field(context(), "Account");
  field.setErrorText("Try again");
  Task& task = app_.addTaskFullScreen(field);
  ASSERT_TRUE(refresh());
  field.onSingleTapUp(30, field.getSuggestedMinimumDimensions().height() - 2);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(field.isEdited());
  field.onSingleTapUp(220, 28);
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(field.isEdited());
  task.navigation().clear();
}
// Verifies detach ends session and pending mask deadline.
TEST_F(Material3TextFieldTest, DetachEndsSessionAndPendingMaskDeadline) {
  SecureTextField field(context(), "Password");
  Task& task = app_.addTaskFullScreen(field);
  ASSERT_TRUE(refresh());
  field.edit();
  task.textFieldEditor().rune(U'猫');
  ASSERT_TRUE(field.isEdited());
  task.navigation().clear();
  EXPECT_FALSE(field.isEdited());
  delay(1600);
  ASSERT_TRUE(refresh());
}
// Verifies narrow viewports keep scroll bounded.
TEST_F(Material3TextFieldTest, NarrowViewportsKeepScrollBounded) {
  Field field(context(), "A long label", TextFieldVariant::kOutlined);
  field.setText("value");
  field.setPrefixText("a wide prefix");
  field.setSuffixText("a wide suffix");
  field.setErrorText("error");
  Task& task = app_.addTask(field, roo_display::Box(0, 0, 15, 99));
  ASSERT_TRUE(refresh());
  field.requestFocus();
  KeyEvent key(KeyPhase::kDown, KeyCode::kEnter, 0, 0);
  field.onKeyEvent(key);
  ASSERT_TRUE(refresh());
  task.textFieldEditor().moveEnd();
  ASSERT_TRUE(refresh());
  EXPECT_LE(task.textFieldEditor().draw_xoffset(), 0);
  task.navigation().clear();
}
// Verifies caret selection and label transitions restore pixels.
TEST_F(Material3TextFieldTest, CaretSelectionAndLabelTransitionsRestorePixels) {
  Field field(context(), "Account", TextFieldVariant::kOutlined);
  Task& task = app_.addTaskFullScreen(field);
  ASSERT_TRUE(refresh());
  field.requestFocus();
  KeyEvent key(KeyPhase::kDown, KeyCode::kEnter, 0, 0);
  field.onKeyEvent(key);
  auto& editor = task.textFieldEditor();
  for (int step = 0; step < 6; ++step) {
    if (step == 0) editor.rune(U'A');
    if (step == 1) editor.setSelection(0, 1);
    if (step == 2) editor.del();
    if (step == 3)
      field.setText(
          "A long line requiring horizontal scrolling beyond the visible "
          "field");
    if (step == 4) context().animations().seek(field, 0, roo_time::Millis(500));
    if (step == 5) editor.cancel();
    ASSERT_TRUE(refresh());
    std::vector<Color> partial;
    for (int y = 0; y < 120; ++y)
      for (int x = 0; x < 240; ++x) partial.push_back(pixelAt(x, y));
    field.invalidateInterior();
    ASSERT_TRUE(refresh());
    for (int y = 0; y < 120; ++y)
      for (int x = 0; x < 240; ++x)
        ASSERT_EQ(partial[y * 240 + x], pixelAt(x, y)) << "step " << step;
  }
  task.navigation().clear();
}
class FieldKeys : public KeySource {
 public:
  void push(KeyEvent key) {
    events.push_back(key);
    notifyReady();
  }
  int drain(KeyEvent* out, int max) override {
    int count = 0;
    while (count < max && next < events.size()) out[count++] = events[next++];
    return count;
  }

 private:
  bool hasPendingEvents() const override { return next < events.size(); }
  std::vector<KeyEvent> events;
  size_t next = 0;
};
// Verifies physical source activates without synthetic slot tap.
TEST_F(Material3TextFieldTest, PhysicalSourceActivatesWithoutSyntheticSlotTap) {
  SecureTextField field(context(), "Password");
  Task& task = app_.addTaskFullScreen(field);
  FieldKeys keys;
  keys.connect(task);
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(field.requestFocus());
  EXPECT_FALSE(field.isEdited());
  app_.start();
  scheduler_.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);
  keys.push(
      KeyEvent(KeyPhase::kDown, KeyCode::kEnter, 0, PhysicalKey::kEnter, 0));
  keys.push(
      KeyEvent(KeyPhase::kUp, KeyCode::kEnter, 0, PhysicalKey::kEnter, 0));
  scheduler_.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(field.isEdited());
  EXPECT_FALSE(field.revealed());
  keys.push(
      KeyEvent(KeyPhase::kDown, KeyCode::kCharacter, 0, PhysicalKey::kX, U'x'));
  scheduler_.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);
  ASSERT_TRUE(refresh());
  EXPECT_EQ("x", field.text());
  keys.push(
      KeyEvent(KeyPhase::kDown, KeyCode::kEnter, 0, PhysicalKey::kEnter, 0));
  keys.push(
      KeyEvent(KeyPhase::kUp, KeyCode::kEnter, 0, PhysicalKey::kEnter, 0));
  scheduler_.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum, 1);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(field.isEdited());
  keys.disconnect();
  task.navigation().clear();
}
// Verifies the secure slot remains reserved, read-only reveal is allowed, and
// disabling or canceling a tap cannot trigger a later reveal or edit.
TEST_F(Material3TextFieldTest, SecureAffordancePolicyAndCancellation) {
  SecureTextField field(context(), "Password");
  Task& task = app_.addTaskFullScreen(field);
  field.setText("secret");
  field.setTrailingIcon(nullptr);
  field.setErrorText("Error");
  field.setReadOnly(true);
  ASSERT_TRUE(refresh());
  field.onSingleTapUp(220, 28);
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(field.revealed());
  EXPECT_FALSE(field.isEdited());
  field.setEnabled(false);
  field.onSingleTapUp(220, 28);
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(field.revealed());
  field.setEnabled(true);
  field.onDown(220, 28);
  field.onShowPress(220, 28);
  field.onCancel();
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(field.revealed());
  task.navigation().clear();
}

// Counts every output path used by field painting; duplicate writes would
// expose a background prefill followed by foreground redraw.
class FieldDisplay
    : public roo_display::OffscreenDevice<roo_display::Argb4444> {
 public:
  explicit FieldDisplay(roo::byte* data)
      : OffscreenDevice(280, 180, data, roo_display::Argb4444()),
        writes(280 * 180) {}
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
      ++writes[y * 280 + x];
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

class FieldBackdrop : public Panel {
 public:
  FieldBackdrop(ApplicationContext& context, TextField& field)
      : Panel(context) {
    add(field);
  }
  Color background() const override { return roo_display::color::LightGreen; }
  void onLayout(bool changed, const Rect& rect) override {
    Widget& field = child_at(0);
    field.layout(Rect(8, 8, width() - 9,
                      8 + field.getSuggestedMinimumDimensions().height() - 1));
  }
};

// Verifies full and dirty paint emit each pixel at most once, and shrinking
// the assistive row restores the ancestor surface without stale label/notch
// ink.
TEST(Material3TextFieldPaint, SinglePassAndAssistiveRemovalOnColoredAncestor) {
  std::vector<roo::byte> pixels(280 * 180 * 2);
  FieldDisplay device(pixels.data());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  Application app(&env, display);
  SecureTextField field(app.context(), "Password", TextFieldVariant::kOutlined);
  field.setSupportingText("Supporting message");
  field.setText("a secret");
  FieldBackdrop backdrop(app.context(), field);
  // Initialize the display before counting a logical field paint.
  ASSERT_TRUE(app.refresh());
  Task& task = app.addTaskFullScreen(backdrop);
  for (int step = 0; step < 5; ++step) {
    if (step == 1) field.setRevealed(true);
    if (step == 2) field.setErrorText("Invalid password");
    if (step == 3) field.setErrorText("");
    if (step == 4) {
      field.setText("");
      field.clearError();
    }
    device.reset();
    ASSERT_TRUE(app.refresh());
    EXPECT_LE(*std::max_element(device.writes.begin(), device.writes.end()), 1)
        << "dirty step " << step;
    std::vector<roo::byte> partial = pixels;
    backdrop.invalidateInterior();
    device.reset();
    ASSERT_TRUE(app.refresh());
    EXPECT_LE(*std::max_element(device.writes.begin(), device.writes.end()), 1)
        << "full step " << step;
    EXPECT_EQ(partial, pixels) << "step " << step;
  }
  task.navigation().clear();
}

// Adding a descender must not move the shared capital's ink or baseline.
TEST_F(Material3TextFieldTest,
       OutlineLabelCentersAscentIndependentlyOfDescenders) {
  Field field(context(), "H", TextFieldVariant::kOutlined);
  field.setText("value");
  Task& task = app_.addTaskFullScreen(field);
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(field.requestFocus());
  ASSERT_TRUE(refresh());
  const TextStyle& small = text_style_body_small();
  auto inkBand = [&]() {
    std::pair<int, int> band(small.lineHeight(), -1);
    Color bg = pixelAt(0, 0);
    auto metrics =
        small.font().getHorizontalStringMetrics("H", small.fontOptions());
    for (int y = 0; y < small.lineHeight(); ++y) {
      for (int x = Scaled(16); x < Scaled(16) + metrics.advance(); ++x) {
        if (pixelAt(x, y) != bg) {
          band.first = std::min(band.first, y);
          band.second = std::max(band.second, y);
        }
      }
    }
    return band;
  };
  auto initial = inkBand();
  ASSERT_GE(initial.second, initial.first);
  field.setLabel("Hg");
  ASSERT_TRUE(refresh());
  EXPECT_EQ(initial, inkBand());
  field.setLabel("HPassword");
  ASSERT_TRUE(refresh());
  EXPECT_EQ(initial, inkBand());
  field.setLabel("HRegion");
  ASSERT_TRUE(refresh());
  EXPECT_EQ(initial, inkBand());
  task.navigation().clear();
}

}  // namespace roo_windows::material3
