#include <memory>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/transient_surface_host.h"
#include "roo_windows/dialogs/dialog.h"
#include "roo_windows_render_test_support.h"

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

using roo_display::Box;
using roo_display::Color;
using test_support::QuantizeToArgb4444;
using test_support::RooWindowsRenderTest;

class TestPanel final : public Panel {
 public:
  TestPanel(ApplicationContext& context, Color color)
      : Panel(context), color_(color) {}

  using Panel::add;
  using Panel::removeLast;
  Color background() const override { return color_; }
  void paint(PaintContext& ctx) const override { ctx.clear(); }

 private:
  Color color_;
};

struct MutablePinState {
  Rect bounds{2, 2, 7, 7};
  Rect dirty{2, 2, 7, 7};
  Rect clip{-32768, -32768, 32767, 32767};
  Color color = roo_display::color::Green;
  int painted = 0;
  int destroyed = 0;
};

class MutablePin final : public PresentationPin {
 public:
  explicit MutablePin(MutablePinState& state) : state_(state) {}
  ~MutablePin() override { ++state_.destroyed; }

 protected:
  Rect boundsInWindow() const override { return state_.bounds; }
  Rect dirtyBoundsInWindow() const override { return state_.dirty; }
  Rect clipBoundsInWindow() const override { return state_.clip; }
  void paint(PaintContext& ctx) const override {
    ++state_.painted;
    Rect settled = Rect::Intersect(state_.bounds, ctx.localClip());
    ctx.fillRect(settled, state_.color);
    ctx.addExclusion(settled);
  }

 private:
  MutablePinState& state_;
};

struct HostedPinState {
  Color color = roo_display::color::Green;
  int painted = 0;
  int destroyed = 0;
};

class FrozenHostedPin final : public PresentationPin {
 public:
  FrozenHostedPin(const internal::TransientSourceGeometry& geometry,
                  HostedPinState& state)
      : bounds_(geometry.bounds_in_window), state_(state) {}

  FrozenHostedPin(Rect bounds, HostedPinState& state)
      : bounds_(bounds), state_(state) {}

  ~FrozenHostedPin() override { ++state_.destroyed; }

 protected:
  Rect boundsInWindow() const override { return bounds_; }

  void paint(PaintContext& ctx) const override {
    ++state_.painted;
    Rect settled = Rect::Intersect(bounds_, ctx.localClip());
    ctx.fillRect(settled, state_.color);
    ctx.addExclusion(settled);
  }

 private:
  Rect bounds_;
  HostedPinState& state_;
};

class HostedRegistration final : public TransientPresentationRegistration {
 protected:
  void detachPresentation(PresentationFinishReason reason) override {
    (void)reason;
  }
};

constexpr TransientSurfaceSpec kHostedPinSpec{
    TransientBarrierPaint::kTransparent,
    TransientAdmissionPolicy::kRejectIfBusy, OutsideInteractionPolicy::kAbsorb,
    TransientPresentationPolicy(), false};

class HostedPinTest : public ::testing::Test {
 protected:
  HostedPinTest()
      : device_(64, 48, raster_, roo_display::Argb4444()),
        display_(device_),
        environment_(scheduler_),
        app_(&environment_, display_) {
    owner_content_ =
        std::make_unique<TestPanel>(app_.context(), roo_display::color::Red);
    owner_ = &app_.addTaskFullScreen(*owner_content_);
    host_ = &internal::GetTransientSurfaceHost(*owner_);
  }

  Color pixelAt(int16_t x, int16_t y) const {
    int16_t px[] = {x};
    int16_t py[] = {y};
    Color color[1];
    device_.raster().readColors(px, py, 1, color);
    return color[0];
  }

  bool refresh() { return app_.refresh(); }

