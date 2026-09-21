#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/containers/list_layout.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace {
class Rows : public ListModel {
 public:
  int elementCount() const override { return 40; }
  void set(int, Widget&) const override {}
};
class Row : public BasicWidget {
 public:
  explicit Row(ApplicationContext& context) : BasicWidget(context) {}
  Dimensions getSuggestedMinimumDimensions() const override {
    return {100, 72};
  }
  void paint(PaintContext& context) const override { context.clear(); }
};

// Verifies a populated row pool detaches before destruction; under ASan this
// catches Panel reading row ownership flags after the pool has freed its rows.
TEST(ListLayoutLifetime, DestroysPopulatedPoolAfterNavigationClear) {
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  roo::byte pixels[240 * 320 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      240, 320, pixels, roo_display::Argb4444());
  roo_display::Display display(device);
  Application app(&environment, display);
  Rows model;
  ListLayout list(app.context(), model,
                  [&]() { return std::make_unique<Row>(app.context()); });
  SimpleScrollablePanel scroll(app.context(), list);
  Task& task = app.addTaskFullScreen(scroll);
  ASSERT_TRUE(app.refresh());
  scroll.scrollTo(0, -200);
  ASSERT_TRUE(app.refresh());
  EXPECT_GT(list.first(), 0);
  task.navigation().clear();
}
}  // namespace
}  // namespace roo_windows
