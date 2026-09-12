#include "gtest/gtest.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/text_field/text_field.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows::material3 {
class FullWidthColumn : public FlexLayout {
 public:
  explicit FullWidthColumn(ApplicationContext& context)
      : FlexLayout(context, FlexDirection::kColumn) {}
  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::WrapContentHeight());
  }
};

Rect ScreenBounds(const Widget& widget) {
  Rect rect = widget.parent_bounds();
  for (const Widget* parent = widget.parent(); parent != nullptr;
       parent = parent->parent()) {
    rect = rect.translate(parent->offsetLeft(), parent->offsetTop());
  }
  return rect;
}

class KeyboardAvoidanceTest
    : public test_support::RooWindowsRenderTestSized<240, 320> {};

// Verifies a form that previously fit can scroll its bottom field above the
// keyboard, and hiding the keyboard restores the viewport and normal limits.
TEST_F(KeyboardAvoidanceTest, ResizesScrollViewportAndPreservesLiveSession) {
  TextField first(context(), "First"), second(context(), "Second"),
      third(context(), "Third"), last(context(), "Last");
  FullWidthColumn form(context());
  form.setPadding(Padding(Scaled(8)));
  form.setGap(Scaled(8));
  form.add(first);
  form.add(second);
  form.add(third);
  form.add(last);
  SimpleScrollablePanel scroller(context(), form);
  Task& task = app_.addTaskFullScreen(scroller);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(240 - 2 * Scaled(8) - last.getMargins().left() -
                last.getMargins().right(),
            last.width());
  EXPECT_EQ(320, scroller.height());
  last.setText("value");
  last.edit();
  task.textFieldEditor().setSelection(1, 3);
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(last.isEdited());
  EXPECT_EQ(160, scroller.height());
  EXPECT_GE(ScreenBounds(last).yMin(), 0);
  EXPECT_LT(ScreenBounds(last).yMax(), 160);
  EXPECT_EQ(1, task.textFieldEditor().selection_begin());
  EXPECT_EQ(3, task.textFieldEditor().selection_end());
  task.textFieldEditor().rune(U'X');
  EXPECT_EQ("vXue", last.text());
  task.textFieldEditor().enter();
  ASSERT_TRUE(refresh());
  EXPECT_EQ(320, scroller.height());
  EXPECT_FALSE(last.isEdited());
  task.navigation().clear();
}

class StaticKeyboardForm : public Panel {
 public:
  StaticKeyboardForm(ApplicationContext& context, TextField& field)
      : Panel(context) {
    add(field, Rect(8, 240, 231,
                    240 + field.getSuggestedMinimumDimensions().height() - 1));
  }
};

// Verifies static content pans inside its own task, returns when input ends,
// and physical-key activation does not move the form or open the keyboard.
TEST_F(KeyboardAvoidanceTest, PansStaticFormAndRestoresOnClose) {
  TextField field(context(), "Bottom");
  StaticKeyboardForm form(context(), field);
  Task& task = app_.addTaskFullScreen(form);
  ASSERT_TRUE(refresh());
  Rect original = form.parent_bounds();
  field.edit();
  ASSERT_TRUE(refresh());
  ASSERT_TRUE(field.isEdited());
  EXPECT_LT(form.offsetTop(), 0);
  EXPECT_GE(ScreenBounds(field).yMin(), 0);
  EXPECT_LT(ScreenBounds(field).yMax(), 160);
  task.textFieldEditor().cancel();
  ASSERT_TRUE(refresh());
  EXPECT_EQ(original, form.parent_bounds());
  field.onKeyEvent(KeyEvent(KeyPhase::kDown, KeyCode::kEnter, 0, 0));
  ASSERT_TRUE(refresh());
  EXPECT_TRUE(field.isEdited());
  EXPECT_EQ(original, form.parent_bounds());
  task.navigation().clear();
}
}  // namespace roo_windows::material3
