#pragma once

#include <vector>

#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/menu/menu_tokens.h"

namespace roo_windows::material3::internal {

/// Vertical group stack owned by one menu panel.
class MenuGroupStack final : public Container {
 public:
  explicit MenuGroupStack(ApplicationContext& context);
  ~MenuGroupStack() override;

  void addGroup(MenuGroup& group);
  void addGroup(std::unique_ptr<MenuGroup> group);
  void clearGroups();
  int groupCount() const;
  void setSeparatorMode(MenuSeparatorMode mode, ListVariant variant);

 protected:
  void paint(PaintContext& ctx) const override;
  Color background() const override;
  bool fullyCoversBoundsWithOpaqueColors() const override;
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
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
  bool isScrolling() const;

  Color background() const override;
  BorderStyle getBorderStyle() const override;

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  const MenuTokens& tokens() const;

  MenuGroupStack groups_;
  SimpleScrollablePanel viewport_;
  MenuPolicy policy_;
  bool scrolling_ = false;
};

/// Transparent full-window owner of all currently visible menu panels.
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

 protected:
  void paint(PaintContext& ctx) const override;
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  std::vector<MenuPanel*> panels_;
};

}  // namespace roo_windows::material3::internal
