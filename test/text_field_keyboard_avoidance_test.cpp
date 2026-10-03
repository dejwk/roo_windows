#include "gtest/gtest.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/keyboard/editor_destination.h"
#include "roo_windows/material3/text_field/secure_text_field.h"
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
  TextField first(context(), "First");
  TextField second(context(), "Second");
  TextField third(context(), "Third");
  TextField last(context(), "Last");
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

// Verifies an outside tap is consumed, ends editing without losing text,
// and restores the viewport. Field and keyboard taps keep their own targets.
TEST_F(KeyboardAvoidanceTest, OutsideTapDismissesKeyboard) {
  TextField field(context(), "Value");
  TextField other(context(), "Other");
  FullWidthColumn form(context());
  form.add(field);
  form.add(other);
  SimpleScrollablePanel scroller(context(), form);
  Task& task = app_.addTaskFullScreen(scroller);
  int back_requests = 0;
  task.setBackCallback([&back_requests](BackSource) {
    ++back_requests;
    return BackResult::kHandled;
  });
  ASSERT_TRUE(refresh());
  field.setText("retained");
  field.edit();
  ASSERT_TRUE(refresh());
  MainWindow& root = *field.getMainWindow();
  std::vector<Widget*> path;
  Rect bounds = ScreenBounds(field);
  ASSERT_TRUE(
      root.fillTouchTargetPath(bounds.xMin() + 1, bounds.yMin() + 1, path));
  EXPECT_EQ(&field, path.back());
  path.clear();
  bounds = ScreenBounds(app_.keyboard().getContents());
  ASSERT_TRUE(
      root.fillTouchTargetPath(bounds.xMin() + 1, bounds.yMin() + 1, path));
  EXPECT_EQ(&app_.keyboard().getContents(), path.back());
  path.clear();
  bounds = ScreenBounds(other);
  ASSERT_TRUE(
      root.fillTouchTargetPath(bounds.xMin() + 1, bounds.yMin() + 1, path));
  ASSERT_EQ(1u, path.size());
  EXPECT_EQ(&root, path.back());
  EXPECT_TRUE(root.supportsTap());
  EXPECT_TRUE(field.isEdited());
  root.onSingleTapUp(bounds.xMin() + 1, bounds.yMin() + 1);
  ASSERT_TRUE(refresh());
  EXPECT_FALSE(field.isEdited());
  EXPECT_FALSE(other.isEdited());
  EXPECT_FALSE(app_.keyboard().getContents().isVisible());
  EXPECT_EQ("retained", field.text());
  EXPECT_EQ(320, scroller.height());
  EXPECT_FALSE(root.supportsTap());
  EXPECT_EQ(0, back_requests);
  task.setBackCallback(nullptr);
  task.navigation().clear();
}

// Verifies blank space dismisses on release, while a canceled gesture leaves
// the editor open and does not change the viewport.
TEST_F(KeyboardAvoidanceTest, BlankSpaceDismissesOnlyOnCompletedTap) {
  TextField field(context(), "Value");
  FullWidthColumn form(context());
  form.add(field);
  Task& task = app_.addTaskFullScreen(form);
  ASSERT_TRUE(refresh());
  field.edit();
  ASSERT_TRUE(refresh());
  std::vector<Widget*> path;
  ASSERT_TRUE(app_.root().fillTouchTargetPath(230, 150, path));
  ASSERT_EQ(1u, path.size());
  Widget* target = path.back();
  ASSERT_TRUE(target->supportsTap());
  target->onDown(230, 150);
  target->onCancel();
  EXPECT_TRUE(field.isEdited());
  EXPECT_TRUE(app_.keyboard().getContents().isVisible());
  target->onDown(230, 150);
  target->onSingleTapUp(230, 150);
  EXPECT_FALSE(field.isEdited());
  EXPECT_FALSE(app_.keyboard().getContents().isVisible());
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

/// Exposes fixed child placement for extraction geometry and lifetime tests.
class ExtractionForm : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeAll;
};

/// Records completion results from the original field after extraction.
class CompletionField : public TextField {
 public:
  using TextField::TextField;
  int completions = 0;
  bool confirmed = false;

 protected:
  void onEditFinished(bool result) override {
    ++completions;
    confirmed = result;
  }
};

