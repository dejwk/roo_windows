#include "roo_windows/core/transient_surface_host.h"

#include <functional>
#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_testing/system/timer.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/text_input.h"
#include "roo_windows/widgets/text_field.h"
#include "roo_windows_render_test_support.h"

namespace roo_windows {
namespace {

using internal::CaptureTransientSourceGeometry;
using internal::GetTransientSurfaceHost;
using internal::TransientHostLayer;
using internal::TransientSourceGeometry;
using internal::TransientSurfaceHost;
using test_support::QuantizeToArgb4444;

constexpr TransientSurfaceSpec kTransparentReject{
    TransientBarrierPaint::kTransparent,
    TransientAdmissionPolicy::kRejectIfBusy, OutsideInteractionPolicy::kAbsorb,
    TransientPresentationPolicy(true, true), false};

constexpr TransientSurfaceSpec kTransparentReplaceable{
    TransientBarrierPaint::kTransparent,
    TransientAdmissionPolicy::kReplaceReplaceable,
    OutsideInteractionPolicy::kDismiss, TransientPresentationPolicy(true, true),
    true};

constexpr TransientSurfaceSpec kScrimReject{
    TransientBarrierPaint::kScrim, TransientAdmissionPolicy::kRejectIfBusy,
    OutsideInteractionPolicy::kAbsorb, TransientPresentationPolicy(true, true),
    false};

constexpr TransientSurfaceSpec kDismissOutside{
    TransientBarrierPaint::kTransparent,
    TransientAdmissionPolicy::kRejectIfBusy, OutsideInteractionPolicy::kDismiss,
    TransientPresentationPolicy(), false};

constexpr TransientSurfaceSpec kPresenterHandlesOutside{
    TransientBarrierPaint::kTransparent,
    TransientAdmissionPolicy::kRejectIfBusy,
    OutsideInteractionPolicy::kPresenterHandled, TransientPresentationPolicy(),
    false};

class TestPanel : public Panel {
 public:
  explicit TestPanel(ApplicationContext& context) : Panel(context) {}

  using Panel::add;
  using Panel::removeLast;

  Widget* preferredFocusChild() override { return preferred_; }

  Widget* preferred_ = nullptr;
};

class FocusableWidget : public BasicWidget {
 public:
  explicit FocusableWidget(ApplicationContext& context)
      : BasicWidget(context) {}

  bool isFocusable() const override { return true; }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(4, 4);
  }
};

class TestRegistration : public TransientPresentationRegistration {
 public:
  int detach_count = 0;
  int finish_count = 0;
  PresentationFinishReason detach_reason = PresentationFinishReason::kAction;
  std::function<void()> completion;
  std::function<void()> outside;
  int outside_count = 0;

 protected:
  void detachPresentation(PresentationFinishReason reason) override {
    ++detach_count;
    detach_reason = reason;
  }

  void onFinished(PresentationFinishReason) override {
    ++finish_count;
    if (completion != nullptr) completion();
  }

  void onOutsideInteraction() override {
    ++outside_count;
    if (outside != nullptr) outside();
  }
};

class QueuedKeySource : public KeySource {
 public:
  void push(KeyEvent event) {
    events_.push_back(event);
    notifyReady();
  }

  int drain(KeyEvent* out, int max_events) override {
    int count = 0;
    while (count < max_events && next_ < events_.size()) {
      out[count++] = events_[next_++];
    }
    return count;
  }

 private:
  bool hasPendingEvents() const override { return next_ < events_.size(); }

  std::vector<KeyEvent> events_;
  size_t next_ = 0;
};

class KeyRecordingWidget : public FocusableWidget {
 public:
  using FocusableWidget::FocusableWidget;

  bool onKeyEvent(const KeyEvent& event) override {
    ++key_count;
    last_key = event.code;
    return consume;
  }

  bool isClickable() const override { return clickable; }

  ClickActivationPolicy getClickActivationPolicy() const override {
    return ClickActivationPolicy::kAfterRefreshNoAnimation;
  }

  bool clickable = false;
  bool consume = true;
  int key_count = 0;
  KeyCode last_key = KeyCode::kUnknown;
};

class BackRegistration : public TestRegistration {
 public:
  BackResult back_result = BackResult::kHandled;
  int back_count = 0;

 protected:
  BackResult onBackRequested(BackSource source) override {
    (void)source;
    ++back_count;
    if (back_result == BackResult::kHandled) {
      finish(PresentationFinishReason::kBack);
    }
    return back_result;
  }
};

class SelfDeletingRegistration : public TestRegistration {
 public:
  explicit SelfDeletingRegistration(bool& deleted) : deleted_(deleted) {}

  ~SelfDeletingRegistration() override { deleted_ = true; }

 protected:
  void onOutsideInteraction() override {
    ++outside_count;
    delete this;
  }

 private:
  bool& deleted_;
};

class ManualTouchDevice : public roo_display::TouchDevice {
 public:
  ManualTouchDevice(int16_t width, int16_t height)
      : width_(width), height_(height) {}

  void set(bool down, int16_t x, int16_t y) {
    down_ = down;
    x_ = x;
    y_ = y;
  }

  roo_display::TouchResult getTouch(roo_display::TouchPoint* points,
                                    int max_points) override {
    roo_time::Uptime timestamp = roo_time::Uptime::Now();
    if (!down_ || max_points <= 0) {
      return roo_display::TouchResult(timestamp, 0);
    }
    points[0].id = 0;
    points[0].x = ScaleToRaw(x_, width_);
    points[0].y = ScaleToRaw(y_, height_);
    points[0].z = 100;
    points[0].vx = 0;
    points[0].vy = 0;
    return roo_display::TouchResult(timestamp, 1);
  }

