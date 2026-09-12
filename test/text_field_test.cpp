#include "roo_windows/widgets/text_field.h"

#include <functional>

#include "gtest/gtest.h"
#include "roo_windows_render_test_support.h"
namespace roo_windows {
namespace {
class Target : public BasicWidget, public internal::TextEditTarget {
 public:
  using BasicWidget::BasicWidget;
  Dimensions getSuggestedMinimumDimensions() const override {
    return {100, 40};
  }
  Widget& editWidget() override { return *this; }
  std::string& textBuffer() override { return buffer; }
  roo::string_view value() const override { return buffer; }
  const roo_display::Font& textFont() const override { return font_body1(); }
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
  TextField field(context(), font_body1(), "Hint",
                  roo_display::kLeft | roo_display::kMiddle,
                  TextField::UNDERLINE);
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
  ASSERT_TRUE(refresh());
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
  TextField field(context(), font_body1(), "Hint",
                  roo_display::kLeft | roo_display::kMiddle,
                  TextField::UNDERLINE);
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
