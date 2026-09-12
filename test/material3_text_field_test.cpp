#include "gtest/gtest.h"
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
TEST_F(Material3TextFieldTest, DisabledSlotBackgroundMatchesContainer) {
  Field field(context(), "Label");
  field.setText("value");
  field.setEnabled(false);
  Task& task = app_.addTaskFullScreen(field);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(pixelAt(8, 36), pixelAt(210, 36));
  task.navigation().clear();
}
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
}  // namespace roo_windows::material3
