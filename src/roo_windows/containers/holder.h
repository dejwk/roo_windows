#pragma once

#include "roo_display/ui/alignment.h"
#include "roo_scheduler.h"
#include "roo_time.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/panel.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {

// Wraps a single child, which can be swapped out. Override methods to customize
// view, e.g. add elevation, custom padding, margins, etc.
class Holder : public Container {
 public:
  Holder(ApplicationContext& context, WidgetRef contents) : Holder(context) {
    setContents(std::move(contents));
  }

  Holder(ApplicationContext& context) : Container(context), contents_(nullptr) {}

  /// Replaces (or clears, if `new_contents` is null) the single held child.
  /// No-op if the same widget with the same ownership is reassigned.
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

  /// Delegates measurement to the held child and caches the result.
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

  /// Lays out the held child to fill `rect`.
  void onLayout(bool changed, const Rect& rect) override;

 private:
  Dimensions measured_;

  Widget* contents_;
};

}  // namespace roo_windows