  roo::byte raster_[64 * 48 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment environment_;
  // Declared before Application so the borrowed task content outlives it.
  std::unique_ptr<TestPanel> owner_content_;
  Application app_;
  Task* owner_ = nullptr;
  internal::TransientSurfaceHost* host_ = nullptr;
};

class TestDialog final : public Dialog {
 public:
  explicit TestDialog(ApplicationContext& context) : Dialog(context, {}) {}
};

std::unique_ptr<MutablePin> MakePin(MutablePinState& state) {
  return std::make_unique<MutablePin>(state);
}

// Verifies a hosted pin shares the owner layer without consuming the ordinary
// widget-pin identity of that exact owner root.
TEST_F(HostedPinTest, HostedAndOwnerRootWidgetPinsRemainDistinct) {
  Widget& owner_root = *owner_->focus().scopeRoot();
  MutablePinState widget_state;
  widget_state.color = roo_display::color::Blue;
  widget_state.bounds = Rect(2, 2, 12, 12);
  widget_state.dirty = widget_state.bounds;
  ASSERT_EQ(PresentationPinShowResult::kShown,
            owner_root.showPresentationPin(MakePin(widget_state)));

  TestPanel hosted_root(app_.context(), roo_display::color::Yellow);
  FocusScope scope;
  HostedRegistration registration;
  ASSERT_EQ(PresentationStartResult::kStarted,
            host_->show(registration, *owner_, hosted_root, Rect(2, 2, 7, 7),
                        scope, kHostedPinSpec));
  HostedPinState hosted_state;
  ASSERT_EQ(PresentationPinShowResult::kShown,
            host_->showPresentationPin(registration,
                                       std::make_unique<FrozenHostedPin>(
                                           Rect(2, 2, 12, 12), hosted_state)));
  EXPECT_TRUE(owner_root.hasPresentationPin());
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Yellow), pixelAt(4, 4));
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Green), pixelAt(10, 10));

  host_->hidePresentationPin(registration);
  EXPECT_EQ(1, hosted_state.destroyed);
  EXPECT_TRUE(owner_root.hasPresentationPin());
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Blue), pixelAt(10, 10));
  registration.finish(PresentationFinishReason::kCancel);
  owner_root.hidePresentationPin();
}

// Verifies a hosted pin retains copied source geometry after the source moves
// and detaches, supports handle-based dirtiness, and is hidden before finish.
TEST_F(HostedPinTest, HostedPinUsesFrozenGeometryAndDeterministicCleanup) {
  PinAnchor source(app_.context());
  owner_content_->add(WidgetRef(source), Rect(3, 4, 8, 9));
  ASSERT_TRUE(refresh());
  internal::TransientSourceGeometry geometry;
  ASSERT_TRUE(
      internal::CaptureTransientSourceGeometry(*owner_, source, geometry));

  TestPanel hosted_root(app_.context(), roo_display::color::Yellow);
  FocusScope scope;
  HostedRegistration registration;
  ASSERT_EQ(PresentationStartResult::kStarted,
            host_->show(registration, *owner_, hosted_root,
                        Rect(40, 30, 55, 43), scope, kHostedPinSpec));
  HostedPinState state;
  ASSERT_EQ(
      PresentationPinShowResult::kShown,
      host_->showPresentationPin(
          registration, std::make_unique<FrozenHostedPin>(geometry, state)));
  owner_content_->removeLast();
  owner_content_->add(WidgetRef(source), Rect(20, 20, 25, 25));
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Green), pixelAt(4, 5));
  EXPECT_NE(QuantizeToArgb4444(roo_display::color::Green), pixelAt(22, 22));

  state.color = roo_display::color::Blue;
  host_->setPresentationPinDirty(registration);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Blue), pixelAt(4, 5));
  registration.finish(PresentationFinishReason::kCancel);
  EXPECT_EQ(1, state.destroyed);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Red), pixelAt(4, 5));
  owner_content_->removeLast();
}

