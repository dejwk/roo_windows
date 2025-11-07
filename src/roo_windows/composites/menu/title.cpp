#include "roo_windows/composites/menu/title.h"

#include "roo_icons/outlined/navigation.h"
#include "roo_windows/core/task.h"

namespace roo_windows {
namespace menu {

Title::Title(const Environment& env, std::string title)
    : HorizontalLayout(env),
      back_(env, SCALED_ROO_ICON(outlined, navigation_arrow_back)),
      label_(env, std::move(title), font_h6(), kGravityLeft | kGravityMiddle) {
  setGravity(kGravityMiddle);
  label_.setMargins(MarginSize::NONE);
  label_.setPadding(PaddingSize::TINY);
  add(back_);
  add(label_, {weight : 1});
  back_.setOnInteractiveChange([&]() { getTask()->exitActivity(); });
}

}  // namespace menu
}  // namespace roo_windows
