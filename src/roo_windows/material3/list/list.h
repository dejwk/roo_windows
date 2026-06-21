#pragma once

#include <stdint.h>

#include <functional>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "roo_display/core/utf8.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/widget_ref.h"
#include "roo_windows/material3/checkbox/checkbox.h"
#include "roo_windows/material3/radio_button/radio_button.h"
#include "roo_windows/material3/switch/switch.h"
#include "roo_windows/widgets/icon.h"

namespace roo_windows {

class StringViewLabel;

namespace material3 {

class AvatarVisual;

/// Selects the Material 3 list visual family.
enum class ListVariant : uint8_t {
  kBaseline,
  kExpressive,
};

/// Selects the Material 3 list row grouping style.
enum class ListStyle : uint8_t {
  kStandard,
  kSegmented,
};

/// Selects how list items participate in selection.
enum class SelectionMode : uint8_t {
  kNone,
  kSingle,
  kMultiple,
};

/// Selects the default visual affordance used for selected rows.
enum class SelectionAffordance : uint8_t {
  kNone,
  kCheckmark,
  kCheckbox,
  kRadio,
  kSwitch,
};

/// Selects where a selection affordance is placed by convenience layers.
enum class AffordancePlacement : uint8_t {
  kLeading,
  kTrailing,
};

/// Selects the list-owned divider treatment.
enum class DividerMode : uint8_t {
  kNone,
  kFullWidth,
  kInset,
};

/// Selects how a text slot handles content that exceeds its line budget.
enum class TextOverflowPolicy : uint8_t {
  kWrap,
  kTruncate,
};

/// Selects vertical alignment for leading and trailing row visuals.
enum class VerticalVisualAlignment : uint8_t {
  kMiddle,
  kTop,
};

/// Describes a row's position within its current visual group.
enum class ListItemPosition : uint8_t {
  kSingle,
  kFirst,
  kMiddle,
  kLast,
};

/// Text wrapping and truncation policy for a single list text slot.
struct ListTextPolicy {
  TextOverflowPolicy overflow = TextOverflowPolicy::kTruncate;
  uint8_t max_lines = 1;
};

/// List-level selection policy resolved into per-entry visual context.
struct ListSelectionPolicy {
  SelectionMode mode = SelectionMode::kNone;
  SelectionAffordance affordance = SelectionAffordance::kNone;
  AffordancePlacement placement = AffordancePlacement::kTrailing;
  bool selection_follows_press = true;
};

/// List-level divider policy.
struct ListDividerPolicy {
  DividerMode mode = DividerMode::kNone;
  uint8_t start_inset = 0;
  uint8_t end_inset = 0;
  bool suppress_between_selected = true;
};

/// Per-item hint used when resolving inset dividers.
struct DividerInsetHint {
  uint8_t start_inset = 0;
  uint8_t end_inset = 0;
};

/// Fully resolved visual context supplied to a row by its owning list.
struct ListEntryVisualContext {
  ListVariant variant = ListVariant::kExpressive;
  ListStyle style = ListStyle::kStandard;
  ListItemPosition position = ListItemPosition::kSingle;
  bool selected = false;
  bool enabled = true;
  bool pressed = false;
  bool focused = false;
  bool hovered = false;
  bool show_divider = false;
  DividerMode divider_mode = DividerMode::kNone;
  uint8_t divider_start_inset = 0;
  uint8_t divider_end_inset = 0;
};

/// Construction-time descriptor for `StandardListItem`.
struct StandardListItemInit {
  roo::string_view overline = {};
  roo::string_view headline = {};
  roo::string_view supporting = {};
  ListTextPolicy overline_policy = {};
  ListTextPolicy headline_policy = {};
  ListTextPolicy supporting_policy = {};
  Widget* leading = nullptr;
  Widget* trailing = nullptr;
  Widget* body = nullptr;
  VerticalVisualAlignment leading_alignment = VerticalVisualAlignment::kMiddle;
  VerticalVisualAlignment trailing_alignment = VerticalVisualAlignment::kMiddle;
  DividerInsetHint divider_inset_hint;
  bool prefer_top_text_alignment = false;

  /// Builds a one-line standard list item descriptor.
  static StandardListItemInit OneLine(roo::string_view headline,
                                      Widget* leading = nullptr,
                                      Widget* trailing = nullptr);

