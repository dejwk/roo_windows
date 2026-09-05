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
        item_(StandardMenuItemInit{"Open", {}, nullptr, true, false, false}),
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
  StandardMenuItem item_;
  MenuEntry row_;
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

}  // namespace
}  // namespace roo_windows::material3
