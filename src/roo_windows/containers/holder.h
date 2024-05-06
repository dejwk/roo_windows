#pragma once

#include "roo_display/ui/alignment.h"
#include "roo_scheduler.h"
#include "roo_time.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

// Wraps a single child, which can be swapped out. Override methods to customize
// view, e.g. add elevation, custom padding, margins, etc.
class Holder : public Container {
 public:
  Holder(const Environment& env, WidgetRef contents) : Holder(env) {
    setContents(std::move(contents));
  }

  Holder(const Environment& env) : Container(env), contents_(nullptr) {}

  void setContents(WidgetRef new_contents) {
    if (contents() == &*new_contents &&
        contents()->isOwnedByParent() == new_contents.is_owned()) {
      return;
    }
    if (contents_ != nullptr) {
      detachChild(contents_);
    }
    contents_ = new_contents.get();
    if (contents_ != nullptr) {
      attachChild(std::move(new_contents));
    }
  }

  void clearContents() { setContents(WidgetRef()); }
  bool hasContents() const { return contents_ != nullptr; }

  int getChildrenCount() const override { return hasContents() ? 1 : 0; }

  Widget& getChild(int idx) { return *contents_; }

  const Widget& getChild(int idx) const { return *contents_; }

  Widget* contents() { return contents_; }
  const Widget* contents() const { return contents_; }

 protected:
  PreferredSize getPreferredSize() const override;

  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  void onLayout(bool changed, const Rect& rect) override;

 private:
  Dimensions measured_;

  Widget* contents_;
};

}  // namespace roo_windows