 private:
  static int16_t ScaleToRaw(int16_t value, int16_t extent) {
    return extent <= 1 ? 0
                       : static_cast<int16_t>((4095LL * value) / (extent - 1));
  }

  int16_t width_;
  int16_t height_;
  bool down_ = false;
  int16_t x_ = 0;
  int16_t y_ = 0;
};

class GestureSpyWidget : public BasicWidget {
 public:
  explicit GestureSpyWidget(ApplicationContext& context)
      : BasicWidget(context) {}

  bool supportsTap() const override { return tap_enabled; }

  DragAxis dragAxis() const override {
    return drag_enabled ? DragAxis::kHorizontal : DragAxis::kNone;
  }

  DragClaim onDragClaim(XDim x, YDim y, XDim dx, YDim dy) override {
    (void)x;
    (void)y;
    (void)dx;
    (void)dy;
    return DragClaim::kAccept;
  }

  void onDragStart(XDim x, YDim y) override {
    (void)x;
    (void)y;
    ++drag_start_count;
  }

  void onSingleTapUp(XDim x, YDim y) override {
    (void)x;
    (void)y;
    ++tap_count;
    if (cancel_on_tap != nullptr) cancel_on_tap->cancelForDisplayCoverage();
  }

  void onCancel() override {
    ++cancel_count;
    BasicWidget::onCancel();
    if (cancellation != nullptr) cancellation();
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(8, 8);
  }

  GestureDetector* cancel_on_tap = nullptr;
  std::function<void()> cancellation;
  bool tap_enabled = false;
  bool drag_enabled = false;
  int tap_count = 0;
  int drag_start_count = 0;
  int cancel_count = 0;
};

class HostTest : public ::testing::Test {
 protected:
  HostTest()
      : device_(64, 48, raster_, roo_display::Argb4444()),
        display_(device_),
        environment_(scheduler_),
        app_(&environment_, display_),
        task_content_(app_.context()),
        owner_(app_.addTaskFullScreen(task_content_)),
        host_(GetTransientSurfaceHost(owner_)) {}

  roo_display::Color pixelAt(int16_t x, int16_t y) const {
    int16_t px[] = {x};
    int16_t py[] = {y};
    roo_display::Color color[1];
    device_.raster().readColors(px, py, 1, color);
    return color[0];
  }

  void queueOutsideTap() {
    std::vector<Widget*> path;
    ASSERT_TRUE(app_.root().fillTouchTargetPath(60, 40, path));
    ASSERT_GE(path.size(), 2u);
    path.back()->onSingleTapUp(60, 40);
  }

  void dispatchInitialApplicationTick() {
    app_.start();
    scheduler_.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum,
                                           1);
  }

  roo::byte raster_[64 * 48 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment environment_;
  // Declared before Application so dynamically-created task content outlives
  // the Task that borrows it during Application teardown.
  std::unique_ptr<TestPanel> auxiliary_task_content_;
  Application app_;
  TestPanel task_content_;
  Task& owner_;
  TransientSurfaceHost& host_;
};

// Verifies admission attaches one borrowed root through the explicit task,
// activates its focus scope, blocks lower hit testing, and fully detaches.
TEST_F(HostTest, AttachesOwnerBoundRootAndRestoresFocus) {
  FocusableWidget base_focus(app_.context());
  task_content_.add(WidgetRef(base_focus), Rect(0, 0, 7, 7));
  ASSERT_TRUE(app_.refresh());
  ASSERT_TRUE(base_focus.requestFocus());

  TestPanel root(app_.context());
  FocusableWidget presenter_focus(app_.context());
  root.add(WidgetRef(presenter_focus), Rect(0, 0, 7, 7));
  root.preferred_ = &presenter_focus;
  FocusScope scope;
  TestRegistration registration;

  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(registration, owner_, root, Rect(8, 8, 31, 31), scope,
                       kTransparentReject));
  EXPECT_NE(nullptr, root.parent());
  EXPECT_EQ(&owner_, root.getTask());
  EXPECT_EQ(&root, owner_.focus().scopeRoot());
  EXPECT_EQ(&presenter_focus, owner_.focus().focused());

  std::vector<Widget*> path;
  ASSERT_TRUE(app_.root().fillTouchTargetPath(10, 10, path));
  EXPECT_EQ(&presenter_focus, path.back());
  path.clear();
  ASSERT_TRUE(app_.root().fillTouchTargetPath(60, 40, path));
  EXPECT_NE(&task_content_, path.back());

  registration.finish(PresentationFinishReason::kCancel);
  EXPECT_EQ(nullptr, root.parent());
  EXPECT_EQ(&base_focus, owner_.focus().focused());
  EXPECT_FALSE(
      app_.root().transient_presentation_slot().hasActivePresentation());
  EXPECT_EQ(1, registration.detach_count);
}