  /// Builds a two-line standard list item descriptor.
  static StandardListItemInit TwoLine(roo::string_view headline,
                                      roo::string_view supporting,
                                      Widget* leading = nullptr,
                                      Widget* trailing = nullptr);

  /// Builds a three-line standard list item descriptor.
  static StandardListItemInit ThreeLine(roo::string_view headline,
                                        roo::string_view supporting,
                                        roo::string_view overline = {},
                                        Widget* leading = nullptr,
                                        Widget* trailing = nullptr,
                                        Widget* body = nullptr);
};

/// Pure-virtual content contract consumed by `ListEntry`.
class ListItem {
 public:
  virtual ~ListItem() = default;

  /// Returns non-owning overline text for this item.
  virtual roo::string_view overlineText() const { return {}; }

  /// Returns non-owning headline text for this item.
  virtual roo::string_view headlineText() const { return {}; }

  /// Returns non-owning supporting text for this item.
  virtual roo::string_view supportingText() const { return {}; }

  /// Returns overline text wrapping and truncation policy.
  virtual ListTextPolicy overlinePolicy() const { return ListTextPolicy{}; }

  /// Returns headline text wrapping and truncation policy.
  virtual ListTextPolicy headlinePolicy() const { return ListTextPolicy{}; }

  /// Returns supporting text wrapping and truncation policy.
  virtual ListTextPolicy supportingPolicy() const { return ListTextPolicy{}; }

  /// Returns the borrowed leading slot widget, or nullptr.
  virtual Widget* leading() { return nullptr; }

  /// Returns the borrowed leading slot widget for read-only access.
  virtual const Widget* leading() const { return nullptr; }

  /// Returns the borrowed trailing slot widget, or nullptr.
  virtual Widget* trailing() { return nullptr; }

  /// Returns the borrowed trailing slot widget for read-only access.
  virtual const Widget* trailing() const { return nullptr; }

  /// Returns the borrowed body slot widget, or nullptr.
  virtual Widget* body() { return nullptr; }

  /// Returns the borrowed body slot widget for read-only access.
  virtual const Widget* body() const { return nullptr; }

  /// Returns the leading visual alignment policy.
  virtual VerticalVisualAlignment leadingAlignment() const {
    return VerticalVisualAlignment::kMiddle;
  }

  /// Returns the trailing visual alignment policy.
  virtual VerticalVisualAlignment trailingAlignment() const {
    return VerticalVisualAlignment::kMiddle;
  }

  /// Returns item-provided divider inset hints.
  virtual DividerInsetHint dividerInsetHint() const {
    return DividerInsetHint{};
  }

  /// Returns whether the text block prefers top alignment.
  virtual bool preferTopTextAlignment() const { return false; }

  /// Returns whether this item participates in row invocation.
  virtual bool isInvokable() const { return false; }

  /// Invokes the item's action when the bound row is activated.
  virtual void invoke() {}
};

/// Reusable expandable body container for list rows and other surfaces.
class ExpandablePanel : public Container {
 public:
  /// Creates an empty collapsed expandable panel.
  explicit ExpandablePanel(ApplicationContext& context);

  /// Sets the expandable content widget.
  void setContent(WidgetRef content);

  /// Clears the expandable content when none has been attached yet.
  void clearContent();

  /// Sets whether the panel is expanded.
  void setExpanded(bool expanded, bool animate = true);

  /// Returns whether the panel is currently expanded.
  bool isExpanded() const;

  /// Returns whether an expand/collapse animation is active.
  bool isAnimating() const;

  /// Sets the intended expand/collapse animation duration in milliseconds.
  void setAnimationDuration(uint16_t millis);

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;

 private:
  WidgetRef content_;
  uint16_t animation_duration_millis_;
  uint16_t animation_progress_millis_;
  bool expanded_;
};

/// Material 3 row surface that binds one stable `ListItem` at a time.
class ListEntry : public Container {
 public:
  /// Creates an empty Material 3 list row surface.
  explicit ListEntry(ApplicationContext& context);

  /// Detaches any stable slot children that are still bound.
  ~ListEntry() override;

  /// Returns whether an item is currently bound.
  bool hasItem() const;

  /// Returns the currently bound item, or nullptr.
  ListItem* item();

  /// Returns the currently bound item, or nullptr.
  const ListItem* item() const;

  /// Binds a non-owning item to this row.
  ///
  /// The item must outlive this entry or be cleared first. Binding is
  /// exclusive: an item can be attached to at most one entry at a time. Slot
  /// widget identity stays fixed for the lifetime of one binding.
  void setItem(ListItem& item);

