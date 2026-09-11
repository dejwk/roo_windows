#include <functional>
#include <memory>

#include "gtest/gtest.h"
#include "roo_display/core/offscreen.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material3/dialog/basic_dialog.h"
#include "roo_windows/material3/snackbar/snackbar.h"

namespace roo_windows::material3 {
namespace test {
class SnackbarTestAccess {
 public:
  static void advance(SnackbarPresenter& p, uint32_t ms) {
    p.update(p.last_ms_ + ms);
  }
  static bool scheduled(const SnackbarPresenter& p) { return p.timer_ > 0; }
};
}  // namespace test
namespace {
using roo_display::Argb4444;
using roo_display::BlendingMode;
using roo_display::OffscreenDevice;
class CountingOffscreenDevice : public OffscreenDevice<Argb4444> {
 public:
  CountingOffscreenDevice(int16_t width, int16_t height, roo::byte* data,
                          const Argb4444& color_mode)
      : OffscreenDevice<Argb4444>(width, height, data, color_mode) {}

  void resetCounters() {
    blit_calls_ = 0;
    output_pixels_ = 0;
    address_windows_ = 0;
  }

  uint32_t blitCalls() const { return blit_calls_; }
  uint32_t outputPixels() const { return output_pixels_; }
  uint32_t addressWindows() const { return address_windows_; }

  void setAddress(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1,
                  BlendingMode mode) override {
    ++address_windows_;
    OffscreenDevice<Argb4444>::setAddress(x0, y0, x1, y1, mode);
  }

  void write(Color* color, uint32_t pixel_count) override {
    output_pixels_ += pixel_count;
    OffscreenDevice<Argb4444>::write(color, pixel_count);
  }

  void fill(Color color, uint32_t pixel_count) override {
    output_pixels_ += pixel_count;
    OffscreenDevice<Argb4444>::fill(color, pixel_count);
  }

  void writePixels(BlendingMode mode, Color* color, int16_t* x, int16_t* y,
                   uint16_t pixel_count) override {
    for (uint16_t i = 0; i < pixel_count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      write(&color[i], 1);
    }
  }

  void fillPixels(BlendingMode mode, Color color, int16_t* x, int16_t* y,
                  uint16_t pixel_count) override {
    for (uint16_t i = 0; i < pixel_count; ++i) {
      setAddress(x[i], y[i], x[i], y[i], mode);
      fill(color, 1);
    }
  }

  void writeRects(BlendingMode mode, Color* color, int16_t* x0, int16_t* y0,
                  int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(color[i], rectArea(x0[i], y0[i], x1[i], y1[i]));
    }
  }

  void fillRects(BlendingMode mode, Color color, int16_t* x0, int16_t* y0,
                 int16_t* x1, int16_t* y1, uint16_t count) override {
    for (uint16_t i = 0; i < count; ++i) {
      setAddress(x0[i], y0[i], x1[i], y1[i], mode);
      fill(color, rectArea(x0[i], y0[i], x1[i], y1[i]));
    }
  }

  void blitCopy(int16_t src_x0, int16_t src_y0, int16_t src_x1, int16_t src_y1,
                int16_t dst_x0, int16_t dst_y0) override {
    ++blit_calls_;
    OffscreenDevice<Argb4444>::blitCopy(src_x0, src_y0, src_x1, src_y1, dst_x0,
                                        dst_y0);
  }

 private:
  static uint32_t rectArea(int16_t x0, int16_t y0, int16_t x1, int16_t y1) {
    return static_cast<uint32_t>(x1 - x0 + 1) *
           static_cast<uint32_t>(y1 - y0 + 1);
  }

