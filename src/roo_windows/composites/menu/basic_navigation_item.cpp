#include "roo_windows/composites/menu/basic_navigation_item.h"

namespace roo_windows {
namespace menu {

BasicNavigationItem::BasicNavigationItem(const roo_windows::Environment& env,
                                         const roo_display::Pictogram& icon,
                                         roo::string_view text,
                                         roo_windows::Activity& target)
    : roo_windows::HorizontalLayout(env),
      icon_(env, icon),
      label_(env, text, roo_windows::font_subtitle1()),
      target_(target) {
  setGravity(roo_windows::kGravityMiddle);
  add(icon_);
  label_.setMargins(roo_windows::MARGIN_NONE);
  label_.setPadding(roo_windows::PADDING_TINY);
  add(label_, { weight : 1 });

  setOnInteractiveChange([&]() { getTask()->enterActivity(&target_); });
}

BasicNavigationItemWithSubtext::BasicNavigationItemWithSubtext(
    const roo_windows::Environment& env, const roo_display::Pictogram& icon,
    roo::string_view label, roo::string_view subtext,
    roo_windows::Activity& target)
    : roo_windows::HorizontalLayout(env),
      icon_(env, icon),
      content_(env),
      label_(env, label, roo_windows::font_subtitle1()),
      subtext_(env, subtext, roo_windows::font_subtitle2()),
      target_(target) {
  setGravity(roo_windows::kGravityMiddle);
  add(icon_);
  label_.setMargins(roo_windows::MARGIN_NONE);
  label_.setPadding(roo_windows::PADDING_NONE);
  subtext_.setMargins(roo_windows::MARGIN_NONE);
  subtext_.setPadding(roo_windows::PADDING_NONE);
  content_.setMargins(roo_windows::MARGIN_NONE);
  content_.setPadding(roo_windows::PADDING_TINY);
  content_.add(label_);
  content_.add(subtext_);
  add(content_, { weight : 1 });

  setOnInteractiveChange([&]() { getTask()->enterActivity(&target_); });
}

}  // namespace menu
}  // namespace roo_windows