  /// Clears the current binding and detaches any attached slot children.
  void clearItem();

  /// Rereads lightweight state from the currently bound item.
  ///
  /// This is an owner-driven refresh hook for mutable custom items. It never
  /// replaces bound slot widgets.
  void refreshFromItem();

  /// Stores list-resolved visual context for this entry.
  void setVisualContext(const ListEntryVisualContext& context);

  /// Returns the current list-resolved visual context.
  const ListEntryVisualContext& visualContext() const;

  /// Returns the row's Material 3 container color role.
  ColorRole containerRole() const override;

  /// Returns the current Material 3 row shape.
  BorderStyle getBorderStyle() const override;

  /// Returns the minimum dimensions for the bound item and slot widgets.
  Dimensions getSuggestedMinimumDimensions() const override;

  /// Returns whether this row should behave as a clickable surface.
  bool isClickable() const override;

  /// Invokes the currently bound item action, if one is available.
  void onClicked() override;

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  enum class TextSlotMode : uint8_t {
    kNone = 0,
    kLabel = 1,
    kBlock = 2,
  };

  void detachBoundChildren();
  void clearTextSlot(Widget*& slot, TextSlotMode& mode);
  void clearTextSlots();
  void syncTextSlotsFromItem();

  ListItem* item_;
  Widget* leading_child_;
  Widget* overline_text_;
  Widget* headline_text_;
  Widget* supporting_text_;
  Widget* trailing_child_;
  Widget* body_child_;
  TextSlotMode overline_mode_;
  TextSlotMode headline_mode_;
  TextSlotMode supporting_mode_;
  ListEntryVisualContext visual_context_;
};

/// Constructor-configured generic Material 3 list item descriptor.
class StandardListItem : public ListItem {
 public:
  /// Creates a standard item from the supplied construction descriptor.
  explicit StandardListItem(const StandardListItemInit& init = {});

  /// Returns non-owning overline text for this item.
  roo::string_view overlineText() const override;

  /// Returns non-owning headline text for this item.
  roo::string_view headlineText() const override;

  /// Returns non-owning supporting text for this item.
  roo::string_view supportingText() const override;

  /// Returns overline text wrapping and truncation policy.
  ListTextPolicy overlinePolicy() const override;

  /// Returns headline text wrapping and truncation policy.
  ListTextPolicy headlinePolicy() const override;

  /// Returns supporting text wrapping and truncation policy.
  ListTextPolicy supportingPolicy() const override;

  /// Returns the borrowed leading slot widget, or nullptr.
  Widget* leading() override;

  /// Returns the borrowed leading slot widget for read-only access.
  const Widget* leading() const override;

  /// Returns the borrowed trailing slot widget, or nullptr.
  Widget* trailing() override;

  /// Returns the borrowed trailing slot widget for read-only access.
  const Widget* trailing() const override;

  /// Returns the borrowed body slot widget, or nullptr.
  Widget* body() override;

  /// Returns the borrowed body slot widget for read-only access.
  const Widget* body() const override;

  /// Returns the leading visual alignment policy.
  VerticalVisualAlignment leadingAlignment() const override;

  /// Returns the trailing visual alignment policy.
  VerticalVisualAlignment trailingAlignment() const override;

  /// Returns item-provided divider inset hints.
  DividerInsetHint dividerInsetHint() const override;

  /// Returns whether the text block prefers top alignment.
  bool preferTopTextAlignment() const override;

  /// Returns the already configured borrowed leading widget.
  Widget* leadingWidget();

  /// Returns the already configured borrowed trailing widget.
  Widget* trailingWidget();

  /// Returns the already configured borrowed body widget.
  Widget* bodyWidget();

 private:
  roo::string_view overline_;
  roo::string_view headline_;
  roo::string_view supporting_;
  ListTextPolicy overline_policy_;
  ListTextPolicy headline_policy_;
  ListTextPolicy supporting_policy_;
  Widget* leading_;
  Widget* trailing_;
  Widget* body_;
  DividerInsetHint divider_inset_hint_;
  uint8_t leading_alignment_ : 1;
  uint8_t trailing_alignment_ : 1;
  uint8_t prefer_top_text_alignment_ : 1;
};

/// Lightweight headline-only convenience item for simple text rows.
class HeadlineListItem : public ListItem {
 public:
  explicit HeadlineListItem(roo::string_view headline = {},
                            ListTextPolicy headline_policy = {});