// Verifies hosted pin admission consumes failures, permits one pin per active
// registration, and rejects use after the hosted session has ended.
TEST_F(HostedPinTest, HostedPinAdmissionValidatesActiveRegistration) {
  HostedRegistration registration;
  HostedPinState inactive_state;
  EXPECT_EQ(PresentationPinShowResult::kAnchorUnavailable,
            host_->showPresentationPin(registration,
                                       std::make_unique<FrozenHostedPin>(
                                           Rect(2, 2, 7, 7), inactive_state)));
  EXPECT_EQ(1, inactive_state.destroyed);

  TestPanel hosted_root(app_.context(), roo_display::color::Yellow);
  FocusScope scope;
  ASSERT_EQ(PresentationStartResult::kStarted,
            host_->show(registration, *owner_, hosted_root,
                        Rect(40, 30, 55, 43), scope, kHostedPinSpec));
  EXPECT_EQ(PresentationPinShowResult::kAllocationFailed,
            host_->showPresentationPin(registration, nullptr));
  HostedPinState active_state;
  HostedPinState duplicate_state;
  ASSERT_EQ(PresentationPinShowResult::kShown,
            host_->showPresentationPin(registration,
                                       std::make_unique<FrozenHostedPin>(
                                           Rect(2, 2, 7, 7), active_state)));
  EXPECT_EQ(PresentationPinShowResult::kAlreadyRegistered,
            host_->showPresentationPin(
                registration, std::make_unique<FrozenHostedPin>(
                                  Rect(8, 8, 12, 12), duplicate_state)));
  EXPECT_EQ(1, duplicate_state.destroyed);
  registration.finish(PresentationFinishReason::kCancel);
  EXPECT_EQ(1, active_state.destroyed);
}

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
  EXPECT_EQ(
      PresentationPinShowResult::kAnchorUnavailable,
      detached.showPresentationPin(std::make_unique<CountingPin>(destroyed)));
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

// Verifies null allocation failure is explicit and consumes no host entry.
TEST_F(RooWindowsRenderTest, NullPinReportsAllocationFailure) {
  auto anchor = std::make_unique<PinAnchor>(context());
  PinAnchor* raw = anchor.get();
  app_.add(std::move(anchor), Box(0, 0, 31, 31));
  EXPECT_EQ(PresentationPinShowResult::kAllocationFailed,
            raw->showPresentationPin(nullptr));
  EXPECT_FALSE(raw->hasPresentationPin());
}

// Verifies hiding before the first frame deletes the pin and never writes its
// pixels to the display.
TEST_F(RooWindowsRenderTest, HideBeforeFirstPaintLeavesNoPixels) {
  auto panel = std::make_unique<TestPanel>(context(), roo_display::color::Red);
  auto anchor = std::make_unique<PinAnchor>(context());
  PinAnchor* raw = anchor.get();
  panel->add(std::move(anchor), Rect(10, 10, 19, 19));
  app_.add(std::move(panel), Box(0, 0, kWidth - 1, kHeight - 1));
  MutablePinState state;
  ASSERT_EQ(PresentationPinShowResult::kShown,
            raw->showPresentationPin(MakePin(state)));
  raw->hidePresentationPin();
  ASSERT_TRUE(refresh());
  EXPECT_EQ(0, state.painted);
  EXPECT_EQ(1, state.destroyed);
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Red), pixelAt(4, 4));
}

// Verifies a pin escapes a clipped intermediate ancestor while the default
// final clip remains the MainWindow bounds.
TEST_F(RooWindowsRenderTest, EscapesClippedAncestorAndClipsAtWindow) {
  auto root = std::make_unique<TestPanel>(context(), roo_display::color::Red);
  auto clipped =
      std::make_unique<TestPanel>(context(), roo_display::color::Blue);
  auto anchor = std::make_unique<PinAnchor>(context());
  PinAnchor* raw = anchor.get();
  clipped->add(std::move(anchor), Rect(2, 2, 5, 5));
  root->add(std::move(clipped), Rect(20, 20, 39, 39));
  app_.add(std::move(root), Box(0, 0, kWidth - 1, kHeight - 1));
  MutablePinState state;
  state.bounds = Rect(-4, 3, 23, 8);
  state.dirty = state.bounds;
  ASSERT_EQ(PresentationPinShowResult::kShown,
            raw->showPresentationPin(MakePin(state)));
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Green), pixelAt(0, 5));
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Green), pixelAt(21, 5));
  EXPECT_EQ(ParentClipMode::kClipped, raw->getParentClipMode());
  raw->hidePresentationPin();
}

