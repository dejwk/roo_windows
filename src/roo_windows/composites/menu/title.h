#pragma once

#include <string>

#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_windows {
namespace menu {

// The title of the activity, with the back button.
class Title : public roo_windows::HorizontalLayout {
 public:
  Title(const roo_windows::Environment& env, std::string title);

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

  void setTitle(std::string title) { label_.setText(std::move(title)); }

 private:
  roo_windows::Icon back_;
  roo_windows::TextLabel label_;
};

}  // namespace menu
}  // namespace roo_toolkit
