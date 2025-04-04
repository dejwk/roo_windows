#pragma once

#include "roo_display/ui/alignment.h"
#include "roo_scheduler.h"
#include "roo_time.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

// Optional scroll bar that appears at the right side of the screen during
// vertical scrolling, and enables quick scrolling.
class VerticalScrollBar : public Widget {
 public:
  enum Presence { ALWAYS_SHOWN, SHOWN_WHEN_SCROLLING, ALWAYS_HIDDEN };

  VerticalScrollBar(const Environment& env) : Widget(env), begin_(0), end_(0) {}

  void setRange(int16_t begin, int16_t end);

  void paint(const Canvas& canvas) const override;

  PreferredSize getPreferredSize() const override;

  Dimensions getSuggestedMinimumDimensions() const override;

  int16_t begin() const { return begin_; }
  int16_t end() const { return end_; }

  // Returns the offset to the interior of the given height, implied by
  // the current location of the scroll bar.
  YDim toOffset(YDim height) const;

 private:
  int16_t begin_;
  int16_t end_;
};

class ScrollablePanel : public Container, private roo_scheduler::Executable {
 public:
  enum Direction { VERTICAL = 0, HORIZONTAL = 1, BOTH = 2 };

  ScrollablePanel(const Environment& env, WidgetRef contents,
                  Direction direction = VERTICAL)
      : ScrollablePanel(env, direction) {
    setContents(std::move(contents));
  }

  ScrollablePanel(const Environment& env, Direction direction = VERTICAL)
      : Container(env),
        direction_(direction),
        alignment_(roo_display::kLeft | roo_display::kTop),
        contents_(nullptr),
        scroll_start_vx_(0.0),
        scroll_start_vy_(0.0),
        scroll_bar_presence_(VerticalScrollBar::ALWAYS_HIDDEN),
        scroll_bar_(env),
        scroll_bar_gesture_(false),
        scheduler_(env.scheduler()),
        notification_id_(-1) {
    scroll_bar_.setVisibility(INVISIBLE);
  }

  ~ScrollablePanel() { cancelPendingUpdate(); }

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

  Widget& getChild(int idx) {
    switch (idx) {
      case 0:
        return *contents_;
      case 1:
      default:
        return scroll_bar_;
    }
  }

  const Widget& getChild(int idx) const {
    switch (idx) {
      case 0:
        return *contents_;
      case 1:
      default:
        return scroll_bar_;
    }
  }

  void setAlign(roo_display::Alignment alignment) {
    alignment_ = alignment;
    update();
  }

  void setVerticalScrollBarPresence(VerticalScrollBar::Presence presence) {
    if (scroll_bar_presence_ == presence) return;
    switch (scroll_bar_presence_) {
      case VerticalScrollBar::ALWAYS_SHOWN: {
        scroll_bar_.setVisibility(VISIBLE);
        break;
      }
      case VerticalScrollBar::ALWAYS_HIDDEN: {
        scroll_bar_.setVisibility(INVISIBLE);
        break;
      }
      default: {
        if (scroll_start_vx_ != 0 || scroll_start_vy_ != 0 ||
            isHandlingGesture()) {
          // Currently scrolling or moving.
          scroll_bar_.setVisibility(VISIBLE);
        } else {
          scroll_bar_.setVisibility(INVISIBLE);
        }
      }
    }
    scroll_bar_presence_ = presence;
  }

  Widget* contents() { return contents_; }
  const Widget* contents() const { return contents_; }

  // Sets the relative position of the underlying content, relative to the the
  // visible rectangle.
  void scrollTo(XDim x, YDim y);

  // Adjusts the relative position of the underlying content by the specified
  // offset.
  void scrollBy(XDim dx, YDim dy) {
    if (contents() == nullptr) return;
    scrollTo(dx + contents()->offsetLeft(), dy + contents()->offsetTop());
  }

  void scrollToTop() {
    if (contents() == nullptr) return;
    scrollTo(contents()->offsetLeft(), 0);
  }

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

  void update() { scrollBy(0, 0); }

  // void paintWidgetContents(const Canvas& canvas, Clipper& clipper) override;

  // Called when the position of the scroll changed, possibly due to an
  // animation in progress.
  virtual void onScrollPositionChanged() {}

  bool onInterceptTouchEvent(const TouchEvent& event) override;

  bool onDown(XDim x, YDim y) override;
  bool onSingleTapUp(XDim x, YDim y) override;
  bool onScroll(XDim x, YDim y, XDim dx, YDim dy) override;
  bool onFling(XDim x, YDim y, XDim vx, YDim vy) override;
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

  Direction direction_;
  roo_display::Alignment alignment_;

  Dimensions measured_;

  // Captured dx_ and dy_ during drag and scroll animations.
  XDim dxStart_;
  YDim dyStart_;

  Widget* contents_;

  unsigned long scroll_start_time_;
  unsigned long scroll_end_time_;
  float scroll_start_vx_;  // initial scroll velocity in pixels/s.
  float scroll_start_vy_;  // initial scroll velocity in pixels/s.
  float scroll_decel_x_;
  float scroll_decel_y_;

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
};

}  // namespace roo_windows
