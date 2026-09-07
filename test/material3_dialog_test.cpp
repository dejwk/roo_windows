#include <memory>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/text_input.h"
#include "roo_windows/material3/dialog/basic_dialog.h"
#include "roo_windows/material3/dialog/dialog_scaffold.h"
#include "roo_windows/material3/dialog/full_screen_dialog.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_windows::material3 {

namespace test {

class DialogTestAccess {
 public:
  static Widget& CloseButton(FullScreenDialog& dialog) { return dialog.close_; }
  static Widget& ConfirmButton(FullScreenDialog& dialog) {
    return dialog.confirm_;
  }
};

}  // namespace test

namespace {

class TestContent final : public BasicWidget {
 public:
  explicit TestContent(ApplicationContext& context,
                       int* destruction_count = nullptr)
      : BasicWidget(context), destruction_count_(destruction_count) {}

  ~TestContent() override {
    if (destruction_count_ != nullptr) ++*destruction_count_;
  }

  bool isFocusable() const override { return true; }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(12, 8);
  }

 private:
  int* destruction_count_;
};

class TestPanel final : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeLast;
};

class TestActionDelegate final : public internal::DialogActionDelegate {
 public:
  void invokeDialogAction(uint8_t id, DialogActionRole role) override {
    last_id = id;
    last_role = role;
    ++count;
  }

  uint8_t last_id = 0;
  DialogActionRole last_role = DialogActionRole::kAcknowledge;
  int count = 0;
};

class TestScaffold final : public internal::DialogScaffoldBase {
 public:
  TestScaffold(ApplicationContext& context, WidgetRef body,
               internal::DialogScaffoldVariant variant =
                   internal::DialogScaffoldVariant::kBasic)
      : DialogScaffoldBase(context, std::move(body), variant) {}

  ~TestScaffold() override { prepareForDerivedDestruction(); }

  DialogShowResult show(Task& owner) {
    return showDialogSurface(owner, Rect(8, 8, 47, 39),
                             TransientBarrierPaint::kScrim);
  }

  void dismiss() { finishDialog(PresentationFinishReason::kCancel); }

  void setBody(WidgetRef body) { setDialogBody(std::move(body)); }

  Widget* body() { return dialogBody(); }

  int finish_count = 0;
  PresentationFinishReason last_reason = PresentationFinishReason::kAction;

 protected:
  void onDialogPresentationFinished(PresentationFinishReason reason) override {
    ++finish_count;
    last_reason = reason;
  }
};

class TestBasicDialog final : public BasicDialog {
 public:
  TestBasicDialog(ApplicationContext& context, WidgetRef body,
                  const DialogActionSpec* actions, uint8_t action_count)
      : BasicDialog(context, std::move(body), actions, action_count) {}

  const std::string& headline() { return dialogTitle().text(); }

  int action_count = 0;
  int dismiss_count = 0;
  uint8_t last_action_id = 0;
  DialogActionRole last_action_role = DialogActionRole::kAcknowledge;
  DialogDismissReason last_dismiss_reason = DialogDismissReason::kProgrammatic;
  bool detached_during_callback = false;

 protected:
  void onActionInvoked(uint8_t action_id, DialogActionRole role) override {
    ++action_count;
    last_action_id = action_id;
    last_action_role = role;
    detached_during_callback = !isShowing() && parent() == nullptr;
  }

  void onDismissed(DialogDismissReason reason) override {
    ++dismiss_count;
    last_dismiss_reason = reason;
    detached_during_callback = !isShowing() && parent() == nullptr;
  }
};

class InlineBodyDialog final : public BasicDialog {
 public:
  InlineBodyDialog(ApplicationContext& context, bool& detached_before_delete,
                   const DialogActionSpec* action)
      : BasicDialog(context, WidgetRef(), action, 1),
        body_(context, detached_before_delete) {
    setBody(WidgetRef(body_));
  }

  ~InlineBodyDialog() override { prepareForDerivedDestruction(); }

 private:
  class InlineBody final : public BasicWidget {
   public:
    InlineBody(ApplicationContext& context, bool& detached_before_delete)
        : BasicWidget(context),
          detached_before_delete_(detached_before_delete) {}

    ~InlineBody() override { detached_before_delete_ = parent() == nullptr; }

    Dimensions getSuggestedMinimumDimensions() const override {
      return Dimensions(10, 10);
    }

   private:
    bool& detached_before_delete_;
  };

  InlineBody body_;
};

