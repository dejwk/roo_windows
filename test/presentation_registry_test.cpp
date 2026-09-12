#include <memory>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {
namespace {

class TestWidget final : public BasicWidget {
 public:
  explicit TestWidget(ApplicationContext& context) : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }
};

class TestPanel final : public Panel {
 public:
  explicit TestPanel(ApplicationContext& context) : Panel(context) {}

  using Panel::add;
};

// Verifies presentation follows every ancestor's visibility, not dimensions.
TEST(PresentationRegistry, QueryClassifiesVisibleAndHiddenAncestors) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);

  auto parent = std::make_unique<TestPanel>(app.context());
  TestPanel* raw_parent = parent.get();
  auto child = std::make_unique<TestWidget>(app.context());
  TestWidget* raw_child = child.get();
  raw_parent->add(WidgetRef(std::move(child)), Rect(0, 0, -1, -1));
  app.add(WidgetRef(std::move(parent)), roo_display::Box(0, 0, 15, 15));

  EXPECT_EQ(PresentationState::kPresented, raw_child->presentationState());
  EXPECT_TRUE(raw_child->isPresented());

  raw_parent->setVisibility(Visibility::kInvisible);
  EXPECT_EQ(PresentationState::kHidden, raw_child->presentationState());
  EXPECT_FALSE(raw_child->isPresented());

  raw_parent->setVisibility(Visibility::kVisible);
  raw_child->setVisibility(Visibility::kGone);
  EXPECT_EQ(PresentationState::kHidden, raw_child->presentationState());
}

// Verifies unattached widgets are detached even when locally visible.
TEST(PresentationRegistry, QueryClassifiesDetachedWidget) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  TestWidget widget(app.context());

  EXPECT_EQ(PresentationState::kDetached, widget.presentationState());
  EXPECT_FALSE(widget.isPresented());
}

// Verifies a surviving widget does not access its expired application context.
TEST(PresentationRegistry, QueryClassifiesExpiredContextAsDetached) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  std::unique_ptr<TestWidget> widget;
  {
    Application app(&environment, display);
    widget = std::make_unique<TestWidget>(app.context());
  }

  EXPECT_EQ(PresentationState::kDetached, widget->presentationState());
  EXPECT_FALSE(widget->isPresented());
}

// Verifies each application resolves only its own MainWindow.
TEST(PresentationRegistry, QueryKeepsIndependentApplicationsIndependent) {
  roo::byte first_raster[16 * 16 * 2] = {};
  roo::byte second_raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> first_device(
      16, 16, first_raster, roo_display::Argb4444());
  roo_display::OffscreenDevice<roo_display::Argb4444> second_device(
      16, 16, second_raster, roo_display::Argb4444());
  roo_display::Display first_display(first_device);
  roo_display::Display second_display(second_device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application first(&environment, first_display);
  Application second(&environment, second_display);

  auto first_widget = std::make_unique<TestWidget>(first.context());
  auto second_widget = std::make_unique<TestWidget>(second.context());
  TestWidget* raw_first = first_widget.get();
  TestWidget* raw_second = second_widget.get();
  first.add(WidgetRef(std::move(first_widget)), roo_display::Box(0, 0, 15, 15));
  second.add(WidgetRef(std::move(second_widget)),
             roo_display::Box(0, 0, 15, 15));

  EXPECT_EQ(PresentationState::kPresented, raw_first->presentationState());
  EXPECT_EQ(PresentationState::kPresented, raw_second->presentationState());
}

}  // namespace
}  // namespace roo_windows
