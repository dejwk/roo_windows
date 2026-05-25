#pragma once

#include "roo_display/ui/alignment.h"
#include "roo_scheduler.h"
#include "roo_time.h"
#include "roo_windows/containers/blit_cache_container.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

// Optional scroll bar that appears at the right side of the screen during
// vertical scrolling, and enables quick scrolling.
class VerticalScrollBar : public Widget {
 public:
  enum class Presence { kAlwaysShown, kShownWhenScrolling, kAlwaysHidden };

  VerticalScrollBar(ApplicationContext& context) : Widget(context), begin_(0), end_(0) {}

  /// Sets the visible range of the scrolled content along the Y axis (in
  /// content coordinates) so the thumb's size and position match the panel.
  void setRange(int16_t begin, int16_t end);

  /// Paints the thumb proportional to the configured range.
  void paint(PaintContext& ctx) const override;

  PreferredSize getPreferredSize() const override;

  /// Reports the fixed minimum bar thickness.
  Dimensions getSuggestedMinimumDimensions() const override;

  int16_t begin() const { return begin_; }
  int16_t end() const { return end_; }

  /// Returns the scroll offset (in content coordinates) implied by the
  /// current thumb position for a viewport of the given `height`.
  YDim toOffset(YDim height) const;

 private:
  int16_t begin_;
  int16_t end_;
};

