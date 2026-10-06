#include "roo_windows/core/background_deferral.h"

#include "roo_windows/core/clipper.h"

namespace roo_windows::internal {

BackgroundDeferralScope::BackgroundDeferralScope(Clipper& clipper,
                                                 roo_display::Box viewport,
                                                 roo_display::Box interior,
                                                 uint16_t band, bool suspension)
    : clipper_(clipper),
      previous_(clipper.background_scope_),
      viewport_(viewport),
      interior_(interior),
      band_(band) {
  clipper_.background_scope_ =
      suspension && previous_ == nullptr ? nullptr : this;
}

BackgroundDeferralScope::~BackgroundDeferralScope() {
  clipper_.background_scope_ = previous_;
}

BackgroundDeferralSuspension::BackgroundDeferralSuspension(Clipper& clipper)
    : scope_(clipper, roo_display::Box(0, 0, -1, -1),
             roo_display::Box(0, 0, -1, -1), 0, true) {}

}  // namespace roo_windows::internal
