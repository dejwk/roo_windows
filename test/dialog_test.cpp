#include "roo_windows/dialogs/dialog.h"

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace {

class TestContent final : public BasicWidget {
 public:
  explicit TestContent(ApplicationContext& context) : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }
};

class TestParent final : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeLast;
};

class TestDialog final : public Dialog {
 public:
  explicit TestDialog(ApplicationContext& context)
      : Dialog(context, {"Cancel", "OK"}) {
    setTitle("Title");
  }

  ~TestDialog() override { prepareForDerivedDestruction(); }

  void chooseFirstAction() { actionTaken(0); }
  void setContent(WidgetRef content) {
    setPresentationContent(std::move(content));
  }

  int enter_count = 0;
  int exit_count = 0;
  int show_count = 0;
  int dismiss_count = 0;
  int exit_result = -2;
  bool enter_result = true;
  int* sequence = nullptr;
  int exit_order = 0;
  int dismiss_order = 0;

 protected:
  bool onEnter() override {
    ++enter_count;
    return enter_result;
  }
  void onExit() override {
    ++exit_count;
    if (sequence != nullptr) exit_order = ++*sequence;
  }
  void onShow() override { ++show_count; }
  void onDismiss(int result) override {
    ++dismiss_count;
    exit_result = result;
    if (sequence != nullptr) dismiss_order = ++*sequence;
  }
};

class MutatingDialog final : public Dialog {
 public:
  MutatingDialog(ApplicationContext& context, TestParent& new_parent)
      : Dialog(context, {"OK"}), new_parent_(new_parent) {
    setTitle("Mutation");
  }

  ~MutatingDialog() override { prepareForDerivedDestruction(); }

  int exit_count = 0;

 protected:
  bool onEnter() override {
    new_parent_.add(WidgetRef(*this), Rect(0, 0, 20, 20));
    return true;
  }

  void onExit() override { ++exit_count; }

 private:
  TestParent& new_parent_;
};

class DestructionContent final : public BasicWidget {
 public:
  DestructionContent(ApplicationContext& context, bool& detached_before_delete)
      : BasicWidget(context), detached_before_delete_(detached_before_delete) {}

  ~DestructionContent() override {
    detached_before_delete_ = parent() == nullptr;
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(12, 8);
  }

 private:
  bool& detached_before_delete_;
};

class DerivedDestructionDialog final : public Dialog {
 public:
  DerivedDestructionDialog(ApplicationContext& context,
                           bool& detached_before_delete)
      : Dialog(context, {"OK"}), content_(context, detached_before_delete) {
    setTitle("Lifetime");
  }

  ~DerivedDestructionDialog() override { prepareForDerivedDestruction(); }

 protected:
  bool onEnter() override {
    setPresentationContent(content_);
    return true;
  }

 private:
  DestructionContent content_;
};

class EnterContentDialog final : public Dialog {
 public:
  EnterContentDialog(ApplicationContext& context, int& measure_count)
      : Dialog(context, {}), content_(context, measure_count) {}

  ~EnterContentDialog() override { prepareForDerivedDestruction(); }

 private:
  class MeasuredContent final : public BasicWidget {
   public:
    MeasuredContent(ApplicationContext& context, int& measure_count)
        : BasicWidget(context), measure_count_(measure_count) {}

    Dimensions getSuggestedMinimumDimensions() const override {
      ++measure_count_;
      return Dimensions(32, 16);
    }

   private:
    int& measure_count_;
  };

  bool onEnter() override {
    setPresentationContent(content_);
    return true;
  }

  MeasuredContent content_;
};

class DialogTest : public ::testing::Test {
 protected:
  DialogTest()
      : device_(64, 64, raster_, roo_display::Argb4444()),
        display_(device_),
        environment_(scheduler_),
        app_(&environment_, display_) {
    task_content_ = std::make_unique<TestContent>(app_.context());
    owner_ = &app_.addTaskFullScreen(*task_content_);
  }

