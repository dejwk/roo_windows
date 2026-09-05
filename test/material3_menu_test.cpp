#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/material3/menu/menu.h"

namespace roo_windows::material3 {
namespace {

class TestPanel final : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeLast;
};

class SourceWidget final : public BasicWidget {
 public:
  explicit SourceWidget(ApplicationContext& context) : BasicWidget(context) {}
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(40, 24);
  }
};

class RecordingMenu final : public Menu {
 public:
  explicit RecordingMenu(ApplicationContext& context) : Menu(context) {}

  int finishes() const { return finishes_; }
  PresentationFinishReason lastReason() const { return last_reason_; }

 protected:
  void onFinished(PresentationFinishReason reason) override {
    ++finishes_;
    last_reason_ = reason;
  }

 private:
  int finishes_ = 0;
  PresentationFinishReason last_reason_ = PresentationFinishReason::kCancel;
};

class RecordingItem final : public StandardMenuItem {
 public:
  explicit RecordingItem(const StandardMenuItemInit& init)
      : StandardMenuItem(init) {}

  void onInvoked() override { ++invocations_; }
  MenuLeafDismissal leafDismissal() const override { return dismissal_; }
  bool hasSubmenu() const override { return has_submenu_; }
  void populateSubmenu(MenuLevelBuilder& builder) override {
    auto group = std::make_unique<MenuGroup>(*context_);
    StandardMenuItemInit first_init;
    first_init.headline = "Child one";
    auto first =
        std::make_unique<MenuRow<StandardMenuItem>>(*context_, first_init);
    first_child_ = first.get();
    StandardMenuItemInit second_init;
    second_init.headline = "Child two";
    auto second =
        std::make_unique<MenuRow<StandardMenuItem>>(*context_, second_init);
    second_child_ = second.get();
    group->add(std::move(first));
    group->add(std::move(second));
    builder.addGroup(std::move(group));
  }
  int invocations() const { return invocations_; }
  void setDismissal(MenuLeafDismissal dismissal) { dismissal_ = dismissal; }
  void enableSubmenu(ApplicationContext& context) {
    context_ = &context;
    has_submenu_ = true;
  }
  MenuEntry* firstChild() const { return first_child_; }
  MenuEntry* secondChild() const { return second_child_; }

 private:
  int invocations_ = 0;
  MenuLeafDismissal dismissal_ = MenuLeafDismissal::kDefault;
  ApplicationContext* context_ = nullptr;
  MenuEntry* first_child_ = nullptr;
  MenuEntry* second_child_ = nullptr;
  bool has_submenu_ = false;
};

class TestMenuEntry final : public MenuEntry {
 public:
  using MenuEntry::MenuEntry;

  void Tap() { onSingleTapUp(1, 1); }
  void DeferredClick() { onClicked(); }
  bool Key(KeyCode code, uint8_t modifiers = 0) {
    return onKeyEvent(
        KeyEvent{KeyPhase::kDown, code, modifiers, PhysicalKey::kNone, 0});
  }
};

class Material3MenuTest : public testing::Test {
 protected:
  Material3MenuTest()
      : device_(320, 240, raster_, roo_display::Argb4444()),
        display_(device_),
        environment_(scheduler_),
        app_(&environment_, display_),
        content_(app_.context()),
        source_(app_.context()),
        owner_(app_.addTaskFullScreen(content_)),
        item_(StandardMenuItemInit{"Open", {}, nullptr, true, true, false}),
        row_(app_.context()),
        group_(app_.context()),
        menu_(app_.context()) {
    content_.add(WidgetRef(source_), Rect(40, 32, 79, 55));
    row_.setMenuItem(item_);
    group_.add(row_);
    menu_.addGroup(group_);
    EXPECT_TRUE(app_.refresh());
  }

  ~Material3MenuTest() override {
    menu_.dismissChain();
    menu_.clearGroups();
    group_.clear();
    content_.removeLast();
  }

