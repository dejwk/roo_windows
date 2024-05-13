#pragma once

#include "roo_display/core/utf8.h"
#include "roo_windows/composites/menu/title.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/activity.h"
#include "roo_windows/core/widget.h"

namespace roo_windows {
namespace menu {

// You use this class to build your own menus. Subclass it, add child widgets,
// and override the constructor to add those widgets to the canvas (calling
// add(), using vertical layout options if needed).
class Menu : public roo_windows::Activity {
 public:
  Menu(const roo_windows::Environment& env, std::string title)
      : roo_windows::Activity(),
        pane_(env),
        container_(env, pane_),
        title_(env, std::move(title)) {
    pane_.add(title_);
  }

  void add(roo_windows::WidgetRef child,
           roo_windows::VerticalLayout::Params params =
               roo_windows::VerticalLayout::Params()) {
    pane_.add(std::move(child), params);
  }

  roo_windows::Widget& getContents() override { return container_; }

 private:
  class Container : public roo_windows::ScrollablePanel {
   public:
    using roo_windows::ScrollablePanel::ScrollablePanel;

    roo_windows::PreferredSize getPreferredSize() const override {
      return roo_windows::PreferredSize(
          roo_windows::PreferredSize::MatchParentWidth(),
          roo_windows::PreferredSize::WrapContentHeight());
    }
  };

  class Pane : public roo_windows::VerticalLayout {
    using roo_windows::VerticalLayout::VerticalLayout;

    roo_windows::PreferredSize getPreferredSize() const override {
      return roo_windows::PreferredSize(
          roo_windows::PreferredSize::MatchParentWidth(),
          roo_windows::PreferredSize::WrapContentHeight());
    }
  };

  Pane pane_;
  Container container_;
  Title title_;
};

}  // namespace menu
}  // namespace roo_windows
