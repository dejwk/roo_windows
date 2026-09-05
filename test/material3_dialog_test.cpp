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
#include "roo_windows/material3/dialog/dialog_scaffold.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_windows::material3 {
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

class Material3DialogTest : public ::testing::Test {
 protected:
  Material3DialogTest()
      : device_(64, 48, raster_, roo_display::Argb4444()),
        display_(device_),
        environment_(scheduler_),
        app_(&environment_, display_),
        task_content_(app_.context()),
        owner_(app_.addTaskFullScreen(task_content_)) {}

  roo::byte raster_[64 * 48 * 2] = {};
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

TEST_F(Material3DialogTest, InactiveBodyReplacementSelectsNewFocusOnReentry) {
  TestContent first(app_.context());
  TestContent second(app_.context());
  TestScaffold dialog(app_.context(), WidgetRef(first));

  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
  EXPECT_EQ(&first, owner_.focus().focused());
  dialog.dismiss();
  dialog.setBody(WidgetRef(second));
  ASSERT_EQ(DialogShowResult::kShown, dialog.show(owner_));
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

TEST(Material3DialogSize, SharedScaffoldRemainsBounded) {
  EXPECT_LE(sizeof(TestScaffold), 1024u + 24u * sizeof(void*));
  EXPECT_LE(sizeof(internal::DialogActionStrip), 768u + 16u * sizeof(void*));
}

}  // namespace
}  // namespace roo_windows::material3