class TestFullScreenDialog final : public FullScreenDialog {
 public:
  TestFullScreenDialog(ApplicationContext& context, WidgetRef body)
      : FullScreenDialog(context, std::move(body)) {}

  const std::string& headerTitle() { return dialogTitle().text(); }

  bool allow_dismiss = true;
  bool allow_confirm = true;
  int dismiss_request_count = 0;
  int confirm_request_count = 0;
  int dismiss_count = 0;
  int confirmed_count = 0;
  uint8_t last_confirm_id = 0;
  DialogDismissReason last_request_reason = DialogDismissReason::kProgrammatic;
  DialogDismissReason last_dismiss_reason = DialogDismissReason::kProgrammatic;
  bool detached_during_callback = false;

 protected:
  bool onDismissRequested(DialogDismissReason reason) override {
    ++dismiss_request_count;
    last_request_reason = reason;
    return allow_dismiss;
  }

  bool onConfirmRequested(uint8_t action_id) override {
    ++confirm_request_count;
    last_confirm_id = action_id;
    return allow_confirm;
  }

  void onDismissed(DialogDismissReason reason) override {
    ++dismiss_count;
    last_dismiss_reason = reason;
    detached_during_callback = !isShowing() && parent() == nullptr;
  }

  void onConfirmed(uint8_t action_id) override {
    ++confirmed_count;
    last_confirm_id = action_id;
    detached_during_callback = !isShowing() && parent() == nullptr;
  }
};

class Material3DialogTest : public ::testing::Test {
 protected:
  Material3DialogTest()
      : device_(320, 240, raster_, roo_display::Argb4444()),
        display_(device_),
        environment_(scheduler_),
        app_(&environment_, display_),
        task_content_(app_.context()),
        owner_(app_.addTaskFullScreen(task_content_)) {}