/// Container that wraps a single child widget and lets it scroll in one or
/// two directions, with momentum/fling, bounce/overshoot, and optional
/// vertical scroll-bar overlay.
///
/// The contents widget is owned by the panel and measured against the panel's
/// content area; the panel then translates child paint and touch dispatch by
/// the current scroll offset. Use this as the base for any view that needs
/// generic scrolling behavior.
class SimpleScrollablePanel : public Container,
                              private roo_scheduler::Executable {
 public:
  enum class Direction { kVertical = 0, kHorizontal = 1, kBoth = 2 };

  SimpleScrollablePanel(ApplicationContext& context, WidgetRef contents,
                        Direction direction = Direction::kVertical)
      : SimpleScrollablePanel(context, direction) {
    setContents(std::move(contents));
  }

  SimpleScrollablePanel(ApplicationContext& context,
                        Direction direction = Direction::kVertical)
      : Container(context),
        direction_(direction),
        alignment_(roo_display::kLeft | roo_display::kTop),
        contents_(nullptr),
        scroll_bar_presence_(VerticalScrollBar::Presence::kAlwaysHidden),
        scroll_bar_(context),
        scroll_bar_gesture_(false),
        scheduler_(context.scheduler()),
        notification_id_(-1),
        animation_(ScrollAnimation::IDLE),
        anim_{} {
    scroll_bar_.setVisibility(Visibility::kInvisible);
  }

  ~SimpleScrollablePanel() { cancelPendingUpdate(); }

  void setContents(WidgetRef new_contents) {
    if (contents() == &*new_contents &&
        contents()->isOwnedByParent() == new_contents.is_owned()) {
      return;
    }
    if (contents_ != nullptr) {
      detachChild(contents_);
      detachChild(&scroll_bar_);
    }
    contents_ = new_contents.get();
    if (contents_ != nullptr) {
      attachChild(std::move(new_contents));
      attachChild(scroll_bar_);
    }
  }

  void clearContents() { setContents(WidgetRef()); }
  bool hasContents() const { return contents_ != nullptr; }

  int getChildrenCount() const override { return hasContents() ? 2 : 0; }

  Widget& getChild(int idx) override {
    switch (idx) {
      case 0:
        return *contents_;
      case 1:
      default:
        return scroll_bar_;
    }
  }

  const Widget& getChild(int idx) const override {
    switch (idx) {
      case 0:
        return *contents_;
      case 1:
      default:
        return scroll_bar_;
    }
  }

  /// Sets the alignment of the content within the panel when it is smaller
  /// than the visible area.
  void setAlign(roo_display::Alignment alignment) {
    alignment_ = alignment;
    update();
  }

  /// Configures whether the vertical scroll bar is always visible, shown
  /// only while scrolling, or always hidden.
  void setVerticalScrollBarPresence(VerticalScrollBar::Presence presence) {
    if (scroll_bar_presence_ == presence) return;
    switch (scroll_bar_presence_) {
      case VerticalScrollBar::Presence::kAlwaysShown: {
        scroll_bar_.setVisibility(Visibility::kVisible);
        break;
      }
      case VerticalScrollBar::Presence::kAlwaysHidden: {
        scroll_bar_.setVisibility(Visibility::kInvisible);
        break;
      }
      default: {
        if (animation_ == ScrollAnimation::FLINGING || isHandlingGesture()) {
          // Currently scrolling or moving.
          scroll_bar_.setVisibility(Visibility::kVisible);
        } else {
          scroll_bar_.setVisibility(Visibility::kInvisible);
        }
      }
    }
    scroll_bar_presence_ = presence;
  }

  Widget* contents() { return contents_; }
  const Widget* contents() const { return contents_; }

  /// Scrolls so that the content origin sits at (x, y) relative to the
  /// viewport's top-left. Clamps to scroll boundaries.
  void scrollTo(XDim x, YDim y);

  /// Adjusts the current scroll position by (dx, dy).
  void scrollBy(XDim dx, YDim dy) {
    if (contents() == nullptr) return;
    scrollTo(dx + contents()->offsetLeft(), dy + contents()->offsetTop());
  }

  /// Snaps to the top edge of the content.
  void scrollToTop() {
    if (contents() == nullptr) return;
    scrollTo(contents()->offsetLeft(), 0);
  }

  /// Snaps to the bottom edge of the content.
  void scrollToBottom();

  struct ScrollPosition {
    XDim x;
    YDim y;
  };

  ScrollPosition getScrollPosition() const {
    const Widget* w = contents();
    if (w == nullptr) return {0, 0};
    return {w->offsetLeft(), w->offsetTop()};
  }

  /// Re-applies the current scroll position. Useful after content changes.
  void update() { scrollBy(0, 0); }

  // void paintWidgetContents(const Canvas& canvas, Clipper& clipper) override;

  /// Hook invoked whenever the scroll position changes (drag, fling, etc.).
  virtual void onScrollPositionChanged() {}

  /// Intercepts gestures targeting the scroll bar or the scrolling motion
  /// itself.
  bool onInterceptTouchEvent(const TouchEvent& event) override;

  /// Pins the touch point and cancels any in-flight animation.
  bool onDown(XDim x, YDim y) override;
  /// Allows the child to handle the tap normally (no scroll action).
  bool onSingleTapUp(XDim x, YDim y) override;
  /// Translates drag deltas into immediate scroll movement.
  bool onScroll(XDim x, YDim y, XDim dx, YDim dy) override;
  /// Starts a momentum fling animation seeded from the gesture velocity.
  bool onFling(XDim x, YDim y, XDim vx, YDim vy) override;
  /// Initiates a spring-back animation if release leaves the panel in
  /// overshoot.
  bool onTouchUp(XDim x, YDim y) override;

  bool supportsScrolling() const override { return true; }

 protected:
  PreferredSize getPreferredSize() const override;

  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  void onLayout(bool changed, const Rect& rect) override;

 private:
  void execute(roo_scheduler::EventID id) override;

  void scrollVertically(YDim yscroll);

  void cancelPendingUpdate();
  void scheduleScrollAnimationUpdate();
  void scheduleHideScrollBarUpdate();

  // Moves content to (x, y) without boundary clamping, for overshoot rendering.
  // Scroll bar is updated as if content were at the nearest boundary.
  void scrollToRaw(XDim x, YDim y);

  // Returns true if the content is currently positioned beyond a scroll
  // boundary (i.e., it has been dragged or flung into overshoot).
  bool isInOvershoot() const;

  // If the content is currently beyond a scroll boundary (in overshoot),
  // initiates a spring-back animation to return it to the boundary.
  void startSpringBack();

  Direction direction_;
  roo_display::Alignment alignment_;

  Dimensions measured_;

  Widget* contents_;

  // TODO: consider also supporting horizontal bar when needed.
  VerticalScrollBar::Presence scroll_bar_presence_;
  VerticalScrollBar scroll_bar_;
  roo_time::Uptime deadline_hide_scrollbar_;

  // Whether the 'down' event happened in the area reserved for the scroll bar.
  // In that case, the motion is never interpreted as anything else but possibly
  // a scroll bar interaction.
  bool scroll_bar_gesture_;

  roo_scheduler::Scheduler& scheduler_;
  roo_scheduler::ExecutionID notification_id_;

  // Whether the 'down' event has been confirmed as a touch of the scroll bar in
  // its active region.
  bool is_scroll_bar_scrolled_;

  // Discriminates which animation state is currently active. DRAGGING,
  // FLINGING, and SPRING_BACK are mutually exclusive, allowing their data to
  // share a single union.
  enum class ScrollAnimation : uint8_t {
    IDLE,
    DRAGGING,
    FLINGING,
    SPRING_BACK
  };
  ScrollAnimation animation_;

  // Per-state data. Only the member corresponding to animation_ is valid.
  union {
    // Valid when animation_ == DRAGGING.
    struct {
      XDim raw_x;  // Raw (unclamped) intended scroll position.
      YDim raw_y;
    } drag;
    // Valid when animation_ == FLINGING.
    struct {
      unsigned long start_time;
      unsigned long end_time;
      float start_vx;  // Initial scroll velocity in pixels/s.
      float start_vy;
      float decel_x;
      float decel_y;
      XDim dx_start;  // Content offset at fling start.
      YDim dy_start;
    } fling;
    // Valid when animation_ == SPRING_BACK.
    struct {
      unsigned long start_time_ms;
      XDim start_ox;  // Initial X overshoot at animation start (scroll coords).
      YDim start_oy;  // Initial Y overshoot at animation start (scroll coords).
      XDim target_x;  // Clamped scroll-coord X to spring back to.
      YDim target_y;  // Clamped scroll-coord Y to spring back to.
    } springback;
  } anim_;
};

// SimpleScrollablePanel variant that wraps the content in a BlitCacheContainer,
// enabling fast-blit scroll optimization for displays with framebuffer blit
// support.
class ScrollableBlitPanel : public SimpleScrollablePanel {
 public:
  ScrollableBlitPanel(ApplicationContext& context, WidgetRef contents,
                      Direction direction = Direction::kVertical)
      : ScrollableBlitPanel(context, direction) {
    setContents(std::move(contents));
  }

  ScrollableBlitPanel(ApplicationContext& context,
                      Direction direction = Direction::kVertical)
      : SimpleScrollablePanel(context, direction), blit_cache_(context) {}

  /// Wraps `new_contents` in the internal `BlitCacheContainer` and installs
  /// it as the panel's scrolled content.
  void setContents(WidgetRef new_contents) {
    blit_cache_.setChild(std::move(new_contents));
    SimpleScrollablePanel::setContents(WidgetRef(blit_cache_));
  }

 private:
  BlitCacheContainer blit_cache_;
};

using ScrollablePanel = ScrollableBlitPanel;

}  // namespace roo_windows
