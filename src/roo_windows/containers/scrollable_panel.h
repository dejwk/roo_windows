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

class ScrollablePanel : public Panel, private roo_scheduler::Executable {
 public:
  enum Direction { VERTICAL = 0, HORIZONTAL = 1, BOTH = 2 };

  ScrollablePanel(const Environment& env, WidgetRef contents,
                  Direction direction = VERTICAL)
      : ScrollablePanel(env, direction) {
    setContents(std::move(contents));
  }

  ScrollablePanel(const Environment& env, Direction direction = VERTICAL)
      : Panel(env),
        direction_(direction),
        alignment_(roo_display::kLeft | roo_display::kTop),
        scroll_start_vx_(0.0),
        scroll_start_vy_(0.0),
        scroll_bar_presence_(VerticalScrollBar::ALWAYS_HIDDEN),
        scroll_bar_(env),
        scroll_bar_gesture_(false) {
    scroll_bar_.setVisibility(INVISIBLE);
  }

  void setContents(WidgetRef new_contents) {
    if (contents() == &*new_contents &&
        contents()->isOwnedByParent() == new_contents.is_owned()) {
      return;
    }
    removeAll();
    add(std::move(new_contents));
    add(scroll_bar_);
  }

  void clearContents() { removeAll(); }

  void setAlign(roo_display::Alignment alignment) {
    alignment_ = alignment;
    update();
  }

  void setVerticalScrollBarPresence(VerticalScrollBar::Presence presence) {
    if (scroll_bar_presence_ = presence) return;
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

  Widget* contents() { return children_.empty() ? nullptr : children_[0]; }
  const Widget* contents() const { return children_[0]; }

  // Sets the relative position of the underlying content, relative to the the
  // visible rectangle.
  void scrollTo(XDim x, YDim y);

  // Adjusts the relative position of the underlying content by the specified
  // offset.
  void scrollBy(XDim dx, YDim dy) {
    if (contents() == nullptr) return;
    scrollTo(dx + contents()->xOffset(), dy + contents()->yOffset());
  }

  void scrollToTop() {
    if (contents() == nullptr) return;
    scrollTo(contents()->xOffset(), 0);
  }

  void scrollToBottom();

  void update() { scrollBy(0, 0); }

  void paintWidgetContents(const Canvas& canvas, Clipper& clipper) override;

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
  void execute(roo_scheduler::EventID id) override {
    if (roo_time::Uptime::Now() >= deadline_hide_scrollbar_) {
      scroll_bar_.setVisibility(INVISIBLE);
    }
  }

  void scrollVertically(YDim yscroll);

  Direction direction_;
  roo_display::Alignment alignment_;

  Dimensions measured_;

  // Captured dx_ and dy_ during drag and scroll animations.
  XDim dxStart_;
  YDim dyStart_;

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

  // Whether the 'down' event has been confirmed as a touch of the scroll bar in
  // its active region.
  bool is_scroll_bar_scrolled_;
};

}  // namespace roo_windows