// Verifies clean-window preflight invalidates the complete old and new bounds
// when pin geometry moves without an explicit widget dirty request.
TEST_F(RooWindowsRenderTest, GeometryPreflightRestoresOldAndPaintsNewBounds) {
  auto panel = std::make_unique<TestPanel>(context(), roo_display::color::Red);
  auto anchor = std::make_unique<PinAnchor>(context());
  PinAnchor* raw = anchor.get();
  panel->add(std::move(anchor), Rect(10, 10, 19, 19));
  app_.add(std::move(panel), Box(0, 0, kWidth - 1, kHeight - 1));
  MutablePinState state;
  ASSERT_EQ(PresentationPinShowResult::kShown,
            raw->showPresentationPin(MakePin(state)));
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Green), pixelAt(4, 4));
  state.bounds = Rect(40, 30, 45, 35);
  state.dirty = state.bounds;
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Red), pixelAt(4, 4));
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Green), pixelAt(42, 32));
  raw->hidePresentationPin();
}

// Verifies a narrow content-dirty request does not discard the full envelope
// needed to restore pixels after the next geometry change.
TEST_F(RooWindowsRenderTest, NarrowDirtyPreservesPresentedEnvelope) {
  auto panel = std::make_unique<TestPanel>(context(), roo_display::color::Red);
  auto anchor = std::make_unique<PinAnchor>(context());
  PinAnchor* raw = anchor.get();
  panel->add(std::move(anchor), Rect(10, 10, 19, 19));
  app_.add(std::move(panel), Box(0, 0, kWidth - 1, kHeight - 1));
  MutablePinState state;
  state.bounds = Rect(2, 2, 9, 9);
  state.dirty = Rect(4, 4, 4, 4);
  ASSERT_EQ(PresentationPinShowResult::kShown,
            raw->showPresentationPin(MakePin(state)));
  ASSERT_TRUE(refresh());
  state.color = roo_display::color::Yellow;
  raw->setPresentationPinDirty();
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Yellow), pixelAt(4, 4));
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Green), pixelAt(8, 8));
  state.bounds = Rect(30, 30, 37, 37);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Red), pixelAt(8, 8));
  raw->hidePresentationPin();
}

// Verifies effective ancestor visibility suppresses and restores the existing
// registration without dereferencing a detached anchor.
TEST_F(RooWindowsRenderTest, AncestorVisibilitySuppressesAndResumesPin) {
  auto panel = std::make_unique<TestPanel>(context(), roo_display::color::Red);
  TestPanel* panel_raw = panel.get();
  auto anchor = std::make_unique<PinAnchor>(context());
  PinAnchor* raw = anchor.get();
  panel->add(std::move(anchor), Rect(10, 10, 19, 19));
  app_.add(std::move(panel), Box(0, 0, kWidth - 1, kHeight - 1));
  MutablePinState state;
  ASSERT_EQ(PresentationPinShowResult::kShown,
            raw->showPresentationPin(MakePin(state)));
  ASSERT_TRUE(refresh());
  panel_raw->setVisibility(Visibility::kInvisible);
  ASSERT_TRUE(refresh());
  EXPECT_NE(QuantizeToArgb4444(roo_display::color::Green), pixelAt(4, 4));
  EXPECT_TRUE(raw->hasPresentationPin());
  panel_raw->setVisibility(Visibility::kVisible);
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Green), pixelAt(4, 4));
  raw->hidePresentationPin();
}