// Verifies empty, outside, and invalid-policy surfaces fail without mutating
// the incoming registration, root, or scope; intersecting bounds are accepted.
TEST_F(HostTest, ValidatesBoundsAndCompletePolicyBeforeMutation) {
  TestPanel root(app_.context());
  FocusScope scope;
  TestRegistration registration;

  EXPECT_EQ(PresentationStartResult::kSurfaceUnavailable,
            host_.show(registration, owner_, root, Rect(0, 0, -1, -1), scope,
                       kTransparentReject));
  EXPECT_EQ(PresentationStartResult::kSurfaceUnavailable,
            host_.show(registration, owner_, root, Rect(70, 0, 80, 10), scope,
                       kTransparentReject));
  TransientSurfaceSpec invalid = kTransparentReject;
  invalid.outside = static_cast<OutsideInteractionPolicy>(0xff);
  EXPECT_EQ(PresentationStartResult::kSurfaceUnavailable,
            host_.show(registration, owner_, root, Rect(0, 0, 10, 10), scope,
                       invalid));
  EXPECT_EQ(nullptr, root.parent());
  EXPECT_EQ(nullptr, scope.root);
  EXPECT_FALSE(registration.isActive());

  EXPECT_EQ(PresentationStartResult::kStarted,
            host_.show(registration, owner_, root, Rect(-8, 8, 8, 20), scope,
                       kTransparentReject));
  registration.finish(PresentationFinishReason::kCancel);
}

// Verifies both sides must opt into replacement and direct slot replacement
// cannot bypass the host's stored occupant policy.
TEST_F(HostTest, ReplacementRequiresIncomingAndOccupantPermission) {
  TestPanel first_root(app_.context());
  TestPanel second_root(app_.context());
  FocusScope first_scope;
  FocusScope second_scope;
  TestRegistration first;
  TestRegistration second;

  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(first, owner_, first_root, Rect(0, 0, 10, 10),
                       first_scope, kTransparentReject));
  EXPECT_EQ(PresentationStartResult::kHostBusy,
            host_.show(second, owner_, second_root, Rect(0, 0, 10, 10),
                       second_scope, kTransparentReject));
  EXPECT_EQ(PresentationStartResult::kHostBusy,
            host_.show(second, owner_, second_root, Rect(0, 0, 10, 10),
                       second_scope, kTransparentReplaceable));
  EXPECT_EQ(PresentationStartResult::kHostBusy,
            app_.root().transient_presentation_slot().replace(second));
  EXPECT_TRUE(first.isActive());
  first.finish(PresentationFinishReason::kCancel);

  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(first, owner_, first_root, Rect(0, 0, 10, 10),
                       first_scope, kTransparentReplaceable));
  EXPECT_EQ(PresentationStartResult::kHostBusy,
            app_.root().transient_presentation_slot().replace(second));
  EXPECT_TRUE(first.isActive());
  EXPECT_EQ(PresentationStartResult::kHostBusy,
            host_.show(second, owner_, second_root, Rect(0, 0, 10, 10),
                       second_scope, kTransparentReject));
  EXPECT_EQ(PresentationStartResult::kStarted,
            host_.show(second, owner_, second_root, Rect(0, 0, 10, 10),
                       second_scope, kTransparentReplaceable));
  EXPECT_EQ(PresentationFinishReason::kReplacement, first.detach_reason);
  EXPECT_TRUE(second.isActive());
  second.finish(PresentationFinishReason::kCancel);
}

// Verifies replacement completion may fill the canonical slot and the outer
// request preserves that reentrant presentation.
TEST_F(HostTest, ReplacementDetectsReentrantHostedAdmission) {
  TestPanel first_root(app_.context());
  TestPanel requested_root(app_.context());
  TestPanel reopened_root(app_.context());
  FocusScope first_scope;
  FocusScope requested_scope;
  FocusScope reopened_scope;
  TestRegistration first;
  TestRegistration requested;
  TestRegistration reopened;
  PresentationStartResult reopened_result = PresentationStartResult::kHostBusy;
  first.completion = [&]() {
    reopened_result =
        host_.show(reopened, owner_, reopened_root, Rect(4, 4, 12, 12),
                   reopened_scope, kTransparentReplaceable);
  };

  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(first, owner_, first_root, Rect(0, 0, 10, 10),
                       first_scope, kTransparentReplaceable));
  EXPECT_EQ(PresentationStartResult::kReentrantReplacement,
            host_.show(requested, owner_, requested_root, Rect(0, 0, 10, 10),
                       requested_scope, kTransparentReplaceable));
  EXPECT_EQ(PresentationStartResult::kStarted, reopened_result);
  EXPECT_TRUE(reopened.isActive());
  EXPECT_FALSE(requested.isActive());
  reopened.finish(PresentationFinishReason::kCancel);
}

// Verifies repeated preflight observes callback-side root attachment after the
// outgoing presentation has irreversibly finished and leaves the slot empty.
TEST_F(HostTest, RepeatedPreflightRejectsMutatedIncomingSurface) {
  TestPanel first_root(app_.context());
  TestPanel incoming_root(app_.context());
  FocusScope first_scope;
  FocusScope incoming_scope;
  TestRegistration first;
  TestRegistration incoming;
  first.completion = [&]() {
    task_content_.add(WidgetRef(incoming_root), Rect(0, 0, 10, 10));
  };

  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(first, owner_, first_root, Rect(0, 0, 10, 10),
                       first_scope, kTransparentReplaceable));
  EXPECT_EQ(PresentationStartResult::kSurfaceUnavailable,
            host_.show(incoming, owner_, incoming_root, Rect(0, 0, 10, 10),
                       incoming_scope, kTransparentReplaceable));
  EXPECT_FALSE(first.isActive());
  EXPECT_FALSE(incoming.isActive());
  EXPECT_FALSE(
      app_.root().transient_presentation_slot().hasActivePresentation());
  EXPECT_NE(nullptr, incoming_root.parent());
  task_content_.removeLast();
}