  roo::string_view headlineText() const override;
  ListTextPolicy headlinePolicy() const override;

  roo::string_view headline() const;
  void setHeadline(roo::string_view headline);
  void setHeadlinePolicy(ListTextPolicy policy);

 private:
  roo::string_view headline_;
  ListTextPolicy headline_policy_;
};

/// Shared headline/supporting text payload for convenience list item families.
class HeadlineSupportingListItemBase : public ListItem {
 public:
  HeadlineSupportingListItemBase(roo::string_view headline = {},
                                 roo::string_view supporting = {},
                                 ListTextPolicy headline_policy = {},
                                 ListTextPolicy supporting_policy = {});

  roo::string_view headlineText() const override;
  roo::string_view supportingText() const override;
  ListTextPolicy headlinePolicy() const override;
  ListTextPolicy supportingPolicy() const override;

  roo::string_view headline() const;
  roo::string_view supporting() const;
  void setHeadline(roo::string_view headline);
  void setSupportingText(roo::string_view supporting);
  void setHeadlinePolicy(ListTextPolicy policy);
  void setSupportingPolicy(ListTextPolicy policy);

 protected:
  roo::string_view headline_;
  roo::string_view supporting_;
  ListTextPolicy headline_policy_;
  ListTextPolicy supporting_policy_;
};

/// Lightweight headline-plus-supporting convenience item with no slot widgets.
class SupportingTextListItem : public HeadlineSupportingListItemBase {
 public:
  SupportingTextListItem(roo::string_view headline = {},
                         roo::string_view supporting = {},
                         ListTextPolicy headline_policy = {},
                         ListTextPolicy supporting_policy = {});
};

/// Convenience item that owns a leading pictogram widget plus standard text.
class PictogramSupportingTextItem : public HeadlineSupportingListItemBase {
 public:
  PictogramSupportingTextItem(ApplicationContext& context,
                              const roo_display::Pictogram& pictogram,
                              roo::string_view headline = {},
                              roo::string_view supporting = {},
                              ListTextPolicy headline_policy = {},
                              ListTextPolicy supporting_policy = {});

  Widget* leading() override;
  const Widget* leading() const override;

  Icon& leadingIcon();
  const Icon& leadingIcon() const;
  void setPictogram(const roo_display::Pictogram& pictogram);

 private:
  Icon leading_icon_;
};

/// Convenience item that owns a leading initials avatar plus standard text.
class AvatarSupportingTextItem : public HeadlineSupportingListItemBase {
 public:
  AvatarSupportingTextItem(ApplicationContext& context,
                           roo::string_view initials = {},
                           roo::string_view headline = {},
                           roo::string_view supporting = {},
                           ListTextPolicy headline_policy = {},
                           ListTextPolicy supporting_policy = {});
  ~AvatarSupportingTextItem() override;

  Widget* leading() override;
  const Widget* leading() const override;

  roo::string_view initials() const;
  void setInitials(roo::string_view initials);

 private:
  std::unique_ptr<AvatarVisual> leading_avatar_;
};

/// Shared invoke-capable convenience base that keeps callback storage in
/// opt-in item families.
class InvokableListItemBase : public HeadlineSupportingListItemBase {
 public:
  /// Creates an invoke-capable convenience item with optional always-clickable
  /// behavior.
  InvokableListItemBase(roo::string_view headline = {},
                        roo::string_view supporting = {},
                        ListTextPolicy headline_policy = {},
                        ListTextPolicy supporting_policy = {},
                        bool always_invokable = false);

  /// Returns whether this item currently allows row invocation.
  bool isInvokable() const override;

  /// Runs item-specific invoke behavior and the optional invoke callback.
  void invoke() override;

  /// Sets an optional invoke callback used by row or affordance presses.
  void setOnInvoked(std::function<void()> on_invoked);

 protected:
  /// Runs the invoke callback when present.
  void notifyInvoked();

  /// Performs item-specific invoke behavior before notifyInvoked().
  virtual void handleInvoke();

 private:
  bool always_invokable_;
  std::function<void()> on_invoked_;
};

/// Convenience item that owns a leading icon and trailing navigation
/// affordance.
class NavigationListItem : public InvokableListItemBase {
 public:
  NavigationListItem(ApplicationContext& context,
                     const roo_display::Pictogram& pictogram,
                     roo::string_view headline = {},
                     roo::string_view supporting = {},
                     ListTextPolicy headline_policy = {},
                     ListTextPolicy supporting_policy = {});

