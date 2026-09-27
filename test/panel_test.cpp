#include "roo_windows/core/panel.h"

#include <memory>

#include "gtest/gtest.h"
#include "roo_scheduler.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/widgets/blank.h"

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

}  // namespace
}  // namespace roo_windows