// Verifies a standalone slot participant remains replaceable by the standalone
// API but is never replaced through hosted admission.
TEST_F(HostTest, HostedAdmissionDoesNotReplaceStandaloneParticipant) {
  TestRegistration standalone;
  TestRegistration standalone_replacement;
  ASSERT_EQ(PresentationStartResult::kStarted,
            app_.root().transient_presentation_slot().show(standalone));

  TestPanel root(app_.context());
  FocusScope scope;
  TestRegistration hosted;
  EXPECT_EQ(PresentationStartResult::kHostBusy,
            host_.show(hosted, owner_, root, Rect(0, 0, 10, 10), scope,
                       kTransparentReplaceable));
  EXPECT_EQ(PresentationStartResult::kStarted,
            app_.root().transient_presentation_slot().replace(
                standalone_replacement));
  standalone_replacement.finish(PresentationFinishReason::kCancel);
}

// Verifies source capture accepts only live geometry physically below the
// exact interaction-owner task panel and never mutates output on failure.
TEST_F(HostTest, CapturesOnlyOwnerTaskSourceGeometry) {
  FocusableWidget source(app_.context());
  task_content_.add(WidgetRef(source), Rect(4, 5, 11, 14));
  ASSERT_TRUE(app_.refresh());
  TransientSourceGeometry output{Rect(90, 91, 92, 93)};

  ASSERT_TRUE(CaptureTransientSourceGeometry(owner_, source, output));
  EXPECT_EQ(Rect(4, 5, 11, 14), output.bounds_in_window);

  auxiliary_task_content_ = std::make_unique<TestPanel>(app_.context());
  Task& other = app_.addTaskFullScreen(*auxiliary_task_content_);
  TransientSourceGeometry unchanged{Rect(1, 2, 3, 4)};
  EXPECT_FALSE(CaptureTransientSourceGeometry(other, source, unchanged));
  EXPECT_EQ(Rect(1, 2, 3, 4), unchanged.bounds_in_window);

  source.setVisibility(Visibility::kInvisible);
  EXPECT_FALSE(CaptureTransientSourceGeometry(owner_, source, unchanged));
  EXPECT_EQ(Rect(1, 2, 3, 4), unchanged.bounds_in_window);
  source.setVisibility(Visibility::kVisible);
  source.layout(Rect(0, 0, -1, -1));
  EXPECT_FALSE(CaptureTransientSourceGeometry(owner_, source, unchanged));
  EXPECT_EQ(Rect(1, 2, 3, 4), unchanged.bounds_in_window);
}

// Verifies detached mini-trees and both display-attached and task-nested host
// ancestors are rejected without relying on getTask() resolution.
TEST_F(HostTest, RejectsDetachedAndHostedSourceChains) {
  TestPanel detached_root(app_.context());
  FocusableWidget detached_source(app_.context());
  detached_root.add(WidgetRef(detached_source), Rect(0, 0, 4, 4));
  TransientSourceGeometry output{Rect(1, 1, 2, 2)};
  EXPECT_FALSE(CaptureTransientSourceGeometry(owner_, detached_source, output));

  TransientHostLayer nested_host(app_.context());
  task_content_.add(WidgetRef(nested_host), Rect(0, 0, 8, 8));
  ASSERT_TRUE(app_.refresh());
  EXPECT_FALSE(CaptureTransientSourceGeometry(owner_, nested_host, output));
  task_content_.removeLast();

  TestPanel root(app_.context());
  FocusableWidget hosted_source(app_.context());
  root.add(WidgetRef(hosted_source), Rect(0, 0, 4, 4));
  FocusScope scope;
  TestRegistration registration;
  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(registration, owner_, root, Rect(0, 0, 10, 10), scope,
                       kTransparentReject));
  EXPECT_EQ(&owner_, hosted_source.getTask());
  EXPECT_FALSE(CaptureTransientSourceGeometry(owner_, hosted_source, output));
  registration.finish(PresentationFinishReason::kCancel);
}

// Verifies registration destruction skips virtual completion but still exits
// focus and detaches every borrowed structural child.
TEST_F(HostTest, RegistrationDestructionDetachesHostedStructure) {
  TestPanel root(app_.context());
  FocusScope scope;
  {
    TestRegistration registration;
    ASSERT_EQ(PresentationStartResult::kStarted,
              host_.show(registration, owner_, root, Rect(0, 0, 10, 10), scope,
                         kTransparentReject));
  }

  EXPECT_EQ(nullptr, root.parent());
  EXPECT_EQ(nullptr, scope.root);
  EXPECT_FALSE(
      app_.root().transient_presentation_slot().hasActivePresentation());
}

// Verifies a transparent host preserves lower paint outside the borrowed root
// and teardown reveals the lower task again.
TEST_F(HostTest, TransparentBarrierPreservesUnderlyingPaint) {
  test_support::ColorBoxWidget base(app_.context(), roo_display::color::Red,
                                    Dimensions(64, 48));
  task_content_.add(WidgetRef(base), Rect(0, 0, 63, 47));
  ASSERT_TRUE(app_.refresh());
  test_support::ColorBoxWidget root(app_.context(), roo_display::color::Blue,
                                    Dimensions(16, 16));
  FocusScope scope;
  TestRegistration registration;
  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(registration, owner_, root, Rect(16, 12, 31, 27), scope,
                       kTransparentReject));
  ASSERT_TRUE(app_.refresh());

  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Red), pixelAt(4, 4));
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Blue), pixelAt(20, 16));
  registration.finish(PresentationFinishReason::kCancel);
  ASSERT_TRUE(app_.refresh());
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Red), pixelAt(20, 16));
  task_content_.removeLast();
}

