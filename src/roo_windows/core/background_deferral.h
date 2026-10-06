#pragma once

#include "roo_display/color/color.h"
#include "roo_display/core/box.h"

namespace roo_windows {
class Clipper;
class PaintContext;
namespace internal {

/// Borrows optional-erasure permission for one synchronous traversal only.
/// An empty viewport suspends permission while retaining outer ownership.
class BackgroundDeferralScope {
 public:
  /// Borrows viewport permission and one mandatory band until destruction.
  BackgroundDeferralScope(Clipper& clipper, roo_display::Box viewport,
                          roo_display::Box interior, uint16_t band);

  /// Restores the enclosing lexical plan without retaining paint pointers.
  ~BackgroundDeferralScope();

  BackgroundDeferralScope(const BackgroundDeferralScope&) = delete;
  BackgroundDeferralScope& operator=(const BackgroundDeferralScope&) = delete;

  /// Reports whether this plan preserved any old display pixels.
  bool deferred() const { return deferred_; }

 private:
  friend class ::roo_windows::Clipper;
  friend class ::roo_windows::PaintContext;
  friend class BackgroundDeferralSuspension;
  Clipper& clipper_;
  BackgroundDeferralScope* previous_;
  roo_display::Box viewport_;
  roo_display::Box interior_;
  uint16_t band_;
  bool deferred_ = false;
};

/// Suspends optional fills for unclipped groups and nested opt-in scrollers.
class BackgroundDeferralSuspension {
 public:
  /// Temporarily disables the current plan without introducing another owner.
  explicit BackgroundDeferralSuspension(Clipper& clipper);

  /// Restores the enclosing plan's permitted interior.
  ~BackgroundDeferralSuspension();

  BackgroundDeferralSuspension(const BackgroundDeferralSuspension&) = delete;
  BackgroundDeferralSuspension& operator=(const BackgroundDeferralSuspension&) =
      delete;

 private:
  BackgroundDeferralScope* scope_;
  roo_display::Box previous_interior_;
};

}  // namespace internal
}  // namespace roo_windows
