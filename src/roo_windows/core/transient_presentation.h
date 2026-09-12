#pragma once

#include <cstdint>
#include <vector>

#include "roo_collections_flat_small_hash_map.h"
#include "roo_windows/core/back_request.h"

namespace roo_windows {

class MainWindow;
class DisplayWindow;
class Widget;
namespace internal {
class TransientSurfaceHost;
}

/// Tracks the lifecycle of a transient presentation.
enum class TransientPresentationState : uint8_t {
  kIdle,
  kVisible,
  kFinishing,
};

/// Identifies why a transient presentation finished.
enum class PresentationFinishReason : uint8_t {
  kAction,
  kCancel,
  kOutsideInteraction,
  kBack,
  kReplacement,
  kOwnerDestroyed,
  kHostDestroyed,
  kTimeout,
  kInteractionOwnerDetached,
};

/// Reports whether a presentation registration occupied a host slot.
enum class PresentationStartResult : uint8_t {
  kStarted,
  kHostBusy,
  kReentrantReplacement,
  kInteractionOwnerUnavailable,
  kSurfaceUnavailable,
};

/// Selects which semantic Back requests a presentation receives.
struct TransientPresentationPolicy {
  constexpr TransientPresentationPolicy(bool dismiss_on_back = false,
                                        bool dismiss_on_escape = false)
      : dismiss_on_back(dismiss_on_back),
        dismiss_on_escape(dismiss_on_escape) {}

  bool dismiss_on_back : 1;
  bool dismiss_on_escape : 1;
};

class TransientPresentationSlot;

/// Presenter-owned participant in a transient-presentation slot.
///
/// Embed this as the final member of a presenter, after every resource that
/// the presenter detaches during normal teardown. Its destructor removes the
/// presenter from its slot without delivering completion.
class TransientPresentationRegistration {
 public:
  virtual ~TransientPresentationRegistration();

  TransientPresentationRegistration(const TransientPresentationRegistration&) =
      delete;
  TransientPresentationRegistration& operator=(
      const TransientPresentationRegistration&) = delete;

  /// Returns the presentation lifecycle state.
  TransientPresentationState state() const { return state_; }

  /// Returns whether this registration currently occupies a slot.
  bool isActive() const { return state_ != TransientPresentationState::kIdle; }

  /// Finishes a visible presentation through its registered host slot.
  ///
  /// This operation is idempotent. A hosted presentation with active click
  /// feedback remains attached, with input disabled, through a forced final
  /// feedback frame. Otherwise it is detached synchronously. In either case,
  /// it is removed from the slot before `onFinished()` receives the reason.
  void finish(PresentationFinishReason reason);

 protected:
  TransientPresentationRegistration() = default;

  /// Removes component-owned presentation resources before completion.
  virtual void detachPresentation(PresentationFinishReason reason) = 0;

  /// Delivers terminal completion after the registration is idle.
  virtual void onFinished(PresentationFinishReason reason) {}

  /// Handles an eligible semantic Back request.
  ///
  /// The default implementation finishes the root presentation with
  /// `PresentationFinishReason::kBack`.
  virtual BackResult onBackRequested(BackSource source);

  /// Handles a completed outside tap when the active surface delegates it.
  ///
  /// The handler may leave the presentation open, finish it, or destroy its
  /// presenter. The host performs no presenter access after this call.
  virtual void onOutsideInteraction() {}

  /// Vacates the slot without terminal delivery during presenter destruction.
  void cancel();

  /// Disables an associated structural host before derived teardown begins.
  void disableHostedInput();

 private:
  friend class TransientPresentationSlot;
  friend class internal::TransientSurfaceHost;

  TransientPresentationSlot* slot_ = nullptr;
  TransientPresentationState state_ = TransientPresentationState::kIdle;
  uint8_t policy_ = 0;
};

/// Stores the single interactive transient presentation for one host.
class TransientPresentationSlot {
 public:
  /// Finishes a remaining presentation with host teardown semantics.
  ~TransientPresentationSlot();

  TransientPresentationSlot() = default;
  explicit TransientPresentationSlot(MainWindow& window) : window_(&window) {}
  TransientPresentationSlot(const TransientPresentationSlot&) = delete;
  TransientPresentationSlot& operator=(const TransientPresentationSlot&) =
      delete;

  /// Registers `registration` when no interactive presentation is visible.
  PresentationStartResult show(TransientPresentationRegistration& registration,
                               TransientPresentationPolicy policy = {});

  /// Replaces the active registration unless completion reentrantly fills it.
  PresentationStartResult replace(
      TransientPresentationRegistration& registration,
      TransientPresentationPolicy policy = {});

  /// Sends an eligible semantic Back request to the active registration.
  BackResult requestBack(BackSource source);

  /// Finishes the active registration, if any.
  void clear(PresentationFinishReason reason);

  /// Returns whether a registration currently occupies this slot.
  bool hasActivePresentation() const { return active_ != nullptr; }

  /// Observes changes to `hasActivePresentation()` for an attached widget.
  /// The hook is delivered during application dispatch, before animations.
  /// It must not invoke application callbacks or mutate the widget hierarchy.
  bool observeActivity(Widget& widget);

  /// Stops activity observation. This operation is idempotent.
  void unobserveActivity(Widget& widget);

 private:
  friend class MainWindow;
  friend class DisplayWindow;
  friend class TransientPresentationRegistration;
  friend class internal::TransientSurfaceHost;
  friend class Container;

  /// Returns whether this slot permanently rejects new presentations.
  bool isAdmissionClosed() const { return admission_closed_; }

  class AdmissionGuard {
   public:
    explicit AdmissionGuard(TransientPresentationSlot& slot);
    ~AdmissionGuard();

   private:
    TransientPresentationSlot& slot_;
  };

  PresentationStartResult showHosted(
      TransientPresentationRegistration& registration,
      TransientPresentationPolicy policy, internal::TransientSurfaceHost& host);

  void shutdown(PresentationFinishReason reason);
  void noteActivityChanged();
  void deliverPendingActivityChanges();
  void activityObserverSubtreeDetaching(Widget& subtree);
  void clearActivityObservers();

  void finish(TransientPresentationRegistration& registration,
              PresentationFinishReason reason);
  void finishNow(TransientPresentationRegistration& registration,
                 PresentationFinishReason reason);
  bool finishDeferredIfReady();
  void cancel(TransientPresentationRegistration& registration);

  struct ActivityObserver {
    bool last_active = false;
  };

  MainWindow* window_ = nullptr;
  TransientPresentationRegistration* active_ = nullptr;
  internal::TransientSurfaceHost* active_host_ = nullptr;
  roo_collections::FlatSmallHashMap<Widget*, ActivityObserver>
      activity_observers_;
  std::vector<Widget*> activity_delivery_;
  bool clearing_ = false;
  bool admission_closed_ = false;
  bool admission_guard_ = false;
  bool activity_pending_ = false;
  bool delivering_activity_ = false;
};

}  // namespace roo_windows