// Verifies later registration is higher within one layer and removing it
// reveals the earlier pin.
TEST_F(RooWindowsRenderTest, SameRootPinsUseReverseRegistrationOrder) {
  auto panel = std::make_unique<TestPanel>(context(), roo_display::color::Blue);
  auto low_anchor = std::make_unique<PinAnchor>(context());
  auto high_anchor = std::make_unique<PinAnchor>(context());
  PinAnchor* low = low_anchor.get();
  PinAnchor* high = high_anchor.get();
  panel->add(std::move(low_anchor), Rect(0, 0, 5, 5));
  panel->add(std::move(high_anchor), Rect(6, 0, 11, 5));
  app_.add(std::move(panel), Box(0, 0, kWidth - 1, kHeight - 1));
  MutablePinState low_state;
  low_state.color = roo_display::color::Red;
  MutablePinState high_state;
  ASSERT_EQ(PresentationPinShowResult::kShown,
            low->showPresentationPin(MakePin(low_state)));
  ASSERT_EQ(PresentationPinShowResult::kShown,
            high->showPresentationPin(MakePin(high_state)));
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Green), pixelAt(4, 4));
  high->hidePresentationPin();
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Red), pixelAt(4, 4));
  low->hidePresentationPin();
}

// Verifies a task pin stays below popup contents and a popup pin stays above
// ordinary contents in the popup layer.
TEST_F(RooWindowsRenderTest, PinsRespectTaskAndPopupLayerOrdering) {
  auto task = std::make_unique<TestPanel>(context(), roo_display::color::Red);
  auto task_anchor = std::make_unique<PinAnchor>(context());
  PinAnchor* task_raw = task_anchor.get();
  task->add(std::move(task_anchor), Rect(0, 0, 5, 5));
  app_.add(std::move(task), Box(0, 0, kWidth - 1, kHeight - 1));
  auto popup = std::make_unique<TestPanel>(context(), roo_display::color::Blue);
  auto popup_anchor = std::make_unique<PinAnchor>(context());
  PinAnchor* popup_raw = popup_anchor.get();
  popup->add(std::move(popup_anchor), Rect(0, 0, 5, 5));
  app_.addPopup(std::move(popup), Box(0, 0, 20, 20));
  MutablePinState task_state;
  ASSERT_EQ(PresentationPinShowResult::kShown,
            task_raw->showPresentationPin(MakePin(task_state)));
  ASSERT_TRUE(refresh());
  EXPECT_NE(QuantizeToArgb4444(roo_display::color::Green), pixelAt(4, 4));
  MutablePinState popup_state;
  popup_state.color = roo_display::color::Yellow;
  ASSERT_EQ(PresentationPinShowResult::kShown,
            popup_raw->showPresentationPin(MakePin(popup_state)));
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Yellow), pixelAt(4, 4));
  popup_raw->hidePresentationPin();
  task_raw->hidePresentationPin();
}

// Verifies a dialog pin is highest in its layer and can use the window clip
// outside the dialog's own layout bounds.
TEST_F(RooWindowsRenderTest, DialogPinIsHighestInDialogLayer) {
  auto task = std::make_unique<TestPanel>(context(), roo_display::color::Red);
  app_.add(std::move(task), Box(0, 0, kWidth - 1, kHeight - 1));
  TestDialog dialog(context());
  ASSERT_EQ(PresentationStartResult::kStarted,
            app_.showDialog(dialog, [](int) {}));
  MutablePinState state;
  state.color = roo_display::color::Yellow;
  ASSERT_EQ(PresentationPinShowResult::kShown,
            dialog.showPresentationPin(MakePin(state)));
  ASSERT_TRUE(refresh());
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Yellow), pixelAt(4, 4));
  dialog.hidePresentationPin();
}

