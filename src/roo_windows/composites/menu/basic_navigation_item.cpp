#include "roo_windows/composites/menu/basic_navigation_item.h"

namespace roo_windows {
namespace menu {

BasicNavigationItem::BasicNavigationItem(const Environment& env,
                                         const roo_display::Pictogram& icon,
                                         roo::string_view text,
                                         Activity& target)
    : HorizontalLayout(env),
      icon_(env, icon),
      label_(env, text, font_subtitle1()),
      target_(target) {
  setGravity(kGravityMiddle);
  add(icon_);
  label_.setMargins(MarginSize::NONE);
  label_.setPadding(PaddingSize::TINY);
  add(label_, {weight : 1});

  setOnInteractiveChange([&]() { getTask()->enterActivity(&target_); });
}

BasicNavigationItemWithSubtext::BasicNavigationItemWithSubtext(
    const Environment& env, const roo_display::Pictogram& icon,
    roo::string_view label, roo::string_view subtext, Activity& target)
    : HorizontalLayout(env),
      icon_(env, icon),
      content_(env),
      label_(env, label, font_subtitle1()),
      subtext_(env, subtext, font_subtitle2()),
      target_(target) {
  setGravity(kGravityMiddle);
  add(icon_);
  label_.setMargins(MarginSize::NONE);
  label_.setPadding(PaddingSize::NONE);
  subtext_.setMargins(MarginSize::NONE);
  subtext_.setPadding(PaddingSize::NONE);
  content_.setMargins(MarginSize::NONE);
  content_.setPadding(PaddingSize::TINY);
  content_.add(label_);
  content_.add(subtext_);
  add(content_, {weight : 1});

  setOnInteractiveChange([&]() { getTask()->enterActivity(&target_); });
}

}  // namespace menu
}  // namespace roo_windows