// Verifies scrim paint is independent from the borrowed root and visibly
// modifies otherwise exposed lower content.
TEST_F(HostTest, ScrimBarrierOverlaysUnderlyingPaint) {
  test_support::ColorBoxWidget base(app_.context(), roo_display::color::Red,
                                    Dimensions(64, 48));
  task_content_.add(WidgetRef(base), Rect(0, 0, 63, 47));
  ASSERT_TRUE(app_.refresh());
  test_support::ColorBoxWidget root(app_.context(), roo_display::color::Blue,
                                    Dimensions(16, 16));
  FocusScope scope;
  TestRegistration registration;
  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(registration, owner_, root, Rect(16, 12, 31, 27), scope,
                       kScrimReject));
  ASSERT_TRUE(app_.refresh());

  EXPECT_NE(QuantizeToArgb4444(roo_display::color::Red), pixelAt(4, 4));
  EXPECT_EQ(QuantizeToArgb4444(roo_display::color::Blue), pixelAt(20, 16));
  registration.finish(PresentationFinishReason::kCancel);
  task_content_.removeLast();
}

// Verifies an absorbed outside activation is deferred through the application
// tick and leaves the active registration and borrowed root unchanged.
TEST_F(HostTest, AbsorbsCompletedOutsideActivation) {
  TestPanel root(app_.context());
  FocusScope scope;
  TestRegistration registration;
  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(registration, owner_, root, Rect(0, 0, 10, 10), scope,
                       kTransparentReject));

  queueOutsideTap();
  EXPECT_TRUE(registration.isActive());
  dispatchInitialApplicationTick();

  EXPECT_TRUE(registration.isActive());
  EXPECT_EQ(0, registration.outside_count);
  registration.finish(PresentationFinishReason::kCancel);
}

// Verifies dismiss policy finishes only after terminal pointer dispatch has
// unwound and reports the dedicated outside-interaction reason.
TEST_F(HostTest, DismissesAfterCompletedOutsideActivation) {
  TestPanel root(app_.context());
  FocusScope scope;
  TestRegistration registration;
  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(registration, owner_, root, Rect(0, 0, 10, 10), scope,
                       kDismissOutside));

  queueOutsideTap();
  EXPECT_TRUE(registration.isActive());
  dispatchInitialApplicationTick();

  EXPECT_FALSE(registration.isActive());
  EXPECT_EQ(PresentationFinishReason::kOutsideInteraction,
            registration.detach_reason);
  EXPECT_EQ(nullptr, root.parent());
}

// Verifies presenter-handled outside activation may veto dismissal or finish
// synchronously, and the host does not impose either behavior.
TEST_F(HostTest, PresenterControlsOutsideActivation) {
  TestPanel root(app_.context());
  FocusScope scope;
  TestRegistration registration;
  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(registration, owner_, root, Rect(0, 0, 10, 10), scope,
                       kPresenterHandlesOutside));
  queueOutsideTap();
  dispatchInitialApplicationTick();
  EXPECT_EQ(1, registration.outside_count);
  EXPECT_TRUE(registration.isActive());
  registration.finish(PresentationFinishReason::kCancel);
}

// Verifies a presenter-handled outside hook may synchronously finish using its
// own semantic reason while the host performs no post-hook presenter access.
TEST_F(HostTest, PresenterHandledOutsideMayFinish) {
  TestPanel root(app_.context());
  FocusScope scope;
  TestRegistration registration;
  registration.outside = [&]() {
    registration.finish(PresentationFinishReason::kAction);
  };
  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(registration, owner_, root, Rect(0, 0, 10, 10), scope,
                       kPresenterHandlesOutside));
  queueOutsideTap();
  dispatchInitialApplicationTick();

  EXPECT_EQ(1, registration.outside_count);
  EXPECT_FALSE(registration.isActive());
  EXPECT_EQ(PresentationFinishReason::kAction, registration.detach_reason);
  EXPECT_EQ(nullptr, root.parent());
}

// Verifies a presenter may destroy itself from its outside hook; registration
// cancellation performs structural cleanup without a second presenter access.
TEST_F(HostTest, OutsideHandlerMayDestroyPresenter) {
  TestPanel root(app_.context());
  FocusScope scope;
  bool deleted = false;
  auto* registration = new SelfDeletingRegistration(deleted);
  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(*registration, owner_, root, Rect(0, 0, 10, 10), scope,
                       kPresenterHandlesOutside));
  queueOutsideTap();
  dispatchInitialApplicationTick();

  EXPECT_TRUE(deleted);
  EXPECT_EQ(nullptr, root.parent());
  EXPECT_EQ(nullptr, scope.root);
}

// Verifies display coverage cancels incomplete Enter/Space activation in both
// owner and non-owner tasks before the presenter becomes input-eligible.
TEST_F(HostTest, AdmissionCancelsArmedKeysInEveryTask) {
  KeyRecordingWidget owner_control(app_.context());
  owner_control.clickable = true;
  owner_control.consume = false;
  task_content_.add(WidgetRef(owner_control), Rect(0, 0, 8, 8));
  auxiliary_task_content_ = std::make_unique<TestPanel>(app_.context());
  KeyRecordingWidget other_control(app_.context());
  other_control.clickable = true;
  other_control.consume = false;
  auxiliary_task_content_->add(WidgetRef(other_control), Rect(0, 0, 8, 8));
  Task& other = app_.addTaskFullScreen(*auxiliary_task_content_);
  QueuedKeySource owner_keys;
  QueuedKeySource other_keys;
  owner_keys.connect(owner_);
  other_keys.connect(other);
  ASSERT_TRUE(app_.refresh());
  ASSERT_TRUE(owner_control.requestFocus());
  ASSERT_TRUE(other_control.requestFocus());
  owner_keys.push(
      {KeyPhase::kDown, KeyCode::kEnter, 0, PhysicalKey::kEnter, 0});
  other_keys.push(
      {KeyPhase::kDown, KeyCode::kSpace, 0, PhysicalKey::kSpace, 0});
  dispatchInitialApplicationTick();
  ASSERT_TRUE(owner_control.isPressed());
  ASSERT_TRUE(other_control.isPressed());

  TestPanel root(app_.context());
  FocusScope scope;
  TestRegistration registration;
  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(registration, owner_, root, Rect(0, 0, 10, 10), scope,
                       kTransparentReject));
  EXPECT_FALSE(owner_control.isPressed());
  EXPECT_FALSE(other_control.isPressed());
  registration.finish(PresentationFinishReason::kCancel);
  auxiliary_task_content_->removeLast();
  task_content_.removeLast();
}

