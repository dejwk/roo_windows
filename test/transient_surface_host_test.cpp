#include "roo_windows/core/transient_surface_host.h"

#include <functional>
#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "roo_display.h"
#include "roo_display/core/offscreen.h"
#include "roo_scheduler.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
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

 protected:
  void detachPresentation(PresentationFinishReason reason) override {
    ++detach_count;
    detach_reason = reason;
  }

  void onFinished(PresentationFinishReason) override {
    ++finish_count;
    if (completion != nullptr) completion();
  }
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
  TransientSourceGeometry output{Rect(90, 91, 92, 93), Rect(94, 95, 96, 97)};

  ASSERT_TRUE(CaptureTransientSourceGeometry(owner_, source, output));
  EXPECT_EQ(Rect(4, 5, 11, 14), output.bounds_in_window);
  EXPECT_EQ(Rect(4, 5, 11, 14), output.visible_bounds_in_window);

  auxiliary_task_content_ = std::make_unique<TestPanel>(app_.context());
  Task& other = app_.addTaskFullScreen(*auxiliary_task_content_);
  TransientSourceGeometry unchanged{Rect(1, 2, 3, 4), Rect(5, 6, 7, 8)};
  EXPECT_FALSE(CaptureTransientSourceGeometry(other, source, unchanged));
  EXPECT_EQ(Rect(1, 2, 3, 4), unchanged.bounds_in_window);
  EXPECT_EQ(Rect(5, 6, 7, 8), unchanged.visible_bounds_in_window);

  source.setVisibility(Visibility::kInvisible);
  EXPECT_FALSE(CaptureTransientSourceGeometry(owner_, source, unchanged));
  EXPECT_EQ(Rect(1, 2, 3, 4), unchanged.bounds_in_window);
  EXPECT_EQ(Rect(5, 6, 7, 8), unchanged.visible_bounds_in_window);
  source.setVisibility(Visibility::kVisible);
  source.layout(Rect(0, 0, -1, -1));
  EXPECT_FALSE(CaptureTransientSourceGeometry(owner_, source, unchanged));
  EXPECT_EQ(Rect(1, 2, 3, 4), unchanged.bounds_in_window);
  EXPECT_EQ(Rect(5, 6, 7, 8), unchanged.visible_bounds_in_window);
}

// Verifies detached mini-trees and both display-attached and task-nested host
// ancestors are rejected without relying on getTask() resolution.
TEST_F(HostTest, RejectsDetachedAndHostedSourceChains) {
  TestPanel detached_root(app_.context());
  FocusableWidget detached_source(app_.context());
  detached_root.add(WidgetRef(detached_source), Rect(0, 0, 4, 4));
  TransientSourceGeometry output{Rect(1, 1, 2, 2), Rect(1, 1, 2, 2)};
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

}  // namespace
}  // namespace roo_windows
