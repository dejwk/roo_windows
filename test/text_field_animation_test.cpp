#include "gtest/gtest.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/widgets/text_field.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace {

using test_support::RooWindowsRenderTestSized;

class TestTextField : public TextField {
 public:
  explicit TestTextField(ApplicationContext& context)
      : TextField(context, font_body1(), "Type here",
                  roo_display::kLeft | roo_display::kMiddle,
                  TextField::UNDERLINE) {}

  bool caretActive() const {
    return context().animations().contains(*this, kCaret);
  }

  AnimationStatus seekCaret(int64_t millis) {
    return context().animations().seek(*this, kCaret,
                                       roo_time::Millis(millis));
  }

  int caretFrameCount() const { return caret_frame_count_; }

 protected:
  void onAnimationFrame(AnimationTag tag,
                        const AnimationSample& sample) override {
    if (tag == kCaret) ++caret_frame_count_;
    TextField::onAnimationFrame(tag, sample);
  }

 private:
  int caret_frame_count_ = 0;
};

class TextFieldAnimationTest : public RooWindowsRenderTestSized<180, 48> {
 protected:
  struct AddedField {
    TestTextField* field;
    Task* task;
  };

  ~TextFieldAnimationTest() override {
    for (Task* task : tasks_) task->navigation().clear();
  }

  AddedField AddField(int16_t y = 4) {
    auto field = std::make_unique<TestTextField>(context());
    TestTextField* field_ptr = field.get();
    fields_.push_back(std::move(field));
    Task& task = app_.addTask(*field_ptr,
                              roo_display::Box(4, y, 175, y + 27));
    tasks_.push_back(&task);
    return AddedField{field_ptr, &task};
  }

  std::vector<std::unique_ptr<TestTextField>> fields_;
  std::vector<Task*> tasks_;
};

TEST_F(TextFieldAnimationTest, MissedBoundariesUseElapsedParity) {
  AddedField added = AddField();
  refresh();
  added.task->textFieldEditor().edit(added.field, false);
  refresh();
  EXPECT_TRUE(added.field->caretActive());
  EXPECT_TRUE(added.task->textFieldEditor().isBlinkingCursorNowOn());

  ASSERT_EQ(AnimationStatus::kOk, added.field->seekCaret(500));
  refresh();
  EXPECT_FALSE(added.task->textFieldEditor().isBlinkingCursorNowOn());

  ASSERT_EQ(AnimationStatus::kOk, added.field->seekCaret(1250));
  refresh();
  EXPECT_TRUE(added.task->textFieldEditor().isBlinkingCursorNowOn());

  ASSERT_EQ(AnimationStatus::kOk, added.field->seekCaret(1750));
  refresh();
  EXPECT_FALSE(added.task->textFieldEditor().isBlinkingCursorNowOn());
}

TEST_F(TextFieldAnimationTest, EditingRestartsAtVisiblePhaseZero) {
  AddedField added = AddField();
  refresh();
  TextFieldEditor& editor = added.task->textFieldEditor();
  editor.edit(added.field, false);
  refresh();
  ASSERT_EQ(AnimationStatus::kOk, added.field->seekCaret(500));
  refresh();
  ASSERT_FALSE(editor.isBlinkingCursorNowOn());

  editor.rune(U'x');
  EXPECT_TRUE(editor.isBlinkingCursorNowOn());
  EXPECT_TRUE(added.field->caretActive());
  refresh();
  EXPECT_TRUE(editor.isBlinkingCursorNowOn());
  EXPECT_EQ("x", added.field->content());
}

TEST_F(TextFieldAnimationTest, FocusTransferAcrossTasksCancelsOldTarget) {
  AddedField first = AddField();
  refresh();
  first.task->textFieldEditor().edit(first.field, false);
  refresh();
  ASSERT_TRUE(first.field->caretActive());

  AddedField second = AddField(8);
  refresh();
  second.task->textFieldEditor().edit(second.field, false);
  refresh();

  EXPECT_FALSE(first.field->caretActive());
  EXPECT_FALSE(first.task->textFieldEditor().isEdited(first.field));
  EXPECT_TRUE(second.field->caretActive());
  EXPECT_TRUE(second.task->textFieldEditor().isEdited(second.field));
}

TEST_F(TextFieldAnimationTest, HideAndShowDoesNotResumeStaleCaret) {
  AddedField added = AddField();
  refresh();
  ASSERT_TRUE(added.field->requestFocus());
  ASSERT_TRUE(added.field->caretActive());

  added.task->setVisible(false);
  EXPECT_FALSE(added.field->caretActive());
  EXPECT_FALSE(added.task->textFieldEditor().isEdited(added.field));
  refresh();

  added.task->setVisible(true);
  refresh();
  delay(550);
  refresh();
  EXPECT_FALSE(added.field->caretActive());
  EXPECT_FALSE(added.task->textFieldEditor().isEdited(added.field));
}

TEST_F(TextFieldAnimationTest, DestroyingTargetClearsEditorAndTrack) {
  AddedField added = AddField();
  refresh();
  ASSERT_TRUE(added.field->requestFocus());
  ASSERT_TRUE(added.field->caretActive());
  const TextField* former_address = added.field;

  added.task->navigation().clear();
  fields_.erase(fields_.begin());
  refresh();
  EXPECT_FALSE(added.task->textFieldEditor().isEdited(former_address));
}

TEST_F(TextFieldAnimationTest, SlowCaretDoesNotPollAtFrameCadence) {
  AddedField added = AddField();
  refresh();
  added.task->textFieldEditor().edit(added.field, false);
  refresh();
  int initial_frames = added.field->caretFrameCount();
  ASSERT_GT(initial_frames, 0);

  delay(20);
  refresh();
  EXPECT_EQ(initial_frames, added.field->caretFrameCount());

  delay(480);
  refresh();
  EXPECT_GT(added.field->caretFrameCount(), initial_frames);
  EXPECT_FALSE(added.task->textFieldEditor().isBlinkingCursorNowOn());
}

}  // namespace
}  // namespace roo_windows
