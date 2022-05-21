#pragma once

#include "roo_windows/core/environment.h"
#include "roo_windows/core/margins.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

// A single-child container that wraps a border of specified width around the
// child widget. The color of the border can be set by setting the background
// color.
class BorderedPanel : public Panel {
 public:
  BorderedPanel(const Environment& env) : Panel(env) {}

  void setBorder(Margins border) {
    border_ = border;
    requestLayout();
  }

  void setContents(WidgetRef contents) {
    removeAll();
    add(std::move(contents),
        Box(border_.left(), border_.top(), width() - border_.right(),
            height() - border_.bottom()));
  }

  Widget* contents() { return children_[0]; }

  const Widget& contents() const { return *children_[0]; }

 protected:
  PreferredSize getPreferredSize() const override;
  Dimensions onMeasure(MeasureSpec width, MeasureSpec height) override;
  void onLayout(bool changed, const roo_display::Box& box) override;

 private:
  Margins border_;
};

}  // namespace roo_windows
