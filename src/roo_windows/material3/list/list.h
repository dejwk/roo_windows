#pragma once

#include <stdint.h>

#include <memory>
#include <vector>

#include "roo_display/core/utf8.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/widget_ref.h"

namespace roo_windows {

class StringViewLabel;

namespace material3 {

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
};

/// Construction-time descriptor for `StandardListItem`.
struct StandardListItemInit {
  roo_display::StringView overline = {};
  roo_display::StringView headline = {};
  roo_display::StringView supporting = {};
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
  static StandardListItemInit OneLine(roo_display::StringView headline,
                                      Widget* leading = nullptr,
                                      Widget* trailing = nullptr);

  /// Builds a two-line standard list item descriptor.
  static StandardListItemInit TwoLine(roo_display::StringView headline,
                                      roo_display::StringView supporting,
                                      Widget* leading = nullptr,
                                      Widget* trailing = nullptr);

  /// Builds a three-line standard list item descriptor.
  static StandardListItemInit ThreeLine(roo_display::StringView headline,
                                        roo_display::StringView supporting,
                                        roo_display::StringView overline = {},
                                        Widget* leading = nullptr,
                                        Widget* trailing = nullptr,
                                        Widget* body = nullptr);
};

/// Pure-virtual content contract consumed by `ListEntry`.
class ListItem {
 public:
  virtual ~ListItem() = default;

  /// Returns non-owning overline text for this item.
  virtual roo_display::StringView overlineText() const { return {}; }

  /// Returns non-owning headline text for this item.
  virtual roo_display::StringView headlineText() const { return {}; }

  /// Returns non-owning supporting text for this item.
  virtual roo_display::StringView supportingText() const { return {}; }

  /// Returns overline text wrapping and truncation policy.
  virtual ListTextPolicy overlinePolicy() const { return ListTextPolicy{}; }

  /// Returns headline text wrapping and truncation policy.
  virtual ListTextPolicy headlinePolicy() const { return ListTextPolicy{}; }

  /// Returns supporting text wrapping and truncation policy.
  virtual ListTextPolicy supportingPolicy() const { return ListTextPolicy{}; }

  /// Returns the borrowed leading slot widget, or nullptr.
  virtual Widget* leading() const { return nullptr; }

  /// Returns the borrowed trailing slot widget, or nullptr.
  virtual Widget* trailing() const { return nullptr; }

  /// Returns the borrowed body slot widget, or nullptr.
  virtual Widget* body() const { return nullptr; }

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

  /// Returns the minimum dimensions for the bound item and slot widgets.
  Dimensions getSuggestedMinimumDimensions() const override;

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  void detachBoundChildren();
  void clearTextLabel(StringViewLabel*& label);
  void clearTextSlots();
  void syncTextSlotsFromItem();

  ListItem* item_;
  Widget* leading_child_;
  StringViewLabel* overline_label_;
  StringViewLabel* headline_label_;
  StringViewLabel* supporting_label_;
  Widget* trailing_child_;
  Widget* body_child_;
  ListEntryVisualContext visual_context_;
};

/// Constructor-configured generic Material 3 list item descriptor.
class StandardListItem : public ListItem {
 public:
  /// Creates a standard item from the supplied construction descriptor.
  explicit StandardListItem(const StandardListItemInit& init = {});

  /// Returns non-owning overline text for this item.
  roo_display::StringView overlineText() const override;

  /// Returns non-owning headline text for this item.
  roo_display::StringView headlineText() const override;

  /// Returns non-owning supporting text for this item.
  roo_display::StringView supportingText() const override;

  /// Returns overline text wrapping and truncation policy.
  ListTextPolicy overlinePolicy() const override;

  /// Returns headline text wrapping and truncation policy.
  ListTextPolicy headlinePolicy() const override;

  /// Returns supporting text wrapping and truncation policy.
  ListTextPolicy supportingPolicy() const override;

  /// Returns the borrowed leading slot widget, or nullptr.
  Widget* leading() const override;

  /// Returns the borrowed trailing slot widget, or nullptr.
  Widget* trailing() const override;

  /// Returns the borrowed body slot widget, or nullptr.
  Widget* body() const override;

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
  roo_display::StringView overline_;
  roo_display::StringView headline_;
  roo_display::StringView supporting_;
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
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  void markEntryContextsDirty();
  void refreshEntryVisualContexts();
  int16_t interRowGap() const;

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