// Verifies detaching an ancestor removes every active pin in that subtree
// before its anchors lose their parent chain.
TEST_F(RooWindowsRenderTest, AncestorDetachDeletesSubtreePins) {
  auto root = std::make_unique<TestPanel>(context(), roo_display::color::Red);
  TestPanel* root_raw = root.get();
  auto subtree =
      std::make_unique<TestPanel>(context(), roo_display::color::Blue);
  auto first = std::make_unique<PinAnchor>(context());
  auto second = std::make_unique<PinAnchor>(context());
  PinAnchor* first_raw = first.get();
  PinAnchor* second_raw = second.get();
  subtree->add(std::move(first), Rect(0, 0, 5, 5));
  subtree->add(std::move(second), Rect(6, 0, 11, 5));
  root->add(std::move(subtree), Rect(10, 10, 29, 29));
  app_.add(std::move(root), Box(0, 0, kWidth - 1, kHeight - 1));
  MutablePinState first_state;
  MutablePinState second_state;
  ASSERT_EQ(PresentationPinShowResult::kShown,
            first_raw->showPresentationPin(MakePin(first_state)));
  ASSERT_EQ(PresentationPinShowResult::kShown,
            second_raw->showPresentationPin(MakePin(second_state)));
  root_raw->removeLast();
  EXPECT_EQ(1, first_state.destroyed);
  EXPECT_EQ(1, second_state.destroyed);
}

// Verifies an interrupted frame keeps the pin pending and the following
// unrestricted frame paints it normally.
TEST_F(RooWindowsRenderTest, DeadlineInterruptionRetainsPendingPin) {
  auto panel = std::make_unique<TestPanel>(context(), roo_display::color::Red);
  auto anchor = std::make_unique<PinAnchor>(context());
  PinAnchor* raw = anchor.get();
  panel->add(std::move(anchor), Rect(10, 10, 19, 19));
  app_.add(std::move(panel), Box(0, 0, kWidth - 1, kHeight - 1));
  MutablePinState state;
  ASSERT_EQ(PresentationPinShowResult::kShown,
            raw->showPresentationPin(MakePin(state)));
  EXPECT_FALSE(refresh(roo_time::Uptime::Start()));
  EXPECT_EQ(0, state.painted);
  ASSERT_TRUE(refresh());
  EXPECT_GT(state.painted, 0);
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Green), pixelAt(4, 4));
  raw->hidePresentationPin();
}

// Verifies repeated presentations own one compact active object and leave no
// dormant storage in the anchor class.
TEST_F(RooWindowsRenderTest, RepeatedShowHideUsesActiveOnlyStorage) {
  static_assert(sizeof(PinAnchor) == sizeof(BasicWidget),
                "pin anchors must not gain dormant storage");
  static_assert(
      sizeof(MutablePin) <= sizeof(PresentationPin) + 2 * sizeof(void*),
      "concrete pin payload must remain inline and compact");
  auto anchor = std::make_unique<PinAnchor>(context());
  PinAnchor* raw = anchor.get();
  app_.add(std::move(anchor), Box(0, 0, 31, 31));
  MutablePinState state;
  for (int i = 0; i < 8; ++i) {
    ASSERT_EQ(PresentationPinShowResult::kShown,
              raw->showPresentationPin(MakePin(state)));
    raw->setPresentationPinDirty();
    ASSERT_TRUE(refresh());
    raw->hidePresentationPin();
  }
  EXPECT_EQ(8, state.destroyed);
}

// Verifies window teardown iteratively deletes all active pins before owned
// anchors become unreachable.
TEST(TransientPresentationPin, WindowTeardownDeletesAllPins) {
  roo::byte raster[32 * 32 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      32, 32, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  MutablePinState states[4];
  {
    Application app(&environment, display);
    auto panel =
        std::make_unique<TestPanel>(app.context(), roo_display::color::Red);
    TestPanel* panel_raw = panel.get();
    app.add(std::move(panel), Box(0, 0, 31, 31));
    for (MutablePinState& state : states) {
      auto anchor = std::make_unique<PinAnchor>(app.context());
      PinAnchor* raw = anchor.get();
      panel_raw->add(std::move(anchor), Rect(0, 0, 5, 5));
      ASSERT_EQ(PresentationPinShowResult::kShown,
                raw->showPresentationPin(MakePin(state)));
    }
  }
  for (const MutablePinState& state : states) EXPECT_EQ(1, state.destroyed);
}

}  // namespace
}  // namespace roo_windows
