#include "roo_windows/core/background_deferral.h"

#include "roo_windows/core/clipper.h"

namespace roo_windows::internal {

BackgroundDeferralScope::BackgroundDeferralScope(Clipper& clipper,
                                                 roo_display::Box viewport,
                                                 roo_display::Box interior,
                                                 uint16_t band)
    : clipper_(clipper),
      previous_(clipper.background_scope_),
      viewport_(viewport),
      interior_(interior),
      band_(band) {
  clipper_.background_scope_ = this;
}

BackgroundDeferralScope::~BackgroundDeferralScope() {
  clipper_.background_scope_ = previous_;
}

BackgroundDeferralSuspension::BackgroundDeferralSuspension(Clipper& clipper)
    : scope_(clipper.background_scope_), previous_interior_(0, 0, -1, -1) {
  if (scope_ != nullptr) {
    previous_interior_ = scope_->interior_;
    scope_->interior_ = roo_display::Box(0, 0, -1, -1);
  }
}

BackgroundDeferralSuspension::~BackgroundDeferralSuspension() {
  if (scope_ != nullptr) scope_->interior_ = previous_interior_;
}

}  // namespace roo_windows::internal