  Widget* leading() override;
  const Widget* leading() const override;
  Widget* trailing() override;
  const Widget* trailing() const override;

  Icon& leadingIcon();
  const Icon& leadingIcon() const;
  Icon& trailingAffordance();
  const Icon& trailingAffordance() const;
  void setPictogram(const roo_display::Pictogram& pictogram);

 private:
  Icon leading_icon_;
  Icon trailing_affordance_;
};

/// Convenience item with initials avatar and navigation affordance.
class AvatarNavigationListItem : public InvokableListItemBase {
 public:
  AvatarNavigationListItem(ApplicationContext& context,
                           roo::string_view initials = {},
                           roo::string_view headline = {},
                           roo::string_view supporting = {},
                           ListTextPolicy headline_policy = {},
                           ListTextPolicy supporting_policy = {});
  ~AvatarNavigationListItem() override;

  Widget* leading() override;
  const Widget* leading() const override;
  Widget* trailing() override;
  const Widget* trailing() const override;

  roo::string_view initials() const;
  Icon& trailingAffordance();
  const Icon& trailingAffordance() const;
  void setInitials(roo::string_view initials);

 private:
  std::unique_ptr<AvatarVisual> leading_avatar_;
  Icon trailing_affordance_;
};

/// Convenience item that owns a checkbox affordance plus standard text.
class CheckboxListItem : public InvokableListItemBase {
 public:
  /// Creates a checkbox-backed convenience item with configurable placement.
  CheckboxListItem(
      ApplicationContext& context, roo::string_view headline = {},
      roo::string_view supporting = {},
      Checkbox::OnOffState checked_state = Checkbox::OnOffState::kOff,
      AffordancePlacement placement = AffordancePlacement::kTrailing,
      ListTextPolicy headline_policy = {},
      ListTextPolicy supporting_policy = {});

  /// Creates a checkbox-backed convenience item from a boolean checked flag.
  CheckboxListItem(
      ApplicationContext& context, roo::string_view headline,
      roo::string_view supporting, bool checked,
      AffordancePlacement placement = AffordancePlacement::kTrailing,
      ListTextPolicy headline_policy = {},
      ListTextPolicy supporting_policy = {});

  /// Returns the leading slot widget when placement is leading.
  Widget* leading() override;

  /// Returns the leading slot widget when placement is leading.
  const Widget* leading() const override;

  /// Returns the trailing slot widget when placement is trailing.
  Widget* trailing() override;

  /// Returns the trailing slot widget when placement is trailing.
  const Widget* trailing() const override;

  /// Returns the current checkbox tri-state value.
  Checkbox::OnOffState checkedState() const;

  /// Returns true when the checkbox is in the on state.
  bool isChecked() const;

  /// Sets the checkbox to on or off.
  void setChecked(bool checked);

  /// Sets the checkbox tri-state value.
  void setCheckedState(Checkbox::OnOffState checked_state);

  /// Sets whether the checkbox appears in the leading or trailing slot.
  void setAffordancePlacement(AffordancePlacement placement);

  /// Returns the owned checkbox affordance.
  Checkbox& checkbox();

  /// Returns the owned checkbox affordance.
  const Checkbox& checkbox() const;

 protected:
  void handleInvoke() override;

 private:
  Checkbox checkbox_;
  uint8_t placement_ : 1;
};

/// Convenience item that owns a radio-button affordance plus standard text.
class RadioListItem : public InvokableListItemBase {
 public:
  /// Creates a radio-backed convenience item with configurable placement.
  RadioListItem(ApplicationContext& context, roo::string_view headline = {},
                roo::string_view supporting = {}, bool selected = false,
                AffordancePlacement placement = AffordancePlacement::kTrailing,
                ListTextPolicy headline_policy = {},
                ListTextPolicy supporting_policy = {});

  /// Returns the leading slot widget when placement is leading.
  Widget* leading() override;

  /// Returns the leading slot widget when placement is leading.
  const Widget* leading() const override;

  /// Returns the trailing slot widget when placement is trailing.
  Widget* trailing() override;

  /// Returns the trailing slot widget when placement is trailing.
  const Widget* trailing() const override;

  /// Returns whether the radio affordance is selected.
  bool isSelected() const;

