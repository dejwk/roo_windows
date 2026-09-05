#pragma once

#include <stdint.h>

#include <memory>
#include <type_traits>
#include <utility>

#include "roo_display/core/utf8.h"
#include "roo_windows/core/layout_direction.h"
#include "roo_windows/core/transient_presentation.h"
#include "roo_windows/material3/badge/badge.h"
#include "roo_windows/material3/list/list.h"

namespace roo_display {
class Drawable;
}

namespace roo_windows {
class Task;
}

namespace roo_windows::material3 {

class Menu;
class MenuLevelBuilder;

/// Selects the Material 3 expressive menu color family.
enum class MenuColorStyle : uint8_t { kStandard, kVibrant };

/// Selects what separates adjacent menu groups.
enum class MenuSeparatorMode : uint8_t { kNone, kDivider, kGap };

/// Overrides whether invoking a leaf closes its menu chain.
enum class MenuLeafDismissal : uint8_t { kDefault, kDismiss, kKeepOpen };

/// Placement preference for an anchored root menu.
enum class MenuPlacement : uint8_t {
  kBelowStart,
  kBelowEnd,
  kAboveStart,
  kAboveEnd,
  kBefore,
  kAfter,
};

/// Result of attempting to present a menu.
enum class MenuShowResult : uint8_t {
  kShown,
  kHostBusy,
  kAlreadyPresented,
  kReentrantReplacement,
  kInteractionOwnerUnavailable,
  kSurfaceUnavailable,
  kAnchorUnavailable,
  kUnimplemented,
};

/// Optional source copied synchronously to retain a trigger's pressed paint.
struct MenuTriggerPaintSource {
  const Widget& widget;
  uint16_t corner_radius = 0;
  uint32_t overlay_argb = 0;
  uint8_t overlay_opacity = 0;
};

/// Presentation and selection policy shared by one menu chain.
struct MenuPolicy {
  ListVariant variant = ListVariant::kExpressive;
  MenuColorStyle color_style = MenuColorStyle::kStandard;
  MenuSeparatorMode separator_mode = MenuSeparatorMode::kNone;
  SelectionMode selection_mode = SelectionMode::kNone;
  LayoutDirection layout_direction = LayoutDirection::kLeftToRight;
};

/// Badge content exposed by a menu item to its bound row.
struct MenuBadgeSpec {
  BadgeMode mode = BadgeMode::kHidden;
  roo::string_view text = {};
};

/// Optional owner-painted content in a menu row's trailing lane.
struct MenuTrailingAffordances {
  roo_display::StringView shortcut = {};
  const roo_display::Drawable* icon = nullptr;
  MenuBadgeSpec badge = {};
};

/// Semantic item contract consumed by `MenuEntry`.
class MenuItem : public ListItem {
 public:
  /// Returns whether this item can receive focus and activation.
  virtual bool isEnabled() const { return true; }

  /// Returns whether this item participates in menu selection.
  virtual bool isSelectable() const { return false; }

  /// Returns the current item-owned selection state.
  virtual bool isSelected() const { return false; }

  /// Applies a selection mutation requested by the menu presenter.
  virtual void setSelectedFromMenu(bool selected) { (void)selected; }

  /// Returns this leaf's dismissal override.
  virtual MenuLeafDismissal leafDismissal() const {
    return MenuLeafDismissal::kDefault;
  }

  /// Returns optional content painted by the bound row's trailing lane.
  virtual MenuTrailingAffordances trailingAffordances() const { return {}; }

  /// Returns whether activating this item should populate a child level.
  virtual bool hasSubmenu() const { return false; }

  /// Synchronously populates a scoped child level.
  virtual void populateSubmenu(MenuLevelBuilder& builder) { (void)builder; }

  /// Runs the semantic action for an invoked leaf.
  virtual void onInvoked() {}
};

/// Construction-time stable borrows and initial state for a standard item.
struct StandardMenuItemInit {
  roo_display::StringView headline = {};
  roo_display::StringView supporting = {};
  Widget* leading = nullptr;
  bool enabled = true;
  bool selectable = false;
  bool selected = false;
};

/// Standard text-and-leading-visual Material 3 menu item.
class StandardMenuItem : public MenuItem {
 public:
  /// Creates an item. Text and `leading` are stable caller-owned borrows.
  explicit StandardMenuItem(const StandardMenuItemInit& init = {});

  /// Releases optional trailing payload storage.
  ~StandardMenuItem() override;