  roo::byte raster_[64 * 64 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment environment_;
  // Declared before Application so the borrowed task content outlives it.
  std::unique_ptr<TestContent> task_content_;
  Application app_;
  Task* owner_ = nullptr;
};

// Verifies dialog content is detached and the slot is clear before completion.
TEST_F(DialogTest, CloseDetachesContentBeforeCompletion) {
  TestDialog dialog(app_.context());
  TestContent content(app_.context());
  dialog.setContent(content);
  bool content_detached = false;
  bool slot_empty = false;

  EXPECT_EQ(
      PresentationStartResult::kStarted, dialog.show(*owner_, [&](int result) {
        EXPECT_EQ(-1, result);
        content_detached = content.parent() == nullptr;
        slot_empty =
            !app_.root().transient_presentation_slot().hasActivePresentation();
      }));
  EXPECT_NE(nullptr, content.parent());

  dialog.close();

  EXPECT_TRUE(content_detached);
  EXPECT_TRUE(slot_empty);
  EXPECT_EQ(nullptr, content.parent());
  EXPECT_EQ(1, dialog.dismiss_count);
  EXPECT_EQ(-1, dialog.exit_result);
}

// Content attached by onEnter participates in the dialog's initial measure.
TEST_F(DialogTest, OnEnterContentIsMeasuredBeforeDialogIsAttached) {
  int measure_count = 0;
  EnterContentDialog dialog(app_.context(), measure_count);

  EXPECT_EQ(PresentationStartResult::kStarted, dialog.show(*owner_, nullptr));
  EXPECT_GT(measure_count, 0);

  dialog.close();
}

// Verifies dialog actions finish the registered presentation exactly once.
TEST_F(DialogTest, ActionCompletesAndReleasesTheSlot) {
  TestDialog dialog(app_.context());
  TestContent content(app_.context());
  dialog.setContent(content);

  EXPECT_EQ(PresentationStartResult::kStarted, dialog.show(*owner_, nullptr));
  dialog.chooseFirstAction();

  EXPECT_EQ(1, dialog.dismiss_count);
  EXPECT_EQ(0, dialog.exit_result);
  EXPECT_FALSE(
      app_.root().transient_presentation_slot().hasActivePresentation());
}

// Verifies the explicit owner supplies the active focus boundary and eligible
// Back dismisses through the common host before restoring its base scope.
TEST_F(DialogTest, ExplicitOwnerFocusAndBackUseTheCommonHost) {
  TestDialog dialog(app_.context());
  TestContent content(app_.context());
  dialog.setContent(content);
  Widget* base_scope = owner_->focus().scopeRoot();

  ASSERT_EQ(PresentationStartResult::kStarted, dialog.show(*owner_, nullptr));
  EXPECT_EQ(&dialog, owner_->focus().scopeRoot());
  EXPECT_EQ(BackResult::kHandled, owner_->requestBack(BackSource::kBackKey));
  EXPECT_EQ(base_scope, owner_->focus().scopeRoot());
  EXPECT_EQ(1, dialog.exit_count);
  EXPECT_EQ(1, dialog.dismiss_count);
  EXPECT_EQ(-1, dialog.exit_result);
}

// Verifies a busy dialog host leaves a second presenter untouched.
TEST_F(DialogTest, ShowRejectsOccupiedTransientSlot) {
  TestDialog first(app_.context());
  TestDialog second(app_.context());
  TestContent content(app_.context());
  first.setContent(content);

  EXPECT_EQ(PresentationStartResult::kStarted, first.show(*owner_, nullptr));
  EXPECT_EQ(PresentationStartResult::kHostBusy, second.show(*owner_, nullptr));
  first.close();
  EXPECT_EQ(1, first.dismiss_count);
  EXPECT_EQ(0, second.enter_count);
  EXPECT_EQ(0, second.exit_count);
}

// Verifies dialog completion may immediately open the next root presentation.
TEST_F(DialogTest, CompletionCanShowAnotherDialog) {
  TestDialog first(app_.context());
  TestDialog second(app_.context());
  TestContent first_content(app_.context());
  TestContent second_content(app_.context());
  first.setContent(first_content);
  second.setContent(second_content);
  PresentationStartResult second_result = PresentationStartResult::kHostBusy;

  EXPECT_EQ(PresentationStartResult::kStarted,
            first.show(*owner_, [&](int result) {
              second_result = second.show(*owner_, nullptr);
            }));
  first.close();

  EXPECT_EQ(PresentationStartResult::kStarted, second_result);
  EXPECT_TRUE(
      app_.root().transient_presentation_slot().hasActivePresentation());
  second.close();
  EXPECT_EQ(1, second.dismiss_count);
}

// Verifies destroying a visible caller-owned dialog detaches it without
// callback.
TEST_F(DialogTest, DestructionCancelsWithoutCompletion) {
  int callback_count = 0;
  TestDialog* dialog = new TestDialog(app_.context());
  TestContent content(app_.context());
  dialog->setContent(content);
  EXPECT_EQ(PresentationStartResult::kStarted,
            dialog->show(*owner_, [&](int result) { ++callback_count; }));

  delete dialog;

  EXPECT_EQ(0, callback_count);
  EXPECT_FALSE(
      app_.root().transient_presentation_slot().hasActivePresentation());
}

// Verifies failed preparation is balanced exactly once and never becomes a
// shown or dismissed presentation.
TEST_F(DialogTest, FailedPreparationRunsOneBalancedExit) {
  TestDialog dialog(app_.context());
  dialog.enter_result = false;

  EXPECT_EQ(PresentationStartResult::kSurfaceUnavailable,
            dialog.show(*owner_, nullptr));
  EXPECT_EQ(1, dialog.enter_count);
  EXPECT_EQ(1, dialog.exit_count);
  EXPECT_EQ(0, dialog.show_count);
  EXPECT_EQ(0, dialog.dismiss_count);
  EXPECT_FALSE(
      app_.root().transient_presentation_slot().hasActivePresentation());
}

// Verifies mutation during preparation is rejected by repeated preflight and
// the created session receives exactly one balanced deletion.
TEST_F(DialogTest, PreparedRootMutationFailsRepeatedPreflight) {
  TestParent new_parent(app_.context());
  MutatingDialog dialog(app_.context(), new_parent);

  EXPECT_EQ(PresentationStartResult::kSurfaceUnavailable,
            dialog.show(*owner_, nullptr));
  EXPECT_EQ(1, dialog.exit_count);
  EXPECT_EQ(&new_parent, dialog.parent());
  EXPECT_FALSE(
      app_.root().transient_presentation_slot().hasActivePresentation());
  new_parent.removeLast();
}

// Verifies session cleanup, visible dismissal, and application completion run
// in that order after the host structure and canonical slot are clear.
TEST_F(DialogTest, LifecycleCleanupPrecedesApplicationCompletion) {
  TestDialog dialog(app_.context());
  TestContent content(app_.context());
  dialog.setContent(content);
  int sequence = 0;
  int completion_order = 0;
  dialog.sequence = &sequence;
  ASSERT_EQ(PresentationStartResult::kStarted, dialog.show(*owner_, [&](int) {
    completion_order = ++sequence;
    EXPECT_EQ(nullptr, dialog.parent());
    EXPECT_EQ(nullptr, content.parent());
    EXPECT_FALSE(
        app_.root().transient_presentation_slot().hasActivePresentation());
  }));

  dialog.close();
  EXPECT_EQ(1, dialog.exit_order);
  EXPECT_EQ(2, dialog.dismiss_order);
  EXPECT_EQ(3, completion_order);
}

// Verifies the protected derived-destruction seam detaches inline borrowed
// content before the derived member's destructor runs and suppresses callback.
TEST_F(DialogTest, DerivedDestructionSeamProtectsInlineContent) {
  bool detached_before_delete = false;
  int callback_count = 0;
  auto* dialog =
      new DerivedDestructionDialog(app_.context(), detached_before_delete);
  ASSERT_EQ(PresentationStartResult::kStarted,
            dialog->show(*owner_, [&](int) { ++callback_count; }));

  delete dialog;
  EXPECT_TRUE(detached_before_delete);
  EXPECT_EQ(0, callback_count);
  EXPECT_FALSE(
      app_.root().transient_presentation_slot().hasActivePresentation());
}

}  // namespace
}  // namespace roo_windows
