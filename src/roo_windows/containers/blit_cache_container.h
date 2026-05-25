#pragma once

#include "roo_display/core/box.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

// A single-child container that caches the framebuffer region occupied by its
// child, and uses hardware blit-copy to accelerate repositioning (e.g. during
// scrolling or dragging).
//
// When the wrapper is moved (via moveTo), it detects the position delta and
// records a pending blit operation. On the next paint, it uses blitCopy() on
// the raw display device to shift the existing framebuffer content, and only
// repaints the newly-revealed strip. This avoids a full repaint of the child
// subtree.
//
// If the display device does not support blitCopy, or if the child content has
// been modified (dirtied) between paints, the wrapper gracefully falls back to
// the standard full-repaint path.
//
// Usage with ScrollablePanel:
//   Wrap the scrollable contents in a BlitCacheContainer. The ScrollablePanel
//   calls moveTo() on the wrapper as usual; the wrapper handles the rest.
//
// Usage for dragging:
//   Place a BlitCacheContainer around an expensive-to-draw widget. When
//   repositioned during drag, only the newly-exposed edges need repainting.
class BlitCacheContainer : public Container {
 public:
  BlitCacheContainer(ApplicationContext& context);

  /// Replaces the held child widget. Detaches the previous child (if any).
  void setChild(WidgetRef child);
  /// Detaches the current child, leaving the container empty.
  void clearChild();

  int getChildrenCount() const override { return child_ == nullptr ? 0 : 1; }

  Widget& getChild(int idx) override { return *child_; }

  const Widget& getChild(int idx) const override { return *child_; }

  Widget* child() { return child_; }
  const Widget* child() const { return child_; }

  /// Records the position delta versus the previous layout so the next paint
  /// can blit-copy the cached pixels instead of repainting from scratch.
  /// Public because a parent (e.g. `ScrollablePanel`) drives the reposition.
  void moveTo(const Rect& parent_bounds) override;

 protected:
  PreferredSize getPreferredSize() const override;

  /// Delegates measurement to the held child.
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  /// Lays out the held child to fill the container's bounds.
  void onLayout(bool changed, const Rect& rect) override;

  /// Performs a hardware blit-copy when a pending move is recorded and the
  /// underlying device supports it; falls back to a full repaint otherwise.
  void paintWidgetContents(PaintContext& ctx) override;

  /// Invalidates the entire cached region (cannot blit-copy stale pixels).
  void invalidateDescending() override;
  /// Invalidates the region intersecting `rect` in the cache.
  void invalidateDescending(const Rect& rect) override;

  /// Marks a child sub-rect as dirty and shrinks the blit-safe region so the
  /// next paint avoids reusing those pixels.
  void propagateDirty(const Widget* child, const Rect& rect) override;
  /// Invalidates the whole cached region (child hidden).
  void childHidden(const Widget* child) override;
  /// Invalidates the whole cached region (child shown).
  void childShown(const Widget* child) override;

 private:
  // Computes the largest full-width clean Y band within the panel's device
  // rect, avoiding all exclusions from the clipper.
  roo_display::Box computeCleanBand(const roo_display::Box& panel_device,
                                    const std::vector<roo_display::Box>& excl,
                                    size_t excl_count, bool vertical) const;

  // Shrinks the blit_safe_region_ by subtracting a dirty rect.
  // For full-width/full-height bands, trims from the edge that the rect
  // touches, or keeps the larger remaining portion if the rect is in the
  // middle.
  void shrinkSafeRegion(const Rect& dirty_rect);

  Widget* child_;

  // The region of the framebuffer (in device coordinates) known to contain
  // correct pixels from the last paint of this widget's child. Only this
  // region is safe to blit from.
  roo_display::Box blit_safe_region_;

  // Accumulated scroll/move delta since last paint.
  int16_t pending_dx_;
  int32_t pending_dy_;
  bool has_pending_blit_;

  // Set to true during moveTo() to distinguish scroll-move signals from
  // genuine content changes in propagateDirty/childHidden/childShown.
  bool moving_;

  // Cached: whether the device supports blitCopy. Set on first paint.
  int8_t blit_supported_;  // -1 = unknown, 0 = no, 1 = yes
};

}  // namespace roo_windows