  roo::string_view headlineText() const override;
  roo::string_view supportingText() const override;
  Widget* leading() override;
  const Widget* leading() const override;
  bool isEnabled() const override;
  bool isSelectable() const override;
  bool isSelected() const override;
  void setSelectedFromMenu(bool selected) override;
  MenuTrailingAffordances trailingAffordances() const override;

  /// Sets the item-owned selected state.
  void setSelected(bool selected);

  /// Sets whether the item is eligible for focus and activation.
  void setEnabled(bool enabled);

  /// Borrows shortcut text until it is replaced, cleared, or this item dies.
  void setShortcut(roo_display::StringView shortcut);

  /// Removes shortcut text and releases its payload when otherwise empty.
  void clearShortcut();

  /// Shows a dot badge.
  void setBadgeDot();

  /// Borrows badge text until it is replaced, cleared, or this item dies.
  void setBadgeText(roo::string_view text);

  /// Formats a bounded numeric badge value owned by the optional payload.
  void setBadgeValue(unsigned int value);

  /// Removes the badge and releases its payload when otherwise empty.
  void clearBadge();

  /// Borrows a trailing drawable until replaced, cleared, or destruction.
  void setTrailingIcon(const roo_display::Drawable* icon);

  /// Removes the trailing drawable.
  void clearTrailingIcon();

 private:
  struct TrailingPayload;

  TrailingPayload& ensureTrailingPayload();
  void releaseEmptyTrailingPayload();

  roo::string_view headline_;
  roo::string_view supporting_;
  Widget* leading_;
  std::unique_ptr<TrailingPayload> trailing_;
  uint8_t enabled_ : 1;
  uint8_t selectable_ : 1;
  uint8_t selected_ : 1;
};

/// List-backed Material 3 menu row with owner-painted trailing adornments.
class MenuEntry : public ListEntry {
 public:
  /// Creates an empty menu row.
  explicit MenuEntry(ApplicationContext& context);

  /// Unbinds the borrowed item before row storage is released.
  ~MenuEntry() override;

  /// Binds a stable, non-owning menu item to this row.
  void setMenuItem(MenuItem& item);

  /// Returns the bound menu item, or nullptr.
  MenuItem* menuItem();

  /// Returns the bound menu item, or nullptr.
  const MenuItem* menuItem() const;

  /// Returns true when a bound, enabled item can be activated.
  bool isClickable() const override;

  /// Handles level-local traversal and submenu navigation keys.
  bool onKeyEvent(const KeyEvent& event) override;

 protected:
  /// Clears the base binding before a derived inline item is destroyed.
  void prepareForItemDestruction();

  void onSingleTapUp(XDim x, YDim y) override;
  void onClicked() override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;
  void paintWidgetContents(PaintContext& ctx) override;

 private:
  friend class Menu;

  struct AdornmentState;

  using ListEntry::clearItem;
  using ListEntry::setItem;

  void syncAdornments();
  int16_t trailingLaneWidth() const;
  void bindToMenu(Menu& owner, uint8_t level, uint16_t row,
                  uint16_t generation);
  void unbindFromMenu();

  std::unique_ptr<AdornmentState> adornments_;
  Menu* menu_ = nullptr;
  uint16_t level_generation_ = 0;
  uint16_t row_ = 0;
  uint8_t level_ = 0;
  bool submenu_allowed_ = true;
  bool suppress_next_click_dispatch_ = false;
};

/// Menu row that owns its item inline and destroys the binding first.
template <typename Item>
class MenuRow : public MenuEntry {
 public:
  static_assert(std::is_base_of<MenuItem, Item>::value,
                "MenuRow Item must derive from MenuItem");

  /// Constructs the inline item and binds it to this row.
  template <typename... Args>
  explicit MenuRow(ApplicationContext& context, Args&&... args)
      : MenuEntry(context), item_(std::forward<Args>(args)...) {
    setMenuItem(item_);
  }

  /// Clears the binding before C++ destroys the inline item member.
  ~MenuRow() override { prepareForItemDestruction(); }

  /// Returns the inline item.
  Item& item() { return item_; }

  /// Returns the inline item.
  const Item& item() const { return item_; }

 private:
  Item item_;
};

/// Ordered group of borrowed or adopted menu rows.
class MenuGroup final : public Container {
 public:
  /// Creates an empty, transparent group.
  explicit MenuGroup(ApplicationContext& context);

  /// Detaches all remaining rows, deleting rows adopted by `add()`.
  ~MenuGroup() override;

