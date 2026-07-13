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
  Menu(roo_windows::ApplicationContext& context, std::string title)
      : roo_windows::Activity(),
        pane_(context),
        container_(context, pane_),
        title_(context, std::move(title)) {
    pane_.add(title_);
  }

  /// Adds a child widget below the title with the supplied vertical layout
  /// params.
  void add(roo_windows::WidgetRef child,
           roo_windows::VerticalLayout::Params params =
               roo_windows::VerticalLayout::Params()) {
    pane_.add(std::move(child), params);
  }

  /// Returns the scrollable container that hosts the menu's contents.
  roo_windows::Widget& getContents() override { return container_; }

 private:
  class Container : public roo_windows::ScrollablePanel {
   public:
    using roo_windows::ScrollablePanel::ScrollablePanel;

    bool onKeyEvent(const roo_windows::KeyEvent& event) override {
      if (event.phase == roo_windows::KeyPhase::kDown ||
          event.phase == roo_windows::KeyPhase::kRepeat) {
        roo_windows::FocusDirection direction;
        if (event.code == roo_windows::KeyCode::kUp) {
          direction = roo_windows::FocusDirection::kUp;
        } else if (event.code == roo_windows::KeyCode::kDown) {
          direction = roo_windows::FocusDirection::kDown;
        } else {
          return roo_windows::ScrollablePanel::onKeyEvent(event);
        }
        if (context().focus().moveFocusDirection(*this, direction)) return true;
      }
      return roo_windows::ScrollablePanel::onKeyEvent(event);
    }

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
