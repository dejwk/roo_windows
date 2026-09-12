#pragma once

#include <memory>
#include <string>

#include "roo_scheduler.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/layout_scaffold/layout_scaffold.h"
#include "roo_windows/widgets/text_block.h"

namespace roo_windows::material3 {

/// Semantic readable-time policy, excluding motion and modal interruptions.
enum class SnackbarDuration : uint8_t { kDefault, kShort, kLong, kPersistent };

/// Terminal result; destruction of a request cancels silently.
enum class SnackbarDismissReason : uint8_t {
  kTimeout,
  kAction,
  kDismiss,
  kReplaced,
  kProgrammatic,
  kCleared,
  kHostUnavailable
};

/// Admission never evicts another request implicitly.
enum class SnackbarShowResult : uint8_t {
  kShown,
  kQueued,
  kAlreadyRegistered,
  kQueueFull,
  kHostUnavailable
};

namespace test {
class SnackbarTestAccess;
}

class SnackbarPresenter;
class SnackbarHost;

/// Owning, self-cancelling request node; subclass for terminal action handling.
class SnackbarRequest {
 public:
  /// Creates an unregistered request with empty text.
  SnackbarRequest() = default;

  /// Silently removes this node from its presenter before releasing its text.
  virtual ~SnackbarRequest();

  SnackbarRequest(const SnackbarRequest&) = delete;
  SnackbarRequest& operator=(const SnackbarRequest&) = delete;
  SnackbarRequest(SnackbarRequest&&) = delete;
  SnackbarRequest& operator=(SnackbarRequest&&) = delete;

  /// Copies text while idle; rejects oversized input or registered mutation.
  bool configure(roo::string_view message, roo::string_view action = {},
                 SnackbarDuration duration = SnackbarDuration::kDefault,
                 bool show_dismiss = false);

  /// Returns whether the node is current or queued.
  bool isRegistered() const { return presenter_ != nullptr; }

  /// Returns the owned message.
  const std::string& message() const { return message_; }

  /// Returns the owned action label.
  const std::string& actionLabel() const { return action_; }

 protected:
  /// Runs after unlinking and releasing visual borrows; may delete this node.
  virtual void onFinished(SnackbarDismissReason reason) {}

 private:
  friend class SnackbarPresenter;
  SnackbarPresenter* presenter_ = nullptr;
  SnackbarRequest* next_ = nullptr;
  std::string message_;
  std::string action_;
  SnackbarDuration duration_ = SnackbarDuration::kDefault;
  bool show_dismiss_ = false;
};

/// One reusable inverse-surface snackbar with ordinary button input behavior.
class SnackbarWidget : public Container {
 public:
  /// Creates the reusable message and control tree.
  explicit SnackbarWidget(ApplicationContext& context);

  /// Detaches its inline children.
  ~SnackbarWidget() override;

  SnackbarWidget(const SnackbarWidget&) = delete;
  SnackbarWidget& operator=(const SnackbarWidget&) = delete;
  SnackbarWidget(SnackbarWidget&&) = delete;
  SnackbarWidget& operator=(SnackbarWidget&&) = delete;

  /// Copies message text and borrows the action label until next configuration.
  void setContent(const std::string& message, roo::string_view action,
                  bool show_dismiss);

  /// Returns the action control for normal focus and input operations.
  Button& actionButton() { return action_; }

  /// Returns the dismiss control for normal focus and input operations.
  Button& dismissButton() { return dismiss_; }

  /// Resolves the inverse surface from the current theme.
  Color background() const override;

  /// Advertises the Material snackbar corner shape.
  BorderStyle getBorderStyle() const override;

  /// Advertises Material level-three elevation.
  uint8_t getElevation() const override { return 3; }

 protected:
  /// Receives an action after the standard button click lifecycle settles.
  virtual void onAction() {}

  /// Receives explicit Close activation.
  virtual void onDismiss() {}

  /// Reports focus changes from either internal snackbar control.
  virtual void onControlFocusChanged(bool focused) { (void)focused; }

  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;
  int getChildrenCount() const override { return 3; }
  const Widget& getChild(int index) const override;
  Widget& getChild(int index) override;

 private:
  friend class SnackbarHost;
  friend class SnackbarPresenter;
  class Control final : public Button {
   public:
    Control(SnackbarWidget& owner, bool dismiss);
    Color background() const override;
    void paint(PaintContext& ctx) const override;
    void onClicked() override;

   protected:
    void onFocusChanged(bool focused) override;

   private:
    SnackbarWidget& owner_;
    bool dismiss_;
  };
  TextBlock message_;
  Control action_;
  Control dismiss_;
  int16_t action_width_ = 0;
  int16_t dismiss_width_ = 0;
  int16_t message_height_ = 0;
  bool stacked_ = false;
  bool separate_dismiss_ = false;
  bool rtl_ = false;
};

/// Bounded intrusive FIFO, one semantic timeout and one live widget per host.
class SnackbarPresenter : private roo_scheduler::Executable {
 public:
  /// Cancels all registrations and scheduled work before releasing state.
  ~SnackbarPresenter();

  SnackbarPresenter(const SnackbarPresenter&) = delete;
  SnackbarPresenter& operator=(const SnackbarPresenter&) = delete;
  SnackbarPresenter(SnackbarPresenter&&) = delete;
  SnackbarPresenter& operator=(SnackbarPresenter&&) = delete;