// Verifies ordinary physical keys reach only the owner presenter scope while
// retained focus in owner and non-owner task content receives nothing.
TEST_F(HostTest, HostedSurfaceIsolatesOwnerAndNonOwnerKeys) {
  KeyRecordingWidget owner_base(app_.context());
  task_content_.add(WidgetRef(owner_base), Rect(0, 0, 8, 8));
  auxiliary_task_content_ = std::make_unique<TestPanel>(app_.context());
  KeyRecordingWidget other_focus(app_.context());
  auxiliary_task_content_->add(WidgetRef(other_focus), Rect(0, 0, 8, 8));
  Task& other = app_.addTaskFullScreen(*auxiliary_task_content_);
  QueuedKeySource owner_keys;
  QueuedKeySource other_keys;
  owner_keys.connect(owner_);
  other_keys.connect(other);
  ASSERT_TRUE(app_.refresh());
  ASSERT_TRUE(owner_base.requestFocus());
  ASSERT_TRUE(other_focus.requestFocus());

  TestPanel root(app_.context());
  KeyRecordingWidget presenter_focus(app_.context());
  root.add(WidgetRef(presenter_focus), Rect(0, 0, 8, 8));
  root.preferred_ = &presenter_focus;
  FocusScope scope;
  TestRegistration registration;
  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(registration, owner_, root, Rect(0, 0, 10, 10), scope,
                       kTransparentReject));
  owner_keys.push(
      {KeyPhase::kDown, KeyCode::kCharacter, 0, PhysicalKey::kA, U'A'});
  other_keys.push(
      {KeyPhase::kDown, KeyCode::kCharacter, 0, PhysicalKey::kB, U'B'});
  dispatchInitialApplicationTick();

  EXPECT_EQ(1, presenter_focus.key_count);
  EXPECT_EQ(0, owner_base.key_count);
  EXPECT_EQ(0, other_focus.key_count);
  registration.finish(PresentationFinishReason::kCancel);
  auxiliary_task_content_->removeLast();
  task_content_.removeLast();
}

// Verifies an eligible physical Back request reaches the hosted registration
// before focused widgets even when the key originates in a non-owner task.
TEST_F(HostTest, HostedBackHasDisplayWidePrecedence) {
  auxiliary_task_content_ = std::make_unique<TestPanel>(app_.context());
  KeyRecordingWidget other_focus(app_.context());
  auxiliary_task_content_->add(WidgetRef(other_focus), Rect(0, 0, 8, 8));
  Task& other = app_.addTaskFullScreen(*auxiliary_task_content_);
  QueuedKeySource other_keys;
  other_keys.connect(other);
  ASSERT_TRUE(app_.refresh());
  ASSERT_TRUE(other_focus.requestFocus());

  TestPanel root(app_.context());
  FocusScope scope;
  BackRegistration registration;
  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(registration, owner_, root, Rect(0, 0, 10, 10), scope,
                       kTransparentReject));
  other_keys.push(
      {KeyPhase::kDown, KeyCode::kEscape, 0, PhysicalKey::kEscape, 0});
  dispatchInitialApplicationTick();

  EXPECT_EQ(1, registration.back_count);
  EXPECT_FALSE(registration.isActive());
  EXPECT_EQ(0, other_focus.key_count);
  auxiliary_task_content_->removeLast();
}

// Verifies a declining hosted Back hook is not called twice; owner focus may
// inspect the key, but task-local Back fallback remains covered.
TEST_F(HostTest, DeclinedHostedBackDoesNotReachTaskFallback) {
  QueuedKeySource owner_keys;
  owner_keys.connect(owner_);
  TestPanel root(app_.context());
  KeyRecordingWidget presenter_focus(app_.context());
  presenter_focus.consume = false;
  root.add(WidgetRef(presenter_focus), Rect(0, 0, 8, 8));
  root.preferred_ = &presenter_focus;
  FocusScope scope;
  BackRegistration registration;
  registration.back_result = BackResult::kUnhandled;
  int task_back_count = 0;
  owner_.setBackCallback([&](BackSource) {
    ++task_back_count;
    return BackResult::kHandled;
  });
  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(registration, owner_, root, Rect(0, 0, 10, 10), scope,
                       kTransparentReject));
  owner_keys.push(
      {KeyPhase::kDown, KeyCode::kBack, 0, PhysicalKey::kEscape, 0});
  dispatchInitialApplicationTick();

  EXPECT_EQ(1, registration.back_count);
  EXPECT_EQ(1, presenter_focus.key_count);
  EXPECT_EQ(0, task_back_count);
  registration.finish(PresentationFinishReason::kCancel);
}

