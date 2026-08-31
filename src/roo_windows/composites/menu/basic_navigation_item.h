#pragma once

#include "roo_display/core/utf8.h"
#include "roo_display/image/image.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/destination.h"
#include "roo_windows/core/navigation_host.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/text_label.h"

// Clickable option with single-line text label and an optional icon.
// Note: the text label is expected to be a constant.

namespace roo_windows {
namespace menu {

class BasicNavigationItem : public FlexLayout {
 public:
  /// Builds a single-line navigation row with the supplied icon and label;
  /// clicking the row enters `target`.
  BasicNavigationItem(ApplicationContext& context,
                      const roo_display::Pictogram& icon, roo::string_view text,
                      NavigationHost& navigation, Destination& target);

  bool isClickable() const override { return true; }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::WrapContentHeight());
  }

 private:
  Icon icon_;
  StringViewLabel label_;
  NavigationHost& navigation_;
  Destination& target_;
};

/// Two-line navigation row: icon plus a primary label stacked above a
/// secondary subtext line. Like `BasicNavigationItem`, clicking enters the
/// supplied target activity.
class BasicNavigationItemWithSubtext : public FlexLayout {
 public:
  /// Builds a navigation row with an icon, a primary label and a secondary
  /// subtext line; clicking the row enters `target`.
  BasicNavigationItemWithSubtext(ApplicationContext& context,
                                 const roo_display::Pictogram& icon,
                                 roo::string_view label,
                                 roo::string_view subtext,
                                 NavigationHost& navigation,
                                 Destination& target);

  bool isClickable() const override { return true; }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::WrapContentHeight());
  }

 private:
  Icon icon_;
  FlexLayout content_;
  StringViewLabel label_;
  StringViewLabel subtext_;
  NavigationHost& navigation_;
  Destination& target_;
};

}  // namespace menu
}  // namespace roo_windows