  roo::byte raster_[320 * 240 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment environment_;
  Application app_;
  TestPanel task_content_;
  Task& owner_;
};

TEST_F(Material3DialogTest, ActionStripValidatesAndCopiesFixedDescriptors) {
  TestActionDelegate delegate;
  DialogActionSpec actions[] = {
      {7, "Cancel", DialogActionRole::kDismiss},
      {9, "Save", DialogActionRole::kConfirm, false},
  };
  internal::DialogActionStrip strip(app_.context(), delegate, actions, 2);
  actions[0].id = 42;

  EXPECT_EQ(2, strip.actionCount());
  EXPECT_EQ(7, strip.action(0).id);
  EXPECT_EQ(DialogActionRole::kConfirm, strip.action(1).role);
  EXPECT_FALSE(strip.action(1).enabled);
  strip.setActionEnabled(9, true);
  EXPECT_TRUE(strip.action(1).enabled);
}

TEST_F(Material3DialogTest, ActionStripRejectsInvalidRoleModels) {
  TestActionDelegate delegate;
  DialogActionSpec confirm[] = {
      {1, "Save", DialogActionRole::kConfirm},
  };
  EXPECT_DEATH_IF_SUPPORTED(
      internal::DialogActionStrip(app_.context(), delegate, confirm, 1), "");

  DialogActionSpec duplicate[] = {
      {1, "Cancel", DialogActionRole::kDismiss},
      {1, "Save", DialogActionRole::kConfirm},
  };
  EXPECT_DEATH_IF_SUPPORTED(
      internal::DialogActionStrip(app_.context(), delegate, duplicate, 2), "");
}

TEST_F(Material3DialogTest, ActionStripUsesLogicalHorizontalOrder) {
  TestActionDelegate delegate;
  DialogActionSpec actions[] = {
      {1, "Cancel", DialogActionRole::kDismiss},
      {2, "Save", DialogActionRole::kConfirm},
  };
  internal::DialogActionStrip strip(app_.context(), delegate, actions, 2);
  strip.measure(WidthSpec::Exactly(240), HeightSpec::AtMost(80));
  strip.layout(Rect(0, 0, 239, 79));
  EXPECT_EQ(strip.actionButton(0).getNaturalDimensions().width(),
            strip.actionButton(0).width());
  EXPECT_EQ(strip.actionButton(0).getNaturalDimensions().height(),
            strip.actionButton(0).height());
  EXPECT_EQ(strip.actionButton(1).getNaturalDimensions().width(),
            strip.actionButton(1).width());
  EXPECT_EQ(strip.actionButton(1).getNaturalDimensions().height(),
            strip.actionButton(1).height());
  EXPECT_LT(strip.actionButton(0).offsetLeft(),
            strip.actionButton(1).offsetLeft());

  strip.setLayoutDirection(LayoutDirection::kRightToLeft);
  strip.measure(WidthSpec::Exactly(240), HeightSpec::AtMost(80));
  strip.layout(Rect(0, 0, 239, 79));
  EXPECT_GT(strip.actionButton(0).offsetLeft(),
            strip.actionButton(1).offsetLeft());
}

TEST_F(Material3DialogTest, ActionStripStacksConfirmAboveDismiss) {
  TestActionDelegate delegate;
  DialogActionSpec actions[] = {
      {1, "Keep editing", DialogActionRole::kDismiss},
      {2, "Discard changes", DialogActionRole::kConfirm},
  };
  internal::DialogActionStrip strip(app_.context(), delegate, actions, 2);
  Dimensions measured =
      strip.measure(WidthSpec::Exactly(80), HeightSpec::AtMost(160));
  strip.layout(Rect(0, 0, 79, measured.height() - 1));

  EXPECT_TRUE(strip.isStacked());
  EXPECT_LT(strip.actionButton(1).offsetTop(),
            strip.actionButton(0).offsetTop());
}

TEST_F(Material3DialogTest, SharedHostRejectsBusyAndAlreadyPresentedDialogs) {
  TestContent first_body(app_.context());
  TestContent second_body(app_.context());
  TestScaffold first(app_.context(), WidgetRef(first_body));
  TestScaffold second(app_.context(), WidgetRef(second_body));

  EXPECT_EQ(DialogShowResult::kShown, first.show(owner_));
  EXPECT_EQ(DialogShowResult::kAlreadyPresented, first.show(owner_));
  EXPECT_EQ(DialogShowResult::kHostBusy, second.show(owner_));
  first.dismiss();
  EXPECT_EQ(DialogShowResult::kShown, second.show(owner_));
  second.dismiss();
}

TEST_F(Material3DialogTest, BorrowedAndAdoptedBodiesPersistUntilReplacement) {
  TestContent borrowed(app_.context());
  int destruction_count = 0;
  {
    TestScaffold dialog(app_.context(),
                        WidgetRef(std::make_unique<TestContent>(
                            app_.context(), &destruction_count)));
    Widget* adopted = dialog.body();
    ASSERT_NE(nullptr, adopted->parent());
    ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
    dialog.dismiss();
    EXPECT_NE(nullptr, adopted->parent());
    EXPECT_EQ(0, destruction_count);

    dialog.setBody(WidgetRef(borrowed));
    EXPECT_EQ(1, destruction_count);
    EXPECT_EQ(&borrowed, dialog.body());
    EXPECT_NE(nullptr, borrowed.parent());
  }
  EXPECT_EQ(nullptr, borrowed.parent());
}

TEST_F(Material3DialogTest, DialogFocusStartsEmptyAndTabEntersOnEveryShow) {
  TestContent first(app_.context());
  TestContent second(app_.context());
  TestScaffold dialog(app_.context(), WidgetRef(first));

  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  EXPECT_EQ(nullptr, owner_.focus().focused());
  EXPECT_TRUE(owner_.focus().moveFocus(dialog, false));
  EXPECT_EQ(&first, owner_.focus().focused());
  dialog.dismiss();
  dialog.setBody(WidgetRef(second));
  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  EXPECT_EQ(nullptr, owner_.focus().focused());
  EXPECT_TRUE(owner_.focus().moveFocus(dialog, false));
  EXPECT_EQ(&second, owner_.focus().focused());
  dialog.dismiss();
}

TEST_F(Material3DialogTest, SemanticTextInputIsIsolatedToDialogBody) {
  TextField owner_field(app_.context(), font_body1(), "", roo_display::kLeft,
                        TextField::NONE);
  task_content_.add(WidgetRef(owner_field), Rect(0, 0, 30, 12));
  TextField dialog_field(app_.context(), font_body1(), "", roo_display::kLeft,
                         TextField::NONE);
  TestScaffold dialog(app_.context(), WidgetRef(dialog_field));
  ASSERT_TRUE(app_.refresh());
  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  TextInputEmitter emitter;
  emitter.connect(app_);

  owner_field.edit();
  EXPECT_FALSE(emitter.commitRune(U'B'));
  dialog_field.edit();
  EXPECT_TRUE(emitter.commitRune(U'D'));
  EXPECT_EQ("", owner_field.content());
  EXPECT_EQ("D", dialog_field.content());

  dialog.dismiss();
  task_content_.removeLast();
}

TEST_F(Material3DialogTest, PresenterDestructionCancelsWithoutCompletion) {
  TestContent body(app_.context());
  auto dialog = std::make_unique<TestScaffold>(app_.context(), WidgetRef(body));
  ASSERT_EQ(DialogShowResult::kShown, dialog->show(owner_));
  dialog.reset();

  EXPECT_EQ(nullptr, body.parent());
  EXPECT_FALSE(
      app_.root().transient_presentation_slot().hasActivePresentation());
}

TEST_F(Material3DialogTest, BasicDialogCentersAndClampsWidth) {
  TestContent body(app_.context());
  DialogActionSpec action{1, "OK", DialogActionRole::kAcknowledge};
  TestBasicDialog dialog(app_.context(), WidgetRef(body), &action, 1);

  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  EXPECT_EQ(272, dialog.width());
  EXPECT_EQ((320 - dialog.width()) / 2, dialog.offsetLeft());
  EXPECT_EQ(Scaled(28), dialog.getBorderStyle().top_left_corner_radius());
  dialog.dismiss();
}

TEST_F(Material3DialogTest, BasicActionClosesBeforeTypedCompletion) {
  DialogActionSpec action{17, "OK", DialogActionRole::kAcknowledge};
  TestBasicDialog dialog(app_.context(), WidgetRef(), &action, 1);
  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  ASSERT_EQ(nullptr, owner_.focus().focused());
  ASSERT_TRUE(owner_.focus().moveFocus(dialog, false));
  Widget* focused = owner_.focus().focused();
  ASSERT_NE(nullptr, focused);

  focused->onClicked();

  EXPECT_EQ(1, dialog.action_count);
  EXPECT_EQ(17, dialog.last_action_id);
  EXPECT_EQ(DialogActionRole::kAcknowledge, dialog.last_action_role);
  EXPECT_TRUE(dialog.detached_during_callback);
}

TEST_F(Material3DialogTest, BasicDialogMapsBackEscapeAndProgrammaticDismissal) {
  DialogActionSpec action{1, "OK", DialogActionRole::kAcknowledge};
  TestBasicDialog dialog(app_.context(), WidgetRef(), &action, 1);
  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  EXPECT_EQ(BackResult::kHandled, owner_.requestBack(BackSource::kEscapeKey));
  EXPECT_EQ(DialogDismissReason::kEscape, dialog.last_dismiss_reason);
  EXPECT_TRUE(dialog.detached_during_callback);

  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  dialog.dismiss();
  EXPECT_EQ(2, dialog.dismiss_count);
  EXPECT_EQ(DialogDismissReason::kProgrammatic, dialog.last_dismiss_reason);
}

TEST_F(Material3DialogTest, BasicDialogRestoresOwnerFocusAfterDismissal) {
  TestContent owner_focus(app_.context());
  task_content_.add(WidgetRef(owner_focus), Rect(0, 0, 20, 20));
  ASSERT_TRUE(app_.refresh());
  ASSERT_TRUE(owner_focus.requestFocus());
  TestContent body(app_.context());
  DialogActionSpec action{1, "OK", DialogActionRole::kAcknowledge};
  TestBasicDialog dialog(app_.context(), WidgetRef(body), &action, 1);

  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  EXPECT_EQ(nullptr, owner_.focus().focused());
  ASSERT_TRUE(owner_.focus().moveFocus(dialog, false));
  EXPECT_EQ(&body, owner_.focus().focused());
  dialog.dismiss();
  EXPECT_EQ(&owner_focus, owner_.focus().focused());
  task_content_.removeLast();
}

TEST_F(Material3DialogTest, BasicDialogOwnsHeadlineSourceText) {
  DialogActionSpec action{1, "OK", DialogActionRole::kAcknowledge};
  TestBasicDialog dialog(app_.context(), WidgetRef(), &action, 1);
  {
    std::string headline = "Call-local owned headline";
    dialog.setHeadline(std::move(headline));
  }
  EXPECT_EQ("Call-local owned headline", dialog.headline());
}

TEST_F(Material3DialogTest, BasicDialogRejectsDisablingMandatoryAction) {
  DialogActionSpec action{1, "OK", DialogActionRole::kAcknowledge};
  TestBasicDialog dialog(app_.context(), WidgetRef(), &action, 1);
  EXPECT_DEATH_IF_SUPPORTED(dialog.setActionEnabled(1, false), "");
  EXPECT_DEATH_IF_SUPPORTED(dialog.setActionEnabled(99, true), "");
}

TEST_F(Material3DialogTest, AlertDialogRetainsOwnedSupportingTextAcrossShow) {
  DialogActionSpec action{1, "OK", DialogActionRole::kAcknowledge};
  AlertDialog dialog(app_.context(), std::string("Alert headline"),
                     std::string("Supporting prose"), &action, 1);
  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  dialog.dismiss();
  dialog.setSupportingText(std::string("Updated supporting prose"));
  EXPECT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  dialog.dismiss();
}

TEST_F(Material3DialogTest, BasicDialogRejectsBodyFromAnotherApplication) {
  roo::byte other_raster[32 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> other_device(
      32, 32, other_raster, roo_display::Argb4444());
  roo_display::Display other_display(other_device);
  Environment other_environment(scheduler_);
  Application other_app(&other_environment, other_display);
  TestPanel other_content(other_app.context());
  Task& other_owner = other_app.addTaskFullScreen(other_content);
  DialogActionSpec action{1, "OK", DialogActionRole::kAcknowledge};
  TestBasicDialog dialog(app_.context(), WidgetRef(), &action, 1);

  EXPECT_EQ(DialogShowResult::kSurfaceUnavailable, dialog.show(other_owner));
  EXPECT_FALSE(dialog.isShowing());
  EXPECT_EQ(nullptr, dialog.parent());
}

TEST_F(Material3DialogTest, DerivedInlineBodyUsesPredestructionSeam) {
  bool detached_before_delete = false;
  DialogActionSpec action{1, "OK", DialogActionRole::kAcknowledge};
  {
    InlineBodyDialog dialog(app_.context(), detached_before_delete, &action);
    ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  }
  EXPECT_TRUE(detached_before_delete);
}

TEST_F(Material3DialogTest, FullScreenDialogCoversWindowWithSquareSurface) {
  TestContent body(app_.context());
  TestFullScreenDialog dialog(app_.context(), WidgetRef(body));
  dialog.setHeaderTitle("Edit schedule");

  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  EXPECT_EQ(0, dialog.offsetLeft());
  EXPECT_EQ(0, dialog.offsetTop());
  EXPECT_EQ(320, dialog.width());
  EXPECT_EQ(240, dialog.height());
  EXPECT_FALSE(dialog.getBorderStyle().hasRoundedCorners());
  dialog.dismiss();
}

TEST_F(Material3DialogTest, FullScreenCloseRequestCanVetoThenAccept) {
  TestFullScreenDialog dialog(app_.context(), WidgetRef());
  dialog.allow_dismiss = false;
  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));

