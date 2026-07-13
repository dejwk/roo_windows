#pragma once

#include <cstdint>

#include "roo_windows/core/back_request.h"

namespace roo_windows {

/// Tracks the lifecycle of a transient presentation.
enum class PresentationState : uint8_t {
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
};

/// Reports whether a presentation registration occupied a host slot.
enum class PresentationStartResult : uint8_t {
  kStarted,
  kHostBusy,
  kReentrantReplacement,
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

  TransientPresentationRegistration(
      const TransientPresentationRegistration&) = delete;
  TransientPresentationRegistration& operator=(
      const TransientPresentationRegistration&) = delete;

  /// Returns the presentation lifecycle state.
  PresentationState state() const { return state_; }

  /// Returns whether this registration currently occupies a slot.
  bool isActive() const { return state_ != PresentationState::kIdle; }

  /// Finishes a visible presentation through its registered host slot.
  ///
  /// This operation is idempotent. The presentation is detached and removed
  /// from the slot before `onFinished()` receives the reason.
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

  /// Vacates the slot without terminal delivery during presenter destruction.
  void cancel();

 private:
  friend class TransientPresentationSlot;

  TransientPresentationSlot* slot_ = nullptr;
  PresentationState state_ = PresentationState::kIdle;
  uint8_t policy_ = 0;
};

/// Stores the single interactive transient presentation for one host.
class TransientPresentationSlot {
 public:
  /// Finishes a remaining presentation with host teardown semantics.
  ~TransientPresentationSlot();

  TransientPresentationSlot() = default;
  TransientPresentationSlot(const TransientPresentationSlot&) = delete;
  TransientPresentationSlot& operator=(const TransientPresentationSlot&) =
      delete;

  /// Registers `registration` when no interactive presentation is visible.
  PresentationStartResult show(
      TransientPresentationRegistration& registration,
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

 private:
  friend class TransientPresentationRegistration;

  void finish(TransientPresentationRegistration& registration,
              PresentationFinishReason reason);
  void cancel(TransientPresentationRegistration& registration);

  TransientPresentationRegistration* active_ = nullptr;
  bool clearing_ = false;
  bool destroying_ = false;
};

}  // namespace roo_windows
