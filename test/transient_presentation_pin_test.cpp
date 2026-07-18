#include <memory>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"

namespace roo_windows {
namespace {

class PinAnchor final : public BasicWidget {
 public:
  explicit PinAnchor(ApplicationContext& context) : BasicWidget(context) {}

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(1, 1);
  }
};

class CountingPin final : public PresentationPin {
 public:
  explicit CountingPin(int& destroyed) : destroyed_(destroyed) {}
  ~CountingPin() override { ++destroyed_; }

 protected:
  Rect boundsInWindow() const override { return Rect(2, 2, 7, 7); }
  void paint(PaintContext& ctx) const override { ctx.clear(); }

 private:
  int& destroyed_;
};

// Verifies rejected pins are consumed and an attached anchor owns exactly one
// active registration until it explicitly hides it.
TEST(TransientPresentationPin, RegistrationOwnsAndReleasesPins) {
  roo::byte raster[32 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 32, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  Application app(&environment, display);
  int destroyed = 0;

  PinAnchor detached(app.context());
  EXPECT_EQ(PresentationPinShowResult::kAnchorUnavailable,
            detached.showPresentationPin(
                std::make_unique<CountingPin>(destroyed)));
  EXPECT_EQ(1, destroyed);

  auto anchor = std::make_unique<PinAnchor>(app.context());
  PinAnchor* raw_anchor = anchor.get();
  app.add(WidgetRef(std::move(anchor)), roo_display::Box(0, 0, 31, 31));
  EXPECT_EQ(PresentationPinShowResult::kShown,
            raw_anchor->showPresentationPin(
                std::make_unique<CountingPin>(destroyed)));
  EXPECT_TRUE(raw_anchor->hasPresentationPin());
  EXPECT_EQ(PresentationPinShowResult::kAlreadyRegistered,
            raw_anchor->showPresentationPin(
                std::make_unique<CountingPin>(destroyed)));
  EXPECT_EQ(2, destroyed);

  raw_anchor->hidePresentationPin();
  EXPECT_FALSE(raw_anchor->hasPresentationPin());
  EXPECT_EQ(3, destroyed);
}

// Verifies an active pin is removed before a parent-owned anchor is destroyed.
TEST(TransientPresentationPin, AnchorTeardownDeletesActivePin) {
  roo::byte raster[32 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 32, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  int destroyed = 0;
  {
    Application app(&environment, display);
    auto anchor = std::make_unique<PinAnchor>(app.context());
    PinAnchor* raw_anchor = anchor.get();
    app.add(WidgetRef(std::move(anchor)), roo_display::Box(0, 0, 31, 31));
    EXPECT_EQ(PresentationPinShowResult::kShown,
              raw_anchor->showPresentationPin(
                  std::make_unique<CountingPin>(destroyed)));
  }
  EXPECT_EQ(1, destroyed);
}

}  // namespace
}  // namespace roo_windows