// Verifies a tall field opens one compact destination, commits only on Enter,
// and returns to the original form with one completion notification.
TEST_F(KeyboardAvoidanceTest, ExtractsTallFieldAndConfirms) {
  CompletionField field(context(), "Value");
  ExtractionForm form(context());
  form.add(field, Rect(0, 0, 239, 199));
  Task& task = app_.addTaskFullScreen(form);
  ASSERT_TRUE(refresh());
  field.setText("old");
  field.edit();
  ASSERT_EQ(2u, task.navigation().depth());
  ASSERT_TRUE(refresh());
  Widget* editor = task.textFieldEditor().editedWidget();
  ASSERT_NE(nullptr, editor);
  EXPECT_NE(&field, editor);
  EXPECT_GE(ScreenBounds(*editor).yMin(), 0);
  EXPECT_LT(ScreenBounds(*editor).yMax(), 160);
  task.textFieldEditor().setSelection(0, 3);
  task.textFieldEditor().rune(U'X');
  EXPECT_EQ("old", field.text());
  task.textFieldEditor().enter();
  EXPECT_EQ(1u, task.navigation().depth());
  EXPECT_EQ("X", field.text());
  EXPECT_EQ(1, field.completions);
  EXPECT_TRUE(field.confirmed);
  EXPECT_FALSE(app_.keyboard().getContents().isVisible());
  ASSERT_TRUE(refresh());
  EXPECT_EQ(&task, field.getTask());
  task.navigation().clear();
}

// Verifies Back discards the draft, while subsequent extraction can reuse the
// shared destination and navigation clear releases it safely.
TEST_F(KeyboardAvoidanceTest, ExtractedBackCancelsAndDestinationIsReusable) {
  CompletionField field(context(), "Value");
  ExtractionForm form(context());
  form.add(field, Rect(0, 0, 239, 199));
  Task& task = app_.addTaskFullScreen(form);
  ASSERT_TRUE(refresh());
  field.setText("old");
  field.edit();
  ASSERT_TRUE(refresh());
  task.textFieldEditor().rune(U'X');
  EXPECT_EQ(BackResult::kHandled, task.requestBack());
  EXPECT_EQ("old", field.text());
  EXPECT_EQ(1, field.completions);
  EXPECT_FALSE(field.confirmed);
  ASSERT_TRUE(refresh());
  field.edit();
  EXPECT_EQ(2u, task.navigation().depth());
  task.navigation().clear();
  EXPECT_EQ(2, field.completions);
  EXPECT_EQ(nullptr, context().editorDestination().getNavigationHost());
}

// Verifies extraction retains password masking in the compact editor.
TEST_F(KeyboardAvoidanceTest, ExtractedSecureFieldRemainsMasked) {
  SecureTextField field(context(), "Password");
  ExtractionForm form(context());
  form.add(field, Rect(0, 0, 239, 199));
  Task& task = app_.addTaskFullScreen(form);
  ASSERT_TRUE(refresh());
  field.setText("secret");
  field.edit();
  ASSERT_TRUE(refresh());
  ASSERT_EQ(2u, task.navigation().depth());
  Widget* editor = task.textFieldEditor().editedWidget();
  ASSERT_NE(nullptr, editor);
  EXPECT_TRUE(static_cast<roo_windows::TextField*>(editor)->obscureText());
  task.textFieldEditor().enter();
  EXPECT_EQ("secret", field.text());
  task.navigation().clear();
}

// Verifies destroying the detached source does not leave a dangling callback.
TEST_F(KeyboardAvoidanceTest, SourceCanBeDestroyedWhileExtracted) {
  auto field = std::make_unique<TextField>(context(), "Value");
  ExtractionForm form(context());
  form.add(*field, Rect(0, 0, 239, 199));
  Task& task = app_.addTaskFullScreen(form);
  ASSERT_TRUE(refresh());
  field->edit();
  ASSERT_EQ(2u, task.navigation().depth());
  ASSERT_TRUE(refresh());
  form.removeAll();
  field.reset();
  task.textFieldEditor().enter();
  EXPECT_EQ(1u, task.navigation().depth());
  task.navigation().clear();
}

// Verifies exact-fit fields and physical-key activation stay in the form.
TEST_F(KeyboardAvoidanceTest, FitsExactlyAndPhysicalKeyboardDoesNotExtract) {
  TextField field(context(), "Value");
  ExtractionForm form(context());
  form.add(field, Rect(0, 0, 239, 159));
  Task& task = app_.addTaskFullScreen(form);
  ASSERT_TRUE(refresh());
  field.edit();
  EXPECT_EQ(1u, task.navigation().depth());
  task.textFieldEditor().cancel();
  form.removeAll();
  form.add(field, Rect(0, 0, 239, 199));
  ASSERT_TRUE(refresh());
  field.onKeyEvent(KeyEvent(KeyPhase::kDown, KeyCode::kEnter, 0, 0));
  EXPECT_EQ(1u, task.navigation().depth());
  EXPECT_TRUE(field.isEdited());
  EXPECT_FALSE(app_.keyboard().getContents().isVisible());
  task.navigation().clear();
}

