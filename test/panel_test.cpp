#include "roo_windows/core/panel.h"

#include <memory>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_icons/outlined/24/action.h"
#include "roo_scheduler.h"
#include "roo_windows/composites/menu/basic_navigation_item.h"
#include "roo_windows/composites/radio/radio_list.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/navigation_panel.h"
#include "roo_windows/containers/navigation_rail.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/widgets/blank.h"
#include "roo_windows/widgets/toggle_buttons.h"

namespace roo_windows {
namespace {

class TestPanel : public Panel {
 public:
  using Panel::add;
  using Panel::Panel;
  using Panel::removeAll;
};

class TrackedChild : public Blank {
 public:
  TrackedChild(ApplicationContext& context, int& destructions)
      : Blank(context, Dimensions(10, 10)), destructions_(destructions) {}

  ~TrackedChild() override {
    EXPECT_EQ(parent(), nullptr);
    ++destructions_;
  }

 private:
  int& destructions_;
};

template <typename T>
class PanelLifetimeTest : public testing::Test {
 protected:
  roo_scheduler::Scheduler scheduler_;
  Environment env_{scheduler_};
  ApplicationContext context_{env_.scheduler(), env_.theme(),
                              env_.keyboardColorTheme()};
};

using PanelTypes =
    testing::Types<TestPanel, VerticalLayout, HorizontalLayout, FlexLayout>;
TYPED_TEST_SUITE(PanelLifetimeTest, PanelTypes);

// Verifies base and layout destructors detach surviving borrowed children and
// delete multiple owned children exactly once, with their parent cleared first.
TYPED_TEST(PanelLifetimeTest, DetachesBorrowedAndDestroysOwnedChildren) {
  int destructions = 0;
  TrackedChild borrowed(this->context_, destructions);
  {
    TypeParam panel(this->context_);
    panel.add(std::make_unique<TrackedChild>(this->context_, destructions));
    panel.add(borrowed);
    panel.add(std::make_unique<TrackedChild>(this->context_, destructions));
    EXPECT_EQ(borrowed.parent(), &panel);
    panel.measure(WidthSpec::Exactly(50), HeightSpec::Exactly(50));
    panel.layout(Rect(0, 0, 49, 49));
    for (Widget* child : panel.children()) {
      child->layout(Rect(0, 0, 9, 9));
    }
  }
  EXPECT_EQ(borrowed.parent(), nullptr);
  EXPECT_EQ(destructions, 2);
  TypeParam replacement(this->context_);
  replacement.add(borrowed);
  EXPECT_EQ(borrowed.parent(), &replacement);
}

// Verifies explicit early cleanup remains safe when the base destructor runs.
TEST(PanelLifetime, ExplicitCleanupIsIdempotent) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context(env.scheduler(), env.theme(),
                             env.keyboardColorTheme());
  int destructions = 0;
  TrackedChild borrowed(context, destructions);
  {
    TestPanel panel(context);
    panel.add(borrowed);
    panel.add(std::make_unique<TrackedChild>(context, destructions));
    panel.removeAll();
    panel.removeAll();
    EXPECT_TRUE(panel.children().empty());
    EXPECT_EQ(borrowed.parent(), nullptr);
    EXPECT_EQ(destructions, 1);
  }
  EXPECT_EQ(destructions, 1);
}

// Verifies legacy composites detach their borrowed members before destroying
// their storage. ASan also checks nested layouts and member-owned heap
// children.
TEST(PanelLifetime, CompositeMembersDetachBeforeDestruction) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  ApplicationContext context(env.scheduler(), env.theme(),
                             env.keyboardColorTheme());
  const MonoIcon& icon = ic_outlined_24_action_done();
  int destructions = 0;
  {
    RadioListItem item(context,
                       std::make_unique<TrackedChild>(context, destructions),
                       [](int) {});
  }
  EXPECT_EQ(destructions, 1);
  {
    ToggleButtons buttons(context);
    buttons.addButton(icon);
    buttons.addButton(icon);
  }
  {
    NavigationRail rail(context);
    rail.addDestination(icon, "Home", []() {});
    rail.addDestination(icon, "Settings", []() {});
  }
  Blank page(context, Dimensions(10, 10));
  {
    NavigationPanel panel(context);
    panel.addPage(icon, "Home", page);
  }
  EXPECT_EQ(page.parent(), nullptr);
}

class TestDestination : public Destination {
 public:
  explicit TestDestination(Widget& content) : content_(content) {}
  Widget& getContents() override { return content_; }

 private:
  Widget& content_;
};

// Verifies both navigation item variants detach inline children, including the
// nested column, while their borrowed labels are still alive.
TEST(PanelLifetime, NavigationItemMembersDetachBeforeDestruction) {
  roo_scheduler::Scheduler scheduler;
  Environment env(scheduler);
  roo::byte pixels[32 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 32, pixels, roo_display::Argb4444());
  roo_display::Display display(device);
  Application app(&env, display);
  Blank page(app.context(), Dimensions(10, 10));
  TestDestination destination(page);
  Task& task = app.addTaskFullScreen(page);
  {
    menu::BasicNavigationItem item(app.context(), ic_outlined_24_action_done(),
                                   "Home", task.navigation(), destination);
    menu::BasicNavigationItemWithSubtext item_with_subtext(
        app.context(), ic_outlined_24_action_done(), "Home", "Details",
        task.navigation(), destination);
  }
  task.navigation().clear();
}

}  // namespace
}  // namespace roo_windows