// Verifies semantic input requires both the interaction-owner editor and a
// live editor target physically inside the hosted root.
TEST_F(HostTest, SemanticTextInputIsContainedByHostedRoot) {
  TextField owner_field(app_.context(), font_body1(), "", roo_display::kLeft,
                        TextField::NONE);
  task_content_.add(WidgetRef(owner_field), Rect(0, 0, 30, 12));
  auxiliary_task_content_ = std::make_unique<TestPanel>(app_.context());
  TextField other_field(app_.context(), font_body1(), "", roo_display::kLeft,
                        TextField::NONE);
  auxiliary_task_content_->add(WidgetRef(other_field), Rect(0, 0, 30, 12));
  app_.addTaskFullScreen(*auxiliary_task_content_);
  TestPanel root(app_.context());
  TextField presenter_field(app_.context(), font_body1(), "",
                            roo_display::kLeft, TextField::NONE);
  root.add(WidgetRef(presenter_field), Rect(0, 0, 30, 12));
  root.preferred_ = &presenter_field;
  FocusScope scope;
  TestRegistration registration;
  ASSERT_TRUE(app_.refresh());
  ASSERT_EQ(PresentationStartResult::kStarted,
            host_.show(registration, owner_, root, Rect(0, 0, 31, 15), scope,
                       kTransparentReject));
  TextInputEmitter emitter;
  emitter.connect(app_);

  other_field.edit();
  EXPECT_FALSE(emitter.commitRune(U'N'));
  owner_field.edit();
  EXPECT_FALSE(emitter.commitRune(U'B'));
  presenter_field.edit();
  EXPECT_TRUE(emitter.commitRune(U'P'));
  EXPECT_EQ("", other_field.content());
  EXPECT_EQ("", owner_field.content());
  EXPECT_EQ("P", presenter_field.content());

  registration.finish(PresentationFinishReason::kCancel);
  auxiliary_task_content_->removeLast();
  task_content_.removeLast();
}

// Verifies window shutdown closes admission before completion and detaches the
// borrowed host structure while its owner task is still alive.
TEST(TransientSurfaceHost, WindowShutdownRejectsReentrantAdmission) {
  roo::byte raster[16 * 16 * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      16, 16, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  TestRegistration active;
  TestRegistration reopened;
  auto* content = static_cast<TestPanel*>(nullptr);
  auto* active_root = static_cast<TestPanel*>(nullptr);
  auto* reopened_root = static_cast<TestPanel*>(nullptr);
  auto* active_scope = static_cast<FocusScope*>(nullptr);
  auto* reopened_scope = static_cast<FocusScope*>(nullptr);
  PresentationStartResult reopened_result = PresentationStartResult::kStarted;

  {
    Application app(&environment, display);
    content = new TestPanel(app.context());
    Task& owner = app.addTaskFullScreen(*content);
    active_root = new TestPanel(app.context());
    reopened_root = new TestPanel(app.context());
    active_scope = new FocusScope();
    reopened_scope = new FocusScope();
    TransientSurfaceHost& host = GetTransientSurfaceHost(owner);
    active.completion = [&]() {
      reopened_result =
          host.show(reopened, owner, *reopened_root, Rect(0, 0, 8, 8),
                    *reopened_scope, kTransparentReject);
    };
    ASSERT_EQ(PresentationStartResult::kStarted,
              host.show(active, owner, *active_root, Rect(0, 0, 8, 8),
                        *active_scope, kTransparentReject));
  }

  EXPECT_EQ(PresentationFinishReason::kHostDestroyed, active.detach_reason);
  EXPECT_EQ(1, active.finish_count);
  EXPECT_EQ(PresentationStartResult::kHostBusy, reopened_result);
  EXPECT_EQ(nullptr, active_root->parent());
  EXPECT_FALSE(reopened.isActive());
  delete reopened_scope;
  delete active_scope;
  delete reopened_root;
  delete active_root;
  delete content;
}

// Verifies a host with no display area rejects even otherwise valid borrowed
// state without attaching the root or activating its scope.
TEST(TransientSurfaceHost, EmptyWindowRejectsSurface) {
  roo::byte raster[1] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      0, 0, raster, roo_display::Argb4444());
  roo_display::Display display(device);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  TestRegistration registration;
  FocusScope scope;
  auto* content = static_cast<TestPanel*>(nullptr);
  auto* root = static_cast<TestPanel*>(nullptr);

  {
    Application app(&environment, display);
    content = new TestPanel(app.context());
    Task& owner = app.addTaskFullScreen(*content);
    root = new TestPanel(app.context());
    EXPECT_EQ(PresentationStartResult::kSurfaceUnavailable,
              GetTransientSurfaceHost(owner).show(registration, owner, *root,
                                                  Rect(0, 0, 4, 4), scope,
                                                  kTransparentReject));
    EXPECT_EQ(nullptr, root->parent());
    EXPECT_EQ(nullptr, scope.root);
  }

  delete root;
  delete content;
}

// Verifies display-coverage cancellation terminates a retained lower drag
// exactly once and clears the detector's target.
TEST(GestureDetector, DisplayCoverageCancelsRetainedDrag) {
  constexpr int16_t kWidth = 64;
  constexpr int16_t kHeight = 48;
  roo::byte raster[kWidth * kHeight * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      kWidth, kHeight, raster, roo_display::Argb4444());
  ManualTouchDevice touch(kWidth, kHeight);
  roo_display::Display display(device, touch);
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestPanel root(context);
  GestureSpyWidget target(context);
  target.drag_enabled = true;
  root.add(WidgetRef(target), Rect(0, 0, 63, 47));
  root.layout(Rect(0, 0, 63, 47));
  TouchSensor sensor(display);
  GestureDetector detector(root, sensor);

  touch.set(true, 4, 4);
  sensor.pollOnce();
  detector.tick();
  touch.set(true, 30, 4);
  sensor.pollOnce();
  detector.tick();
  ASSERT_EQ(1, target.drag_start_count);

  detector.cancelForDisplayCoverage();
  EXPECT_EQ(1, target.cancel_count);
  EXPECT_EQ(nullptr, detector.currentGestureTarget());
  detector.cancelForDisplayCoverage();
  EXPECT_EQ(1, target.cancel_count);
  root.removeLast();
}

