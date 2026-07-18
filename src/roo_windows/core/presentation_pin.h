#pragma once

#include <cstdint>
#include <memory>

#include "roo_windows/core/rect.h"

namespace roo_windows {

class MainWindow;
class PaintContext;
class Widget;

/// Result of attempting to register a presentation pin.
enum class PresentationPinShowResult : uint8_t {
  kShown,
  kAlreadyRegistered,
  kAnchorUnavailable,
  kAllocationFailed,
};

/// Host-owned paint-only visual settled at a MainWindow layer boundary.
///
/// Pins are intentionally not widgets: they do not take part in layout, hit
/// testing, focus, or gesture dispatch. A concrete pin is allocated by its
/// anchor only while presented, then owned by the window.
class PresentationPin {
 public:
  /// Releases a host-owned pin; derived destructors must not access anchor().
  virtual ~PresentationPin() = default;

  PresentationPin(const PresentationPin&) = delete;
  PresentationPin& operator=(const PresentationPin&) = delete;

 protected:
  PresentationPin() = default;

  /// Returns the registered anchor while this pin is active.
  Widget& anchor() const { return *anchor_; }

  /// Returns current paint bounds in MainWindow coordinates.
  virtual Rect boundsInWindow() const = 0;

  /// Returns the conservative region affected by a content-only change.
  virtual Rect dirtyBoundsInWindow() const { return boundsInWindow(); }

  /// Returns an optional tighter clip in MainWindow coordinates.
  ///
  /// The default is deliberately unbounded; MainWindow always intersects it
  /// with its own bounds, yielding the window-bounds default without storing
  /// a window pointer in every pin.
  virtual Rect clipBoundsInWindow() const {
    return Rect(-32768, -32768, 32767, 32767);
  }

  /// Paints final pixels and registers the corresponding paint-plan effects.
  virtual void paint(PaintContext& ctx) const = 0;

 private:
  friend class MainWindow;

  std::unique_ptr<PresentationPin> next_;
  Widget* anchor_ = nullptr;
  Widget* z_scope_root_ = nullptr;
  Rect presented_bounds_{0, 0, -1, -1};
};

}  // namespace roo_windows
