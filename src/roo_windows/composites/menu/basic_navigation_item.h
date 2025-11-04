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

class BasicNavigationItem : public roo_windows::HorizontalLayout {
 public:
  BasicNavigationItem(const roo_windows::Environment& env,
                      const roo_display::Pictogram& icon, roo::string_view text,
                      roo_windows::Activity& target);

  bool isClickable() const override { return true; }

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

 private:
  roo_windows::Icon icon_;
  roo_windows::StringViewLabel label_;
  roo_windows::Activity& target_;
};

class BasicNavigationItemWithSubtext : public roo_windows::HorizontalLayout {
 public:
  BasicNavigationItemWithSubtext(const roo_windows::Environment& env,
                                 const roo_display::Pictogram& icon,
                                 roo::string_view label,
                                 roo::string_view subtext,
                                 roo_windows::Activity& target);

  bool isClickable() const override { return true; }

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

 private:
  roo_windows::Icon icon_;
  roo_windows::VerticalLayout content_;
  roo_windows::StringViewLabel label_;
  roo_windows::StringViewLabel subtext_;
  roo_windows::Activity& target_;
};

}  // namespace menu
}  // namespace roo_windows
