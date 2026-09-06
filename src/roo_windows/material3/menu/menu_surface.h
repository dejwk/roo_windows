#pragma once

#include <vector>

#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/menu/menu_tokens.h"

namespace roo_windows::material3::internal {

/// Transparent scrolling viewport that leaves menu-surface paint to its panel.
class MenuViewport final : public SimpleScrollablePanel {
 public:
  using SimpleScrollablePanel::SimpleScrollablePanel;

 protected:
  Color background() const override { return roo_display::color::Transparent; }
  bool fullyCoversBoundsWithOpaqueColors() const override { return false; }
  Rect getDirectPaintExclusionBounds() const override { return Rect(); }
  void paint(PaintContext& ctx) const override { (void)ctx; }
};

/// Vertical group stack owned by one menu panel.
class MenuGroupStack final : public Container {
 public:
  explicit MenuGroupStack(ApplicationContext& context);
  ~MenuGroupStack() override;

  void addGroup(MenuGroup& group);
  void addGroup(std::unique_ptr<MenuGroup> group);
  void clearGroups();
  int groupCount() const;
  MenuGroup& groupAt(int idx);
  const MenuGroup& groupAt(int idx) const;
  void setSeparatorMode(MenuSeparatorMode mode, ListVariant variant);
  MenuSeparatorMode separatorMode() const;
  Widget* preferredFocusChild() override;
  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::WrapContentHeight());
  }

 protected:
  void paint(PaintContext& ctx) const override;
  Color background() const override;
  bool fullyCoversBoundsWithOpaqueColors() const override;
  Rect getDirectPaintExclusionBounds() const override { return Rect(); }
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  friend class MenuPanel;

  void setResolvedSeparatorMode(MenuSeparatorMode mode, ListVariant variant);

  std::vector<MenuGroup*> groups_;
  MenuSeparatorMode separator_mode_ = MenuSeparatorMode::kNone;
  ListVariant variant_ = ListVariant::kExpressive;
};

/// Surface and persistent scrolling viewport for one menu level.
class MenuPanel final : public Container {
 public:
  explicit MenuPanel(ApplicationContext& context);
  ~MenuPanel() override;

  void setPolicy(const MenuPolicy& policy);
  void addGroup(MenuGroup& group);
  void addGroup(std::unique_ptr<MenuGroup> group);
  void clearGroups();
  int groupCount() const;
  MenuGroup& groupAt(int idx);
  const MenuGroup& groupAt(int idx) const;
  bool isScrolling() const;

  /// Returns the separator actually used after scrolling coercion.
  MenuSeparatorMode effectiveSeparatorMode() const;

  Color background() const override;
  ColorToken containerRole() const override;
  BorderStyle getBorderStyle() const override;
  uint8_t getElevation() const override;
  bool isFocusable() const override { return false; }
  Widget* preferredFocusChild() override;

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  const MenuTokens& tokens() const;

  MenuGroupStack groups_;
  MenuViewport viewport_;
  MenuPolicy policy_;
  bool scrolling_ = false;
};

/// Transparent full-window owner of all currently visible menu panels.
///
/// It intentionally inherits the null preferred-focus target: entering the
/// menu scope captures traversal without focusing a row until Tab is pressed.
class MenuOverlay final : public Container {
 public:
  explicit MenuOverlay(ApplicationContext& context);
  ~MenuOverlay() override;

  void addPanel(MenuPanel& panel, const Rect& bounds);
  void setPanelBounds(MenuPanel& panel, const Rect& bounds);
  void removePanel(MenuPanel& panel);
  void clearPanels();

  Color background() const override;
  bool fullyCoversBoundsWithOpaqueColors() const override;
  bool fillTouchTargetPath(XDim x, YDim y, std::vector<Widget*>& path) override;
  bool isFocusable() const override { return false; }

 protected:
  void paint(PaintContext& ctx) const override;
  Rect getDirectPaintExclusionBounds() const override { return Rect(); }
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  std::vector<MenuPanel*> panels_;
};

}  // namespace roo_windows::material3::internal
