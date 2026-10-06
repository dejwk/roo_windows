#pragma once

#include "roo_windows/containers/scrollable_panel.h"

namespace roo_windows {

/// Opt-in scrolling that trades temporary background and pending-composition
/// trails for more frequent display updates. With a finite advisory paint
/// budget, moving paints progressively repair 16-pixel bands. The next
/// scheduled paint at an unchanged position completes the entire current image,
/// even during a held-still drag. Ordinary scrollers retain complete-paint
/// behavior. No framebuffer cache or frame-spanning paint records are retained.
class AcceleratedScrollablePanel : public SimpleScrollablePanel {
 public:
  /// Creates a scroll viewport around @p contents with the given axis policy.
  /// Contents use normal WidgetRef ownership; an unlimited window budget keeps
  /// all background operations complete.
  AcceleratedScrollablePanel(ApplicationContext& context, WidgetRef contents,
                             Direction direction = Direction::kVertical);

  /// Requests a complete viewport redraw asynchronously, without stopping
  /// motion. Use before capture or when application policy requires a clean
  /// frame.
  void requestCompleteRedraw();

 protected:
  void paintWidgetContents(PaintContext& ctx) override;

  enum class PaintMode { kUnavailable, kPartial, kComplete, kAccelerated };

  /// Consumes the old complete-paint obligation before traversal so new damage
  /// raised during paint survives. Geometry is independent of the damage clip.
  PaintMode beginPaint(PaintContext& ctx);

  /// Requests ordinary full-viewport damage without treating it as foreign.
  void requestCleanup();

  /// Advances rotating progress and schedules cleanup after actual omission.
  void finishAcceleratedPaint(bool deferred);

  /// Forces normal complete painting at the next scheduled opportunity.
  void requireCompleteRedraw();

  void propagateDirty(const Widget* child, const Rect& rect) override;

  void invalidateDescending() override;

  void invalidateDescending(const Rect& rect) override;

  bool invalidateBeneathDescending(const Rect& rect,
                                   const Widget* subject) override;

  void onScrollUpdate(bool active) override;

  void onLayout(bool changed, const Rect& rect) override;

  void onPresentationChanged(const PresentationChange& change) override;

  roo_display::Box viewport() const { return last_viewport_; }
  roo_display::Box opaqueInterior() const { return last_opaque_interior_; }
  uint16_t nextBand() const { return next_band_; }
  bool cleanupPending() const { return cleanup_pending_; }

 private:
  /// Computes the display-clipped viewport, respecting each child clip mode.
  roo_display::Box visibleViewport(const PaintContext& ctx) const;

  /// Exempts only framework scroll updates and our own cleanup requests.
  void observeDamage();

  ScrollPosition last_position_{0, 0};
  roo_display::Box last_viewport_{0, 0, -1, -1};
  roo_display::Box last_opaque_interior_{0, 0, -1, -1};
  roo_display::Color last_background_;
  uint16_t next_band_ = 0;
  bool baseline_valid_ = false;
  bool cleanup_pending_ = false;
  bool complete_required_ = true;
  bool in_scroll_update_ = false;
  bool requesting_cleanup_ = false;
};

}  // namespace roo_windows