  /// Sets whether the radio affordance is selected.
  void setSelected(bool selected);

  /// Sets whether the radio appears in the leading or trailing slot.
  void setAffordancePlacement(AffordancePlacement placement);

  /// Returns the owned radio-button affordance.
  RadioButton& radioButton();

  /// Returns the owned radio-button affordance.
  const RadioButton& radioButton() const;

 protected:
  void handleInvoke() override;

 private:
  RadioButton radio_button_;
  uint8_t placement_ : 1;
};

/// Convenience item that owns a switch affordance plus standard text.
class SwitchListItem : public InvokableListItemBase {
 public:
  /// Creates a switch-backed convenience item with configurable placement.
  SwitchListItem(ApplicationContext& context, roo::string_view headline = {},
                 roo::string_view supporting = {}, bool on = false,
                 AffordancePlacement placement = AffordancePlacement::kTrailing,
                 ListTextPolicy headline_policy = {},
                 ListTextPolicy supporting_policy = {});

  /// Returns the leading slot widget when placement is leading.
  Widget* leading() override;

  /// Returns the leading slot widget when placement is leading.
  const Widget* leading() const override;

  /// Returns the trailing slot widget when placement is trailing.
  Widget* trailing() override;

  /// Returns the trailing slot widget when placement is trailing.
  const Widget* trailing() const override;

  /// Returns whether the switch is currently on.
  bool isOn() const;

  /// Sets the switch on/off state.
  void setOn(bool on);

  /// Toggles the switch on/off state.
  void toggle();

  /// Sets whether the switch appears in the leading or trailing slot.
  void setAffordancePlacement(AffordancePlacement placement);

  /// Returns the owned switch affordance.
  Switch& switchControl();

  /// Returns the owned switch affordance.
  const Switch& switchControl() const;

 protected:
  void handleInvoke() override;

 private:
  Switch switch_;
  uint8_t placement_ : 1;
};

/// Thin ownership adapter that binds one inline-owned item to a `ListEntry`.
template <typename Item>
class ListRow : public ListEntry {
 public:
  template <typename... Args>
  explicit ListRow(ApplicationContext& context, Args&&... args)
      : ListEntry(context),
        item_(makeItem(context, std::forward<Args>(args)...)) {
    setItem(item_);
  }

  ~ListRow() override { clearItem(); }

  /// Returns the row-owned item.
  Item& item() { return item_; }

  /// Returns the row-owned item.
  const Item& item() const { return item_; }

 private:
  template <typename... Args>
  static Item makeItem(ApplicationContext& context, Args&&... args) {
    if constexpr (std::is_constructible<Item, ApplicationContext&,
                                        Args...>::value) {
      return Item(context, std::forward<Args>(args)...);
    } else {
      return Item(std::forward<Args>(args)...);
    }
  }

  Item item_;
};

/// Material 3 list container that owns row sequencing policy.
class List : public Container {
 public:
  /// Creates an empty column-oriented Material 3 list.
  explicit List(ApplicationContext& context);

  /// Releases any adopted rows still attached to this list.
  ~List() override;

  /// Sets the Material 3 list variant used for future visual propagation.
  void setVariant(ListVariant variant);

  /// Sets the Material 3 list style used for future visual propagation.
  void setStyle(ListStyle style);

  /// Sets the selection policy used for future visual propagation.
  void setSelectionPolicy(const ListSelectionPolicy& policy);

  /// Sets the divider policy used for future visual propagation.
  void setDividerPolicy(const ListDividerPolicy& policy);

  /// Adds a borrowed row entry to the list.
  void add(ListEntry& entry);

  /// Adds an adopted row entry to the list.
  void add(std::unique_ptr<ListEntry> entry);

  /// Clears all row entries from the list.
  void clear();

 protected:
  void paint(PaintContext& ctx) const override;
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  void onStructureOrPolicyChanged();
  void addEntryInternal(ListEntry* entry, WidgetRef ref);
  void markEntryContextsDirty();
  void refreshEntryVisualContexts();
  int16_t interRowGap(int previous_idx, int next_idx) const;

  std::vector<ListEntry*> entries_;
  std::vector<uint8_t> selected_entries_;
  ListVariant variant_;
  ListStyle style_;
  ListSelectionPolicy selection_policy_;
  ListDividerPolicy divider_policy_;
  bool contexts_dirty_;
};

}  // namespace material3
}  // namespace roo_windows