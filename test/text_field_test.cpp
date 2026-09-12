#include "gtest/gtest.h"
#include "roo_windows/widgets/text_field.h"
#include "roo_windows_render_test_support.h"
namespace roo_windows { namespace {
class Target : public BasicWidget, public internal::TextEditTarget {
 public:
  using BasicWidget::BasicWidget;
  Dimensions getSuggestedMinimumDimensions() const override { return {100, 40}; }
  Widget& editWidget() override { return *this; }
  const Widget& editWidget() const override { return *this; }
  std::string& textBuffer() override { return value; }
  const std::string& textBuffer() const override { return value; }
  const roo_display::Font& textFont() const override { return font_body1(); }
  bool obscureText() const override { return masked; }
  void notifyEditVisualChange() override { setDirty(); }
  void notifyTextChanged() override { ++changes; }
  void onEditFinished(bool confirmed) override { result = confirmed ? 1 : 0; }
  std::string value;
  bool masked = false;
  int changes = 0, result = -1;
};
class TextFieldTest : public test_support::RooWindowsRenderTestSized<180, 80> {};
TEST_F(TextFieldTest, AbstractTargetUnicodeSelectionAndCompletion) {
  Target target(context());
  Task& task = app_.addTaskFullScreen(target);
  auto& editor = task.textFieldEditor();
  target.masked = true;
  target.value = u8"aé猫";
  editor.edit(&target, false);
  ASSERT_EQ(3u, editor.glyphs().size());
  editor.setSelection(1, 2);
  editor.rune(U'ß');
  EXPECT_EQ(u8"aß猫", target.value);
  editor.forwardDelete();
  EXPECT_EQ(u8"aß", target.value);
  editor.del();
  EXPECT_EQ("a", target.value);
  EXPECT_EQ(3, target.changes);
  editor.cancel();
  EXPECT_EQ(0, target.result);
  EXPECT_EQ("a", target.value);
  editor.edit(&target, false);
  editor.enter();
  EXPECT_EQ(1, target.result);
  task.navigation().clear();
}
TEST_F(TextFieldTest, EmptyLegacyHintDoesNotEnterEditorMetrics) {
  TextField field(context(), font_body1(), "Hint", roo_display::kLeft | roo_display::kMiddle, TextField::UNDERLINE);
  Task& task = app_.addTaskFullScreen(field);
  auto& editor = task.textFieldEditor();
  editor.edit(&field, false);
  EXPECT_TRUE(editor.glyphs().empty());
  editor.moveEnd(true);
  editor.rune(U'x');
  EXPECT_EQ("x", field.content());
  task.navigation().clear();
}
} }