// Verifies a host opened from successful UP completion clears that terminal
// stream without also canceling the widget whose tap already succeeded.
TEST(GestureDetector, DisplayCoverageDoesNotCancelSuccessfulTerminalTap) {
  constexpr int16_t kWidth = 64;
  constexpr int16_t kHeight = 48;
  roo::byte raster[kWidth * kHeight * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      kWidth, kHeight, raster, roo_display::Argb4444());
  ManualTouchDevice touch(kWidth, kHeight);
  roo_display::Display display(device, touch);
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestPanel root(context);
  GestureSpyWidget target(context);
  target.tap_enabled = true;
  root.add(WidgetRef(target), Rect(0, 0, 63, 47));
  root.layout(Rect(0, 0, 63, 47));
  TouchSensor sensor(display);
  GestureDetector detector(root, sensor);
  target.cancel_on_tap = &detector;

  touch.set(true, 4, 4);
  sensor.pollOnce();
  detector.tick();
  touch.set(false, 4, 4);
  sensor.pollOnce();
  detector.tick();

  EXPECT_EQ(1, target.tap_count);
  EXPECT_EQ(0, target.cancel_count);
  EXPECT_EQ(nullptr, detector.currentGestureTarget());
  root.removeLast();
}

// Verifies generic pre-detach cleanup removes only subtree-retained gesture
// roles and does not deliver a second cancellation during later shutdown.
TEST(GestureDetector, SubtreeCleanupCancelsBeforeParentLinksChange) {
  constexpr int16_t kWidth = 64;
  constexpr int16_t kHeight = 48;
  roo::byte raster[kWidth * kHeight * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      kWidth, kHeight, raster, roo_display::Argb4444());
  ManualTouchDevice touch(kWidth, kHeight);
  roo_display::Display display(device, touch);
  roo_scheduler::Scheduler scheduler;
  ApplicationContext context(scheduler, DefaultTheme(),
                             DefaultKeyboardColorTheme());
  TestPanel root(context);
  GestureSpyWidget target(context);
  target.tap_enabled = true;
  root.add(WidgetRef(target), Rect(0, 0, 63, 47));
  root.layout(Rect(0, 0, 63, 47));
  TouchSensor sensor(display);
  GestureDetector detector(root, sensor);

  touch.set(true, 4, 4);
  sensor.pollOnce();
  detector.tick();
  detector.cancelTargetsInSubtree(target);
  EXPECT_EQ(1, target.cancel_count);
  EXPECT_EQ(nullptr, detector.currentGestureTarget());
  root.removeLast();
  detector.cancel();
  EXPECT_EQ(1, target.cancel_count);
}

// Verifies host admission guards gesture-cancellation callbacks and repeats
// complete preflight before committing any incoming state.
TEST(TransientSurfaceHost, CancellationMutationFailsRepeatedPreflight) {
  constexpr int16_t kWidth = 64;
  constexpr int16_t kHeight = 48;
  roo::byte raster[kWidth * kHeight * 2] = {};
  roo_display::OffscreenDevice<roo_display::Argb4444> device(
      kWidth, kHeight, raster, roo_display::Argb4444());
  ManualTouchDevice touch(kWidth, kHeight);
  roo_display::Display display(device, touch);
  roo_scheduler::Scheduler scheduler;
  Environment environment(scheduler);
  TestRegistration registration;
  FocusScope scope;
  auto* content = static_cast<TestPanel*>(nullptr);
  auto* lower_target = static_cast<GestureSpyWidget*>(nullptr);
  auto* incoming_root = static_cast<TestPanel*>(nullptr);

  {
    Application app(&environment, display);
    content = new TestPanel(app.context());
    lower_target = new GestureSpyWidget(app.context());
    lower_target->tap_enabled = true;
    content->add(WidgetRef(*lower_target), Rect(0, 0, 63, 47));
    Task& owner = app.addTaskFullScreen(*content);
    incoming_root = new TestPanel(app.context());
    lower_target->cancellation = [&]() {
      content->add(WidgetRef(*incoming_root), Rect(0, 0, 10, 10));
    };
    ASSERT_TRUE(app.refresh());
    touch.set(true, 4, 4);
    app.start();
    for (int i = 0;
         i < 10 && app.gesture_detector().currentGestureTarget() == nullptr;
         ++i) {
      system_time_delay_micros(5000);
      scheduler.executeEligibleTasksUpToNow(roo_scheduler::Priority::kMinimum,
                                            1);
    }
    ASSERT_EQ(lower_target, app.gesture_detector().currentGestureTarget());

    EXPECT_EQ(PresentationStartResult::kSurfaceUnavailable,
              GetTransientSurfaceHost(owner).show(
                  registration, owner, *incoming_root, Rect(0, 0, 10, 10),
                  scope, kTransparentReject));
    EXPECT_EQ(1, lower_target->cancel_count);
    EXPECT_FALSE(registration.isActive());
    EXPECT_EQ(nullptr, scope.root);
    EXPECT_EQ(content, incoming_root->parent());
    content->removeLast();
    content->removeLast();
  }

  delete incoming_root;
  delete lower_target;
  delete content;
}

}  // namespace
}  // namespace roo_windows