  roo::byte raster_[320 * 240 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment environment_;
  Application app_;
  TestPanel content_;
  SourceWidget source_;
  Task& owner_;
  RecordingItem item_;
  TestMenuEntry row_;
  MenuGroup group_;
  RecordingMenu menu_;
};

TEST_F(Material3MenuTest, PresentsWithExplicitOwnerAndCapturesFocus) {
  EXPECT_EQ(MenuShowResult::kShown, menu_.show(owner_, source_));
  EXPECT_EQ(&row_, owner_.focus().focused());
  EXPECT_NE(&content_, owner_.focus().scopeRoot());

  EXPECT_EQ(BackResult::kHandled, owner_.requestBack(BackSource::kBackKey));
  EXPECT_EQ(1, menu_.finishes());
  EXPECT_EQ(PresentationFinishReason::kBack, menu_.lastReason());
  EXPECT_NE(&row_, owner_.focus().focused());
}

TEST_F(Material3MenuTest, RejectsDetachedRequiredSourceWithoutMutation) {
  SourceWidget detached(app_.context());
  Widget* prior_scope = owner_.focus().scopeRoot();
  EXPECT_EQ(MenuShowResult::kAnchorUnavailable, menu_.show(owner_, detached));
  EXPECT_EQ(0, menu_.finishes());
  EXPECT_EQ(prior_scope, owner_.focus().scopeRoot());
}

TEST_F(Material3MenuTest, RectanglePresentationNeedsNoWidgetProvenance) {
  EXPECT_EQ(MenuShowResult::kShown,
            menu_.showFromRect(owner_, Rect(300, 220, 300, 220),
                               MenuPlacement::kBelowEnd));
  EXPECT_EQ(&row_, owner_.focus().focused());
  menu_.dismissChain();
  EXPECT_EQ(PresentationFinishReason::kCancel, menu_.lastReason());
}

TEST_F(Material3MenuTest, ActiveInstanceAndReplacementAreDeterministic) {
  EXPECT_EQ(MenuShowResult::kShown, menu_.show(owner_, source_));
  EXPECT_EQ(MenuShowResult::kAlreadyPresented, menu_.show(owner_, source_));

  StandardMenuItem second_item(
      StandardMenuItemInit{"Second", {}, nullptr, true, false, false});
  MenuEntry second_row(app_.context());
  second_row.setMenuItem(second_item);
  MenuGroup second_group(app_.context());
  second_group.add(second_row);
  RecordingMenu second(app_.context());
  second.addGroup(second_group);

  EXPECT_EQ(MenuShowResult::kShown, second.show(owner_, source_));
  EXPECT_EQ(1, menu_.finishes());
  EXPECT_EQ(PresentationFinishReason::kReplacement, menu_.lastReason());
  second.dismissChain();
  second.clearGroups();
  second_group.clear();
}

TEST_F(Material3MenuTest, ReanchorIsAtomicForInvalidRequiredSource) {
  EXPECT_EQ(MenuShowResult::kShown, menu_.show(owner_, source_));
  SourceWidget detached(app_.context());
  EXPECT_FALSE(menu_.reanchor(detached));
  EXPECT_EQ(&row_, owner_.focus().focused());
  EXPECT_TRUE(menu_.reanchorFromRect(Rect(8, 8, 8, 8), MenuPlacement::kAfter));
}

TEST_F(Material3MenuTest, InvalidOptionalTriggerDoesNotFailPresentation) {
  SourceWidget detached(app_.context());
  MenuTriggerPaintSource trigger{detached, 8, 0xFF000000, 20};
  EXPECT_EQ(MenuShowResult::kShown,
            menu_.show(owner_, source_, MenuPlacement::kBelowStart, &trigger));
}

TEST_F(Material3MenuTest, SingleSelectionInvokesOnceAndDismisses) {
  MenuPolicy policy;
  policy.selection_mode = SelectionMode::kSingle;
  menu_.setPolicy(policy);
  ASSERT_EQ(MenuShowResult::kShown, menu_.show(owner_, source_));

  row_.Tap();
  row_.DeferredClick();

  EXPECT_TRUE(item_.isSelected());
  EXPECT_EQ(1, item_.invocations());
  EXPECT_EQ(1, menu_.finishes());
  EXPECT_EQ(PresentationFinishReason::kAction, menu_.lastReason());
  EXPECT_FALSE(row_.isClickable());
}

TEST_F(Material3MenuTest, MultipleSelectionTogglesAndStaysOpenByDefault) {
  MenuPolicy policy;
  policy.selection_mode = SelectionMode::kMultiple;
  menu_.setPolicy(policy);
  ASSERT_EQ(MenuShowResult::kShown, menu_.show(owner_, source_));

  row_.Tap();
  row_.DeferredClick();

  EXPECT_TRUE(item_.isSelected());
  EXPECT_EQ(1, item_.invocations());
  EXPECT_EQ(0, menu_.finishes());
  EXPECT_TRUE(row_.isClickable());
}

TEST_F(Material3MenuTest, LeafDismissalOverrideKeepsSingleSelectionOpen) {
  MenuPolicy policy;
  policy.selection_mode = SelectionMode::kSingle;
  menu_.setPolicy(policy);
  item_.setDismissal(MenuLeafDismissal::kKeepOpen);
  ASSERT_EQ(MenuShowResult::kShown, menu_.show(owner_, source_));

  row_.Tap();

  EXPECT_TRUE(item_.isSelected());
  EXPECT_EQ(0, menu_.finishes());
}

TEST_F(Material3MenuTest, SubmenuOpensAndBackClosesDeepestFirst) {
  item_.enableSubmenu(app_.context());
  ASSERT_EQ(MenuShowResult::kShown, menu_.show(owner_, source_));

  row_.Tap();
  ASSERT_NE(nullptr, item_.firstChild());
  EXPECT_EQ(item_.firstChild(), owner_.focus().focused());
  EXPECT_EQ(0, menu_.finishes());

  EXPECT_EQ(BackResult::kHandled, owner_.requestBack(BackSource::kBackKey));
  EXPECT_EQ(&row_, owner_.focus().focused());
  EXPECT_EQ(0, menu_.finishes());
  EXPECT_EQ(BackResult::kHandled, owner_.requestBack(BackSource::kBackKey));
  EXPECT_EQ(1, menu_.finishes());
}

TEST_F(Material3MenuTest, SubmenuRowsHandleWrappedTraversalAndHomeEnd) {
  item_.enableSubmenu(app_.context());
  ASSERT_EQ(MenuShowResult::kShown, menu_.show(owner_, source_));
  row_.Tap();
  ASSERT_EQ(item_.firstChild(), owner_.focus().focused());

  EXPECT_TRUE(item_.firstChild()->onKeyEvent(
      KeyEvent{KeyPhase::kDown, KeyCode::kDown, 0, PhysicalKey::kNone, 0}));
  EXPECT_EQ(item_.secondChild(), owner_.focus().focused());
  EXPECT_TRUE(item_.secondChild()->onKeyEvent(
      KeyEvent{KeyPhase::kDown, KeyCode::kDown, 0, PhysicalKey::kNone, 0}));
  EXPECT_EQ(item_.firstChild(), owner_.focus().focused());
  EXPECT_TRUE(item_.firstChild()->onKeyEvent(
      KeyEvent{KeyPhase::kDown, KeyCode::kEnd, 0, PhysicalKey::kNone, 0}));
  EXPECT_EQ(item_.secondChild(), owner_.focus().focused());
}

TEST_F(Material3MenuTest, RtlAfterArrowOpensAndBeforeArrowRestoresParent) {
  item_.enableSubmenu(app_.context());
  MenuPolicy policy;
  policy.layout_direction = LayoutDirection::kRightToLeft;
  menu_.setPolicy(policy);
  ASSERT_EQ(MenuShowResult::kShown, menu_.show(owner_, source_));

  EXPECT_TRUE(row_.Key(KeyCode::kLeft));
  ASSERT_EQ(item_.firstChild(), owner_.focus().focused());
  EXPECT_TRUE(item_.firstChild()->onKeyEvent(
      KeyEvent{KeyPhase::kDown, KeyCode::kRight, 0, PhysicalKey::kNone, 0}));
  EXPECT_EQ(&row_, owner_.focus().focused());
}

}  // namespace
}  // namespace roo_windows::material3
