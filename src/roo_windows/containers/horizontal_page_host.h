#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

#include "roo_windows/containers/blit_cache_container.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/touch_event.h"
#include "roo_windows/core/widget_ref.h"

namespace roo_windows {

/// Viewport-sized host that keeps one selected content page active.
///
/// Phase 1 behavior supports programmatic page selection and current-page-only
/// layout. Swipe and settle animation hooks are reserved for later phases.
class HorizontalPageHost : public Container {
 public:
  /// Creates an empty page host.
  explicit HorizontalPageHost(ApplicationContext& context);

  ~HorizontalPageHost() override;

  /// Appends a page to the host's backing page list.
  ///
  /// The first page added becomes the current page.
  void addPage(WidgetRef page);

  /// Removes all pages and detaches any active children.
  void clearPages();

  /// Returns the number of stored pages.
  int pageCount() const;

  /// Returns the currently settled page index, or -1 when empty.
  int currentIndex() const;

  /// Returns the current gesture/programmatic target index, or -1 when empty.
  ///
  /// During a drag, this may switch before `currentIndex()` changes. It is
  /// intended for selector surfaces, such as tabs, that should react as soon
  /// as the gesture has chosen a likely destination.
  int targetIndex() const;

  /// Selects a new current page.
  ///
  /// Returns false if the index is out of range or already selected.
  /// In phase 2, adjacent targets animate and non-adjacent targets snap.
  bool setCurrentIndex(int index, bool animate = true);

  /// Clears invalidated viewport regions left uncovered by active pages.
  void paint(PaintContext& ctx) const override;

 protected:
  /// Hook called after the settled page index changes.
  virtual void onSettledIndexChanged(int old_index, int new_index);

  /// Hook called when the gesture/programmatic target page changes.
  virtual void onTargetIndexChanged(int old_index, int new_index);

  /// Measures as one viewport, not a strip width.
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  /// Lays out the active current page to fill the viewport.
  void onLayout(bool changed, const Rect& rect) override;

  /// Special-case interception for horizontal paging.
  ///
  /// Most containers should not override interception and should let child
  /// widgets own touch unless they explicitly relinquish it. This host is a
  /// deliberate exception: it must claim gestures once horizontal intent is
  /// confirmed (slop exceeded and horizontal motion dominates) so paging can
  /// stay synchronized across current/adjacent pages.
  bool onInterceptTouchEvent(const TouchEvent& event) override;
  DragAxis dragAxis() const override { return DragAxis::kHorizontal; }
  bool supportsFling() const override { return true; }

  /// Initializes an owned drag for animation handoff.
  ///
  /// The host cancels any in-flight settle animation and normalizes drag state
  /// so a new owned drag starts from the current visible fractional position.
  void onDragStart(XDim x, YDim y) override;
  void onDrag(XDim x, YDim y, XDim dx, YDim dy) override;
  void onFling(XDim x, YDim y, XDim vx, YDim vy) override;
  void onDragFinished(XDim x, YDim y) override;
  void onCancel() override;

  void onAnimationFrame(AnimationTag tag,
                        const AnimationSample& sample) override;
  void onAnimationFinished(AnimationTag tag,
                           AnimationFinishReason reason) override;
  void onPresentationChanged(const PresentationChange& change) override;

  bool shouldDelayChildPressedState() override { return true; }

  int getChildrenCount() const override;
  Widget& getChild(int idx) override;
  const Widget& getChild(int idx) const override;

 private:
  enum SlotId : uint8_t {
    kPreviousSlot = 0,
    kCurrentSlot = 1,
    kNextSlot = 2,
    kSlotCount = 3,
  };

  struct ActiveSlot {
    BlitCacheContainer* wrapper = nullptr;
    Widget* page = nullptr;
    int page_index = -1;
    bool attached = false;
  };

  static constexpr AnimationTag kSettle = 0;

  /// Binds slots to pages according to the current settled index.
  void syncActiveSlots();

  /// Assigns one slot to one page using a borrowed child attachment.
  void bindSlotToPage(SlotId slot_id, int page_index);

  /// Detaches the current page from the specified slot.
  void clearSlot(SlotId slot_id);

  /// Returns the idx-th active child in fixed slot order.
  Widget* activeChildAt(int idx);
  const Widget* activeChildAt(int idx) const;

  /// Positions active pages based on the current fractional page position.
  void updateActivePagePositions();

  /// Silently removes the shared settle channel.
  void cancelSettle();

  /// Returns whether the shared settle channel is active.
  bool isSettling() const;

  /// Starts settle animation toward target page index.
  void startSettleToIndex(int target_index);

  /// Snaps immediately to target page index.
  void snapToIndex(int target_index);

  /// Silently applies the selected target after a detached interval.
  void reconcileToTarget();

  /// Updates the transient target page and notifies subclasses on changes.
  void setTargetIndex(int target_index);

  /// Chooses gesture settle target from drag offset and optional fling speed.
  int resolveGestureSettleTarget(XDim velocity_x) const;

  /// Clamps and resists raw page position against host boundaries.
  float applyEdgeResistance(float raw_position) const;

  std::vector<WidgetRef> pages_;
  std::vector<int8_t> page_to_slot_;
  std::array<std::unique_ptr<BlitCacheContainer>, kSlotCount> slot_wrappers_;
  std::array<ActiveSlot, kSlotCount> active_slots_;

  bool dragging_;
  bool intercepted_gesture_;
  bool reconcile_when_presented_;

  int settled_index_;
  int target_index_;
  float raw_drag_position_;
  float page_position_;
};

}  // namespace roo_windows