  uint32_t blit_calls_ = 0;
  uint32_t output_pixels_ = 0;
  uint32_t address_windows_ = 0;
};

class Request : public SnackbarRequest {
 public:
  explicit Request(const char* text = "Saved", const char* action = "") {
    EXPECT_TRUE(configure(text, action));
  }
  std::vector<SnackbarDismissReason> reasons;
  std::function<void()> finished;
  void onFinished(SnackbarDismissReason reason) override {
    reasons.push_back(reason);
    std::function<void()> callback = finished;
    if (callback) callback();
  }
};
class SnackbarTest : public testing::Test {
 protected:
  SnackbarTest()
      : device_(320, 240, raster_, roo_display::Argb4444()),
        display_(device_),
        env_(scheduler_),
        app_(new Application(&env_, display_)),
        host_(app_->context()),
        body_(app_->context(), "Settings", ButtonVariant::kText),
        task_(app_->addTaskFullScreen(host_)) {
    host_.setBody(body_);
    host_.snackbars().setAnimationsEnabled(false);
    app_->refresh();
  }
  ~SnackbarTest() override {
    app_.reset();
    host_.setBody(WidgetRef());
  }
  SnackbarPresenter& presenter() { return host_.snackbars(); }
  roo::byte raster_[320 * 240 * 2]{};
  CountingOffscreenDevice device_;
  roo_display::Display display_;
  roo_scheduler::Scheduler scheduler_;
  Environment env_;
  std::unique_ptr<Application> app_;
  SnackbarHost host_;
  Button body_;
  Task& task_;
};

// Verifies call-local text ownership, capacity rejection and FIFO progression.
TEST_F(SnackbarTest, OwnedPayloadBoundedAdmissionAndFifo) {
  Request a, b, c, d, overflow;
  std::string message = "Original";
  ASSERT_TRUE(a.configure(message, "Undo"));
  message[0] = 'X';
  EXPECT_EQ("Original", a.message());
  EXPECT_FALSE(a.configure(std::string(257, 'x')));
  EXPECT_EQ("Original", a.message());
  EXPECT_EQ(SnackbarShowResult::kShown, presenter().show(a));
  EXPECT_FALSE(a.configure("Changed"));
  EXPECT_EQ(SnackbarShowResult::kAlreadyRegistered, presenter().show(a));
  EXPECT_EQ(SnackbarShowResult::kQueued, presenter().show(b));
  presenter().show(c);
  presenter().show(d);
  EXPECT_EQ(SnackbarShowResult::kQueueFull, presenter().show(overflow));
  EXPECT_EQ(3u, presenter().pendingCount());
  presenter().dismissCurrent();
  EXPECT_FALSE(a.isRegistered());
  EXPECT_TRUE(b.isRegistered());
  EXPECT_EQ(2u, presenter().pendingCount());
  presenter().clear();
  EXPECT_EQ(SnackbarDismissReason::kCleared, d.reasons.at(0));
  EXPECT_FALSE(test::SnackbarTestAccess::scheduled(presenter()));
}

// Verifies request destruction unregisters current and queued nodes silently.
TEST_F(SnackbarTest, RequestDestructionCancelsWithoutDanglingLinks) {
  auto a = std::make_unique<Request>();
  auto b = std::make_unique<Request>();
  Request c;
  presenter().show(*a);
  presenter().show(*b);
  presenter().show(c);
  b.reset();
  EXPECT_EQ(1u, presenter().pendingCount());
  a.reset();
  EXPECT_TRUE(c.isRegistered());
  EXPECT_EQ(0u, presenter().pendingCount());
  presenter().dismissCurrent();
  EXPECT_EQ(1u, c.reasons.size());
}

// Verifies replacement at capacity preserves FIFO, and callbacks see new state.
TEST_F(SnackbarTest, ReplacementAndReentrantClear) {
  Request a, b, c, d, replacement;
  presenter().show(a);
  presenter().show(b);
  presenter().show(c);
  presenter().show(d);
  a.finished = [&] { EXPECT_TRUE(replacement.isRegistered()); };
  EXPECT_EQ(SnackbarShowResult::kShown,
            presenter().replaceCurrent(replacement));
  EXPECT_EQ(SnackbarDismissReason::kReplaced, a.reasons.at(0));
  EXPECT_EQ(3u, presenter().pendingCount());
  replacement.finished = [&] {
    EXPECT_EQ(SnackbarShowResult::kHostUnavailable, presenter().show(a));
    presenter().clear();
  };
  presenter().clear();
  EXPECT_FALSE(b.isRegistered());
  EXPECT_EQ(1u, replacement.reasons.size());
}

// Verifies a terminal callback may destroy itself and enqueue its successor.
TEST_F(SnackbarTest, TerminalCallbackCanDeleteRequestAndReadmit) {
  auto a = std::make_unique<Request>();
  Request b;
  a->finished = [&] {
    EXPECT_FALSE(a->isRegistered());
    a.reset();
    presenter().show(b);
  };
  presenter().show(*a);
  presenter().dismissCurrent();
  EXPECT_EQ(nullptr, a);
  EXPECT_TRUE(b.isRegistered());
}

// Verifies task teardown drains all nodes and rejects completion-time
// reopening.
TEST_F(SnackbarTest, ApplicationTeardownClearsRegistrationsAndTimer) {
  Request a, b, rejected;
  a.finished = [&] {
    EXPECT_EQ(SnackbarShowResult::kHostUnavailable, presenter().show(rejected));
  };
  presenter().show(a);
  presenter().show(b);
  app_.reset();
  EXPECT_FALSE(a.isRegistered());
  EXPECT_FALSE(b.isRegistered());
  EXPECT_EQ(SnackbarDismissReason::kHostUnavailable, a.reasons.at(0));
  EXPECT_FALSE(test::SnackbarTestAccess::scheduled(presenter()));
}

// Verifies readable duration pauses behind dialogs and while controls hold
// focus.
TEST_F(SnackbarTest, TimeoutModalPauseFocusPauseAndPersistentDefault) {
  Request a;
  presenter().show(a);
  test::SnackbarTestAccess::advance(presenter(), 3999);
  EXPECT_TRUE(a.isRegistered());
  DialogActionSpec ok{1, "OK", DialogActionRole::kAcknowledge};
  AlertDialog dialog(app_->context(), "Confirm", "Continue?", &ok, 1);
  ASSERT_EQ(DialogShowResult::kShown, dialog.show(task_));
  test::SnackbarTestAccess::advance(presenter(), 5000);
  EXPECT_TRUE(a.isRegistered());
  dialog.dismiss();
  test::SnackbarTestAccess::advance(presenter(), 1);
  EXPECT_EQ(SnackbarDismissReason::kTimeout, a.reasons.at(0));
  Request persistent("Saved", "Undo");
  presenter().show(persistent);
  test::SnackbarTestAccess::advance(presenter(), 60000);
  EXPECT_TRUE(persistent.isRegistered());
  presenter().clear();
  Request timed;
  timed.configure("Saved", "Undo", SnackbarDuration::kLong);
  presenter().show(timed);
  app_->refresh();
  ASSERT_TRUE(
      task_.focus().requestFocus(host_.snackbarWidget().actionButton()));
  test::SnackbarTestAccess::advance(presenter(), 10000);
  EXPECT_TRUE(timed.isRegistered());
  task_.focus().requestFocus(body_);
  test::SnackbarTestAccess::advance(presenter(), 10000);
  EXPECT_EQ(SnackbarDismissReason::kTimeout, timed.reasons.at(0));
}

// Verifies no focus theft, outside pass-through, action completion, and passive
// Back.
TEST_F(SnackbarTest, FocusTouchAndBackUseOrdinaryTaskRouting) {
  Request a("Saved", "Undo");
  ASSERT_TRUE(task_.focus().requestFocus(body_));
  presenter().show(a);
  app_->refresh();
  EXPECT_EQ(&body_, task_.focus().focused());
  std::vector<Widget*> path;
  ASSERT_TRUE(host_.fillTouchTargetPath(1, 1, path));
  EXPECT_EQ(&body_, path.back());
  EXPECT_EQ(BackResult::kUnhandled, task_.requestBack());
  EXPECT_TRUE(a.isRegistered());
  ASSERT_TRUE(task_.focus().moveFocus(host_, false));
  EXPECT_EQ(&host_.snackbarWidget().actionButton(), task_.focus().focused());
  host_.snackbarWidget().actionButton().onClicked();
  EXPECT_EQ(SnackbarDismissReason::kAction, a.reasons.at(0));
  EXPECT_NE(&host_.snackbarWidget().actionButton(), task_.focus().focused());
}

// Verifies visible motion, reduced-motion settlement, and obstacle/chrome
// avoidance.
TEST_F(SnackbarTest, AnimationPlacementAndInsets) {
  host_.setSafetyInsets(Insets(4, 8, 4, 12));
  Button bottom(app_->context(), "Navigation");
  host_.setBottomBar(bottom);
  app_->refresh();
  presenter().setAnimationsEnabled(true);
  Request a;
  presenter().show(a);
  app_->refresh();
  const Rect initial = host_.snackbarWidget().parent_bounds();
  test::SnackbarTestAccess::advance(presenter(), 150);
  const Rect settled = host_.snackbarWidget().parent_bounds();
  EXPECT_LT(settled.yMin(), initial.yMin());
  EXPECT_LT(settled.yMax(), host_.bottomBarBounds().yMin());
  Rect obstacle(100, settled.yMin(), 150, settled.yMax());
  host_.setSnackbarAvoidance(&obstacle, 1);
  EXPECT_LT(host_.snackbarWidget().parent_bounds().yMax(), obstacle.yMin());
  presenter().dismissCurrent();
  EXPECT_TRUE(a.isRegistered());
  presenter().setAnimationsEnabled(false);
  EXPECT_FALSE(a.isRegistered());
  host_.clearBottomBar();
}

// Verifies removal callbacks may delete the detached host while clear is
// draining.
TEST_F(SnackbarTest, ClearCallbackCanRemoveAndDestroyHost) {
  Request a, b;
  auto host = std::make_unique<SnackbarHost>(app_->context());
  host->setBody(std::make_unique<Button>(app_->context(), "Other"));
  Task& other = app_->addTaskFullScreen(*host);
  app_->refresh();
  host->snackbars().setAnimationsEnabled(false);
  a.finished = [&] {
    other.navigation().clear();
    host.reset();
  };
  host->snackbars().show(a);
  host->snackbars().show(b);
  host->snackbars().clear();
  EXPECT_EQ(nullptr, host);
  EXPECT_FALSE(a.isRegistered());
  EXPECT_FALSE(b.isRegistered());
  EXPECT_EQ(SnackbarDismissReason::kHostUnavailable, b.reasons.at(0));
}

// Verifies reusable widgets measure wrapping without a parent height limit.
TEST_F(SnackbarTest, WrapContentHeightAndTinyConstraints) {
  SnackbarWidget widget(app_->context());
  widget.setContent("A long message that wraps across two readable lines", {},
                    false);
  const Dimensions wrapped =
      widget.measure(WidthSpec::Exactly(180), HeightSpec::Unspecified(0));
  EXPECT_GT(wrapped.height(), Scaled(48));
  const Dimensions tiny =
      widget.measure(WidthSpec::Exactly(8), HeightSpec::AtMost(8));
  EXPECT_EQ(8, tiny.width());
  EXPECT_LE(tiny.height(), 8);
}

// Verifies navigation cancellation, hidden-time pause, and root-only admission.
TEST_F(SnackbarTest, VisibilityAndDestinationLifetime) {
  Request a;
  presenter().show(a);
  host_.setVisibility(Visibility::kInvisible);
  test::SnackbarTestAccess::advance(presenter(), 5000);
  EXPECT_TRUE(a.isRegistered());
  host_.setVisibility(Visibility::kVisible);
  task_.navigation().clear();
  EXPECT_FALSE(a.isRegistered());
  EXPECT_EQ(SnackbarDismissReason::kHostUnavailable, a.reasons.at(0));
  EXPECT_FALSE(test::SnackbarTestAccess::scheduled(presenter()));
  EXPECT_EQ(SnackbarShowResult::kHostUnavailable, presenter().show(a));
}

// Verifies moving and dismissing an elevated snackbar restores every pixel,
// including decorations outside its logical bounds.
TEST_F(SnackbarTest, MotionAndDismissalRestoreDecorations) {
  const std::vector<roo::byte> baseline(raster_, raster_ + sizeof(raster_));
  presenter().setAnimationsEnabled(true);
  Request a;
  presenter().show(a);
  app_->refresh();
  for (int i = 0; i < 5; ++i) {
    test::SnackbarTestAccess::advance(presenter(), 30);
    app_->refresh();
  }
  const std::vector<roo::byte> animated(raster_, raster_ + sizeof(raster_));
  host_.invalidateInterior();
  app_->refresh();
  EXPECT_EQ(animated,
            std::vector<roo::byte>(raster_, raster_ + sizeof(raster_)));
  presenter().dismissCurrent();
  for (int i = 0; i < 5; ++i) {
    test::SnackbarTestAccess::advance(presenter(), 20);
    app_->refresh();
  }
  EXPECT_FALSE(presenter().isShowing());
  EXPECT_EQ(baseline,
            std::vector<roo::byte>(raster_, raster_ + sizeof(raster_)));
}

// Verifies a motion frame only writes the snackbar band, then idle writes
// nothing.
TEST_F(SnackbarTest, AnimationDoesNotRepaintWholeDisplay) {
  presenter().setAnimationsEnabled(true);
  Request a;
  presenter().show(a);
  app_->refresh();
  device_.resetCounters();
  test::SnackbarTestAccess::advance(presenter(), 75);
  app_->refresh();
  EXPECT_GT(device_.outputPixels(), 0u);
  EXPECT_LT(device_.outputPixels(), 320u * 120u);
  test::SnackbarTestAccess::advance(presenter(), 75);
  app_->refresh();
  device_.resetCounters();
  app_->refresh();
  EXPECT_EQ(0u, device_.outputPixels());
}
}  // namespace
}  // namespace roo_windows::material3
