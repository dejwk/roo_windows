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
  BackgroundDeferralScope(Clipper& clipper, roo_display::Box viewport,
                          roo_display::Box interior, uint16_t band,
                          bool suspension = false);

  ~BackgroundDeferralScope();

  BackgroundDeferralScope(const BackgroundDeferralScope&) = delete;
  BackgroundDeferralScope& operator=(const BackgroundDeferralScope&) = delete;

  bool deferred() const { return deferred_; }

 private:
  friend class ::roo_windows::Clipper;
  friend class ::roo_windows::PaintContext;
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
  explicit BackgroundDeferralSuspension(Clipper& clipper);

 private:
  BackgroundDeferralScope scope_;
};

}  // namespace internal
}  // namespace roo_windows