  test::DialogTestAccess::CloseButton(dialog).onClicked();
  EXPECT_TRUE(dialog.isShowing());
  EXPECT_EQ(1, dialog.dismiss_request_count);
  EXPECT_EQ(DialogDismissReason::kCloseButton, dialog.last_request_reason);

  dialog.allow_dismiss = true;
  test::DialogTestAccess::CloseButton(dialog).onClicked();
  EXPECT_FALSE(dialog.isShowing());
  EXPECT_EQ(1, dialog.dismiss_count);
  EXPECT_EQ(DialogDismissReason::kCloseButton, dialog.last_dismiss_reason);
  EXPECT_TRUE(dialog.detached_during_callback);
}

TEST_F(Material3DialogTest, FullScreenBackAndEscapeUseVetoHook) {
  TestFullScreenDialog dialog(app_.context(), WidgetRef());
  dialog.allow_dismiss = false;
  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  EXPECT_EQ(BackResult::kHandled, owner_.requestBack(BackSource::kBackKey));
  EXPECT_TRUE(dialog.isShowing());
  EXPECT_EQ(DialogDismissReason::kBack, dialog.last_request_reason);

  dialog.allow_dismiss = true;
  EXPECT_EQ(BackResult::kHandled, owner_.requestBack(BackSource::kEscapeKey));
  EXPECT_FALSE(dialog.isShowing());
  EXPECT_EQ(DialogDismissReason::kEscape, dialog.last_dismiss_reason);
}