// Verifies the renamed public destination sets its hint and invokes the
// callback once, after removal, and can be reused with a different value.
TEST_F(KeyboardAvoidanceTest, ExplicitEditorDestinationRoundTrip) {
  ExtractionForm form(context());
  Task& task = app_.addTaskFullScreen(form);
  EditorDestination editor(context());
  int completions = 0;
  std::string result;
  editor.triggerEdit(task.navigation(), "before", "New hint",
                     [&](const std::string& value) {
                       EXPECT_EQ(nullptr, editor.getNavigationHost());
                       ++completions;
                       result = value;
                     });
  ASSERT_TRUE(refresh());
  Widget* target = task.textFieldEditor().editedWidget();
  ASSERT_NE(nullptr, target);
  EXPECT_EQ("New hint", static_cast<roo_windows::TextField*>(target)->hint());
  task.textFieldEditor().enter();
  EXPECT_EQ(1, completions);
  EXPECT_EQ("before", result);
  editor.triggerEdit(task.navigation(), "discard", "Another", nullptr);
  task.requestBack();
  EXPECT_EQ(1, completions);
  task.navigation().clear();
}

// Verifies an oversized field in a scrolling form is extracted as well;
// scrolling alone cannot make an intrinsically tall field fully visible.
TEST_F(KeyboardAvoidanceTest, ExtractsTallScrollableField) {
  class TallField : public TextField {
   public:
    using TextField::TextField;

    /// Forces a field taller than the available software-keyboard viewport.
    PreferredSize getPreferredSize() const override {
      return PreferredSize(PreferredSize::MatchParentWidth(),
                           PreferredSize::ExactHeight(200));
    }
  };

  TallField field(context(), "Tall");
  FullWidthColumn form(context());
  form.add(field);
  SimpleScrollablePanel scroller(context(), form);
  Task& task = app_.addTaskFullScreen(scroller);
  ASSERT_TRUE(refresh());
  field.edit();
  EXPECT_EQ(2u, task.navigation().depth());
  task.requestBack();
  ASSERT_TRUE(refresh());
  EXPECT_EQ(320, scroller.height());
  task.navigation().clear();
}

// Verifies a commit callback may delete the source without receiving a later
// edit-finished callback through the stale source pointer.
TEST_F(KeyboardAvoidanceTest, ValueChangeMayDestroySource) {
  class DestructiveField : public TextField {
   public:
    using TextField::TextField;
    std::function<void()> changed;

   protected:
    void onTextChanged() override {
      std::function<void()> callback = changed;
      if (callback != nullptr) callback();
    }
  };

  auto field = std::make_unique<DestructiveField>(context(), "Value");
  ExtractionForm form(context());
  form.add(*field, Rect(0, 0, 239, 199));
  Task& task = app_.addTaskFullScreen(form);
  ASSERT_TRUE(refresh());
  field->changed = [&]() {
    form.removeAll();
    field.reset();
  };
  field->edit();
  ASSERT_EQ(2u, task.navigation().depth());
  ASSERT_TRUE(refresh());
  task.textFieldEditor().rune(U'X');
  task.textFieldEditor().enter();
  EXPECT_EQ(nullptr, field);
  EXPECT_EQ(1u, task.navigation().depth());
  task.navigation().clear();
}

// Verifies a field wider than the task also uses the compact editor.
TEST_F(KeyboardAvoidanceTest, ExtractsFieldWiderThanTask) {
  TextField field(context(), "Value");
  ExtractionForm form(context());
  form.add(field, Rect(0, 0, 299, 99));
  Task& task = app_.addTaskFullScreen(form);
  ASSERT_TRUE(refresh());
  field.edit();
  EXPECT_EQ(2u, task.navigation().depth());
  task.navigation().clear();
}

// Verifies application teardown removes the borrowed editor destination and
// cancels completion before widgets that outlive the context are destroyed.
TEST_F(KeyboardAvoidanceTest, ApplicationTeardownCancelsExtractedEditor) {
  auto owner = std::make_unique<Application>(&env_, display_);
  CompletionField field(owner->context(), "Value");
  ExtractionForm form(owner->context());
  form.add(field, Rect(0, 0, 239, 199));
  Task& task = owner->addTaskFullScreen(form);
  ASSERT_TRUE(owner->refresh());
  field.edit();
  ASSERT_EQ(2u, task.navigation().depth());
  ASSERT_TRUE(owner->refresh());
  task.textFieldEditor().rune(U'X');
  owner.reset();
  EXPECT_EQ(1, field.completions);
  EXPECT_FALSE(field.confirmed);
  EXPECT_EQ("", field.text());
}
}  // namespace roo_windows::material3
