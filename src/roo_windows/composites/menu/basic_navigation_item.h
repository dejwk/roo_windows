#pragma once

#include "roo_display/core/utf8.h"
#include "roo_display/image/image.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/text_label.h"

// Clickable option with single-line text label and an optional icon.
// Note: the text label is expected to be a constant.

namespace roo_windows {
namespace menu {

class BasicNavigationItem : public HorizontalLayout {
 public:
  /// Builds a single-line navigation row with the supplied icon and label;
  /// clicking the row enters `target`.
  BasicNavigationItem(const Environment& env,
                      const roo_display::Pictogram& icon, roo::string_view text,
                      Activity& target);

  bool isClickable() const override { return true; }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::WrapContentHeight());
  }

 private:
  Icon icon_;
  StringViewLabel label_;
  Activity& target_;
};

/// Two-line navigation row: icon plus a primary label stacked above a
/// secondary subtext line. Like `BasicNavigationItem`, clicking enters the
/// supplied target activity.
class BasicNavigationItemWithSubtext : public HorizontalLayout {
 public:
  /// Builds a navigation row with an icon, a primary label and a secondary
  /// subtext line; clicking the row enters `target`.
  BasicNavigationItemWithSubtext(const Environment& env,
                                 const roo_display::Pictogram& icon,
                                 roo::string_view label,
                                 roo::string_view subtext, Activity& target);

  bool isClickable() const override { return true; }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::WrapContentHeight());
  }

 private:
  Icon icon_;
  VerticalLayout content_;
  StringViewLabel label_;
  StringViewLabel subtext_;
  Activity& target_;
};

}  // namespace menu
}  // namespace roo_windows
