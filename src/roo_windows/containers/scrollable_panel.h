#pragma once

#include "roo_display/ui/alignment.h"
#include "roo_scheduler.h"
#include "roo_time.h"
#include "roo_windows/containers/blit_cache_container.h"
#include "roo_windows/containers/scroll_motion_controller.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/scroll_connection.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/core/widget_event_dispatcher.h"

namespace roo_windows {

// Optional scroll bar that appears at the right side of the screen during
// vertical scrolling, and enables quick scrolling.
class VerticalScrollBar : public Widget {
 public:
  enum class Presence { kAlwaysShown, kShownWhenScrolling, kAlwaysHidden };

  VerticalScrollBar(ApplicationContext& context)
      : Widget(context), begin_(0), end_(0) {}

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
/// generic scrolling behavior. The direct contents widget should use
/// `ParentClipMode::kClipped`; an unclipped contents widget may paint outside
/// the scrolling viewport. Fling and spring-back physics use one application
/// animation-registry channel; transient scrollbar hiding remains a separate
/// one-shot scheduler deadline.
class SimpleScrollablePanel : public Container,
                              private roo_scheduler::Executable {
 public:
  enum class Direction { kVertical = 0, kHorizontal = 1, kBoth = 2 };

  /// Returns the configured scroll axis policy.
  Direction direction() const { return direction_; }

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
        hide_notification_id_(-1),
        motion_() {
    scroll_bar_.setVisibility(Visibility::kInvisible);
    context.presentations().observe(*this);
  }

  ~SimpleScrollablePanel() override {
    internal::ScrollConnectionRegistry::Disconnect(*this);
    cancelMotion();
    cancelHideScrollBarUpdate();
    setContentsInternal(WidgetRef(),
                        false);  // Delete if owned, without events.
  }

  /// Allows caller-provided contents to opt out of the viewport clip.
  bool mayHaveUnclippedChildren() const override { return true; }

  /// Replaces the content and reports any resulting change of scroll origin.
  void setContents(WidgetRef new_contents) {
    setContentsInternal(std::move(new_contents), true);
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
        if (motion_.isAnimating() || isHandlingGesture()) {
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
    ScrollPosition pos = getScrollPosition();
    scrollTo(dx + pos.x, dy + pos.y);
  }

  /// Snaps to the top edge of the content.
  void scrollToTop() {
    if (contents() == nullptr) return;
    ScrollPosition pos = getScrollPosition();
    scrollTo(pos.x, 0);
  }

  /// Snaps to the bottom edge of the content.
  void scrollToBottom();

  /// Content-origin coordinates; retained here for source compatibility.
  using ScrollPosition = roo_windows::ScrollPosition;

  /// Handler receiving the previous and newly applied content origins.
  using ScrollHandler = WidgetEventDispatcher::ScrollHandler;

  /// Reacts to position changes from scrolling, layout, or content replacement.
  /// Runs synchronously after the virtual hook, only when coordinates change.
  /// Registration does not emit an initial event; use getScrollPosition().
  /// An empty handler disconnects. Captured objects must outlive registration.
  /// Register and scroll on the UI thread. Handlers may change registrations,
  /// but must not destroy the panel or recursively change its scroll position.
  /// Storage is application-owned; registration may allocate, dispatch does
  /// not.
  void setOnScrollPositionChanged(ScrollHandler handler);

  ScrollPosition getScrollPosition() const {
    if (contents() == nullptr) return {0, 0};
    return currentScrollPosition();
  }

  /// Re-applies the current scroll position. Useful after content changes.
  void update() { scrollBy(0, 0); }

  // void paintWidgetContents(const Canvas& canvas, Clipper& clipper) override;

  /// Hook invoked whenever the scroll position changes (drag, fling, etc.).
  virtual void onScrollPositionChanged() {}

  /// Reveals a focused descendant with the minimum scroll needed to make it
  /// visible in this panel's viewport.
  bool revealFocusedDescendant(Widget& descendant) override;

  /// Scrolls this panel in response to arrow, page, Home, and End keys after
  /// focused descendants leave those keys unhandled.
  bool onKeyEvent(const KeyEvent& event) override;

  /// Intercepts gestures targeting the scroll bar or the scrolling motion
  /// itself.
  bool onInterceptTouchEvent(const TouchEvent& event) override;

  /// Pins the scroll motion and cancels any in-flight animation on ownership.
  void onDragStart(XDim x, YDim y) override;
  /// Allows the child to handle the tap normally (no scroll action).
  void onSingleTapUp(XDim x, YDim y) override;
  /// Translates drag deltas into immediate scroll movement.
  void onDrag(XDim x, YDim y, XDim dx, YDim dy) override;
  /// Starts a momentum fling animation seeded from the gesture velocity.
  void onFling(XDim x, YDim y, XDim vx, YDim vy) override;
  /// Initiates a spring-back animation if release leaves the panel in
  /// overshoot.
  void onDragFinished(XDim x, YDim y) override;

  DragAxis dragAxis() const override {
    if (direction_ == Direction::kBoth) return DragAxis::kBoth;
    return direction_ == Direction::kVertical ? DragAxis::kVertical
                                              : DragAxis::kHorizontal;
  }
  bool supportsFling() const override { return true; }

 protected:
  PreferredSize getPreferredSize() const override;

  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  void onLayout(bool changed, const Rect& rect) override;

  void onAnimationFrame(AnimationTag tag,
                        const AnimationSample& sample) override;
  void onPresentationChanged(const PresentationChange& change) override;

  static constexpr AnimationTag kMotion = 0;

 private:
  void execute(roo_scheduler::EventID id) override;

  void scrollVertically(YDim yscroll);

  void cancelMotion();
  void startMotionTrack();
  void stopMotionAndClamp();
  void cancelHideScrollBarUpdate();
  void scheduleHideScrollBarUpdate();
  static scroll_motion::Axis AxisForDirection(Direction direction);
  scroll_motion::Geometry motionGeometry() const;
  ScrollPosition currentScrollPosition() const;
  void applyScrollResult(const scroll_motion::Result& result);
  void applyConnectedDelta(internal::ScrollConnection& connection, XDim dx,
                           YDim dy);
  void finishConnectedMotion(internal::ScrollConnection& connection);
  static scroll_motion::Geometry FreeMotionGeometry(Direction direction);
  void notifyScrollPositionChanged(ScrollPosition previous);

  friend class ScrollableBlitPanel;
  friend class internal::ScrollConnection;
  void setContentsInternal(WidgetRef new_contents, bool notify);
  bool containsDescendant(const Widget& descendant) const;

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

  roo_scheduler::SchedulerClient& scheduler_;
  roo_scheduler::ExecutionID hide_notification_id_;

  // Whether the 'down' event has been confirmed as a touch of the scroll bar in
  // its active region.
  bool is_scroll_bar_scrolled_;
  scroll_motion::State motion_;
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

  ~ScrollableBlitPanel() override {
    internal::ScrollConnectionRegistry::Disconnect(*this);
    // Detach the member wrapper before it is destroyed and before the base
    // destructor consults its content pointer. The wrapper then releases any
    // adopted child while both objects are still alive.
    setContentsInternal(WidgetRef(), false);
    blit_cache_.clearChild();
  }

  /// Wraps `new_contents` in the internal `BlitCacheContainer` and installs
  /// it as the panel's scrolled content.
  void setContents(WidgetRef new_contents) {
    bool replaced = blit_cache_.child() != new_contents.get();
    blit_cache_.setChild(std::move(new_contents));
    SimpleScrollablePanel::setContents(WidgetRef(blit_cache_));
    auto connection = internal::ScrollConnectionRegistry::Find(*this);
    if (replaced && connection != nullptr) {
      cancelMotion();
      motion_ = scroll_motion::State();
      connection->kinetic = false;
      connection->cancel();
      connection->reset();
    }
  }

 private:
  BlitCacheContainer blit_cache_;
};

using ScrollablePanel = ScrollableBlitPanel;

}  // namespace roo_windows