  /// Admits a node, with four total current/queued registrations maximum.
  SnackbarShowResult show(SnackbarRequest& request);

  /// Reuses the current slot and leaves pending FIFO entries in place.
  SnackbarShowResult replaceCurrent(SnackbarRequest& request);

  /// Starts explicit dismissal; completion follows the exit transition.
  void dismissCurrent(
      SnackbarDismissReason reason = SnackbarDismissReason::kProgrammatic);

  /// Immediately finishes all registrations, rejecting reentrant admission.
  void clear();

  /// Returns whether a current request exists, including during transitions.
  bool isShowing() const { return head_ != nullptr; }

  /// Returns queued entries excluding the current request.
  size_t pendingCount() const;

  /// Enables motion, or snaps current motion to its terminal geometry.
  void setAnimationsEnabled(bool enabled);

 private:
  friend class SnackbarHost;
  friend class SnackbarRequest;
  friend class test::SnackbarTestAccess;
  struct Lifetime {
    SnackbarPresenter* owner;
  };
  explicit SnackbarPresenter(SnackbarHost& host);
  void execute(roo_scheduler::ExecutionID id) override;
  void cancel(SnackbarRequest& request);
  void cancelMotion();
  void cancelTimeout(roo_time::Uptime now, bool consume_elapsed = true);
  void consumeReadableTime(roo_time::Uptime now);
  void armTimeout(roo_time::Uptime now);
  void reconcile(roo_time::Uptime now);
  bool motionPaused() const;
  bool readableTimePaused() const;
  bool startMotion(float from, float to, roo_time::Duration duration);
  void start();
  void finish(SnackbarDismissReason reason, bool notify = true);
  void shutdown();
  void drain(SnackbarDismissReason reason);
  bool available() const;
  float offset() const { return offset_; }
  void motionFrame(float value);
  void motionFinished(AnimationFinishReason reason);
  void presentationOrLayoutChanged();
  void transientActivityChanged(bool active);
  void controlFocusChanged(bool focused);

  SnackbarHost& host_;
  std::shared_ptr<Lifetime> lifetime_;
  SnackbarRequest* head_ = nullptr;
  roo_scheduler::ExecutionID timeout_id_ = -1;
  roo_time::Uptime timeout_anchor_;
  roo_time::Duration timeout_remaining_;
  float offset_ = 0.0f;
  enum class Phase : uint8_t { kEntering, kVisible, kExiting };
  Phase phase_ = Phase::kVisible;
  SnackbarDismissReason exit_reason_ = SnackbarDismissReason::kProgrammatic;
  bool animations_ = true;
  bool draining_ = false;
  bool timeout_enabled_ = false;
  bool transient_active_ = false;
  bool control_focused_ = false;
};

/// Opt-in scaffold with one snackbar lane; use as a task destination root.
/// Navigation detachment cancels all feedback belonging to this host.
class SnackbarHost : public LayoutScaffold {
 public:
  /// Creates a shell with an initially idle feedback lane.
  explicit SnackbarHost(ApplicationContext& context);

  /// Finishes registered requests and detaches the inline visual.
  ~SnackbarHost() override;

  SnackbarHost(const SnackbarHost&) = delete;
  SnackbarHost& operator=(const SnackbarHost&) = delete;
  SnackbarHost(SnackbarHost&&) = delete;
  SnackbarHost& operator=(SnackbarHost&&) = delete;

  /// Returns this host's bounded feedback presenter.
  SnackbarPresenter& snackbars() { return presenter_; }

  /// Returns the live snackbar visual for focus and inspection.
  SnackbarWidget& snackbarWidget() { return widget_; }

  /// Copies up to three local avoidance rectangles; rejects excessive count.
  bool setSnackbarAvoidance(const Rect* rectangles, size_t count);

  /// Selects logical start placement instead of the default centered layout.
  void setSnackbarStartAligned(bool start);

 protected:
  void onLayout(bool changed, const Rect& rect) override;
  void setParent(Container* parent, bool is_owned) override;
  void onAnimationFrame(AnimationTag tag,
                        const AnimationSample& sample) override;
  void onAnimationFinished(AnimationTag tag,
                           AnimationFinishReason reason) override;
  void onPresentationChanged(const PresentationChange& change) override;
  void onTransientActivityChanged(bool active) override;
  int getChildrenCount() const override;
  const Widget& getChild(int index) const override;
  Widget& getChild(int index) override;

 private:
  friend class SnackbarPresenter;
  friend class test::SnackbarTestAccess;
  class Visual final : public SnackbarWidget {
   public:
    Visual(ApplicationContext& context, SnackbarPresenter& presenter);

   protected:
    void onAction() override;
    void onDismiss() override;
    void onControlFocusChanged(bool focused) override;

   private:
    SnackbarPresenter& presenter_;
  };
  void placeSnackbar(bool measure);
  void observeTransientActivity();
  void unobserveTransientActivity();
  static constexpr AnimationTag kMotion = 0;
  SnackbarPresenter presenter_;
  Visual widget_;
  Rect target_{0, 0, -1, -1};
  Rect avoidance_[3];
  uint8_t avoidance_count_ = 0;
  bool start_aligned_ = false;
  bool detaching_ = false;
};

}  // namespace roo_windows::material3