  /// Appends a detached caller-owned row.
  void add(MenuEntry& entry);

  /// Appends and adopts a detached row.
  void add(std::unique_ptr<MenuEntry> entry);

  /// Detaches all rows and deletes adopted rows.
  void clear();

  /// Returns the first enabled row, or null when the group has none.
  Widget* preferredFocusChild() override;

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
  friend class Menu;

  std::vector<MenuEntry*> entries_;
};

/// Scoped, synchronous target used while one submenu level is populated.
class MenuLevelBuilder {
 public:
  MenuLevelBuilder(const MenuLevelBuilder&) = delete;
  MenuLevelBuilder& operator=(const MenuLevelBuilder&) = delete;
  MenuLevelBuilder(MenuLevelBuilder&&) = delete;
  MenuLevelBuilder& operator=(MenuLevelBuilder&&) = delete;

  /// Adds a borrowed group to the child level being populated.
  void addGroup(MenuGroup& group);

  /// Adds and adopts a group in the child level being populated.
  void addGroup(std::unique_ptr<MenuGroup> group);

 private:
  friend class Menu;
  MenuLevelBuilder(Menu& owner, uint8_t level, uint16_t generation);

  Menu* owner_;
  uint16_t generation_;
  uint8_t level_;
};

/// Reusable presenter and persistent root structure for one menu chain.
class Menu {
 public:
  /// Creates an idle menu using `context` for its persistent widget tree.
  explicit Menu(ApplicationContext& context);

  /// Cancels an active presentation and releases all persistent structure.
  virtual ~Menu();

  /// Replaces the policy used by the next presentation.
  void setPolicy(const MenuPolicy& policy);

  /// Adds a detached borrowed group to the persistent root panel.
  void addGroup(MenuGroup& group);

  /// Adds and adopts a group in the persistent root panel.
  void addGroup(std::unique_ptr<MenuGroup> group);

  /// Detaches all persistent root groups.
  void clearGroups();

  /// Presents below or beside an attached placement source.
  MenuShowResult show(::roo_windows::Task& interaction_owner,
                      const Widget& placement_source,
                      MenuPlacement placement = MenuPlacement::kBelowStart,
                      const MenuTriggerPaintSource* trigger = nullptr);

  /// Presents relative to a rectangle already expressed in window coordinates.
  MenuShowResult showFromRect(
      ::roo_windows::Task& interaction_owner, const Rect& bounds_in_window,
      MenuPlacement placement = MenuPlacement::kBelowStart,
      const MenuTriggerPaintSource* trigger = nullptr);

  /// Recaptures a live placement source for an active menu.
  bool reanchor(const Widget& placement_source,
                MenuPlacement placement = MenuPlacement::kBelowStart,
                const MenuTriggerPaintSource* trigger = nullptr);

  /// Reanchors an active menu to a window-coordinate rectangle.
  bool reanchorFromRect(const Rect& bounds_in_window,
                        MenuPlacement placement = MenuPlacement::kBelowStart,
                        const MenuTriggerPaintSource* trigger = nullptr);

  /// Dismisses the complete chain as a cancellation.
  void dismissChain();

 protected:
  /// Detaches persistent borrowed members before a derived destructor runs.
  void prepareForDerivedDestruction();

  /// Receives terminal delivery only after host and menu structure detach.
  virtual void onFinished(PresentationFinishReason reason) { (void)reason; }

 private:
  friend class MenuLevelBuilder;
  friend class MenuEntry;

  void bindRootEntries();
  void bindLevelEntries(uint8_t level);
  void unbindLevelEntries(uint8_t level);
  void unbindAllEntries();
  bool handleEntryKey(MenuEntry& entry, const KeyEvent& event);
  bool closeDeepestLevel();
  void closeLevelsFrom(uint8_t first_level, bool restore_focus);
  void openSubmenu(MenuEntry& entry, MenuItem& item, uint8_t level,
                   uint16_t row, uint16_t generation);
  void invokeEntry(MenuEntry& entry, uint8_t level, uint16_t row,
                   uint16_t generation);

  MenuShowResult showCaptured(::roo_windows::Task& interaction_owner,
                              const Rect& bounds_in_window,
                              MenuPlacement placement,
                              const MenuTriggerPaintSource* trigger);

  class Impl;
  std::unique_ptr<Impl> impl_;
  uint8_t admission_in_progress_ : 1;
};

}  // namespace roo_windows::material3