TEST_F(Material3DialogTest, FullScreenProgrammaticDismissBypassesVeto) {
  TestFullScreenDialog dialog(app_.context(), WidgetRef());
  dialog.allow_dismiss = false;
  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));

  dialog.dismiss();

  EXPECT_EQ(0, dialog.dismiss_request_count);
  EXPECT_EQ(1, dialog.dismiss_count);
  EXPECT_EQ(DialogDismissReason::kProgrammatic, dialog.last_dismiss_reason);
}

TEST_F(Material3DialogTest, FullScreenConfirmCanRejectThenAccept) {
  TestFullScreenDialog dialog(app_.context(), WidgetRef());
  DialogActionSpec confirm{23, "Save", DialogActionRole::kConfirm};
  dialog.setConfirmAction(confirm);
  dialog.allow_confirm = false;
  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));

  test::DialogTestAccess::ConfirmButton(dialog).onClicked();
  EXPECT_TRUE(dialog.isShowing());
  EXPECT_EQ(1, dialog.confirm_request_count);
  EXPECT_EQ(0, dialog.confirmed_count);

  dialog.allow_confirm = true;
  test::DialogTestAccess::ConfirmButton(dialog).onClicked();
  EXPECT_FALSE(dialog.isShowing());
  EXPECT_EQ(2, dialog.confirm_request_count);
  EXPECT_EQ(1, dialog.confirmed_count);
  EXPECT_EQ(23, dialog.last_confirm_id);
  EXPECT_TRUE(dialog.detached_during_callback);
}

TEST_F(Material3DialogTest, FullScreenHeaderMirrorsWithLayoutDirection) {
  TestFullScreenDialog dialog(app_.context(), WidgetRef());
  DialogActionSpec confirm{23, "Save", DialogActionRole::kConfirm};
  dialog.setConfirmAction(confirm);
  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  Widget& close = test::DialogTestAccess::CloseButton(dialog);
  Widget& save = test::DialogTestAccess::ConfirmButton(dialog);
  EXPECT_LT(close.offsetLeft(), save.offsetLeft());

  dialog.setLayoutDirection(LayoutDirection::kRightToLeft);
  ASSERT_TRUE(app_.refresh());
  EXPECT_GT(close.offsetLeft(), save.offsetLeft());
  dialog.dismiss();
}

TEST_F(Material3DialogTest, FullScreenOwnsCallLocalHeaderTitle) {
  TestFullScreenDialog dialog(app_.context(), WidgetRef());
  {
    std::string title = "Owned wizard title";
    dialog.setHeaderTitle(std::move(title));
  }
  EXPECT_EQ("Owned wizard title", dialog.headerTitle());
}

TEST_F(Material3DialogTest, FullScreenRejectsNonConfirmHeaderAction) {
  TestFullScreenDialog dialog(app_.context(), WidgetRef());
  DialogActionSpec dismiss{1, "Cancel", DialogActionRole::kDismiss};
  EXPECT_DEATH_IF_SUPPORTED(dialog.setConfirmAction(dismiss), "");
}

TEST_F(Material3DialogTest, FullScreenDialogIsMutuallyExclusiveWithBasic) {
  DialogActionSpec action{1, "OK", DialogActionRole::kAcknowledge};
  TestBasicDialog basic(app_.context(), WidgetRef(), &action, 1);
  TestFullScreenDialog full_screen(app_.context(), WidgetRef());
  ASSERT_EQ(DialogShowResult::kShown, basic.show(owner_));

  EXPECT_EQ(DialogShowResult::kHostBusy, full_screen.show(owner_));
  EXPECT_FALSE(full_screen.isShowing());
  basic.dismiss();
}

TEST_F(Material3DialogTest, ActiveFullScreenDestructionCancelsHost) {
  TestContent body(app_.context());
  auto dialog =
      std::make_unique<TestFullScreenDialog>(app_.context(), WidgetRef(body));
  ASSERT_EQ(DialogShowResult::kShown, dialog->show(owner_));

  dialog.reset();

  EXPECT_EQ(nullptr, body.parent());
  EXPECT_FALSE(
      app_.root().transient_presentation_slot().hasActivePresentation());
}

TEST(Material3DialogSize, SharedScaffoldRemainsBounded) {
  EXPECT_LE(sizeof(TestScaffold), 1024u + 24u * sizeof(void*));
  EXPECT_LE(sizeof(internal::DialogActionStrip), 768u + 16u * sizeof(void*));
  EXPECT_LE(sizeof(BasicDialog), 2048u + 48u * sizeof(void*));
  EXPECT_LE(sizeof(AlertDialog), 2560u + 64u * sizeof(void*));
  EXPECT_LE(sizeof(FullScreenDialog), 2560u + 64u * sizeof(void*));
}

}  // namespace
}  // namespace roo_windows::material3
