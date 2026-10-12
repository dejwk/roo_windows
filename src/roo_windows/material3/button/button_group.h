#pragma once
#include "roo_windows/material3/button/internal/button_group_base.h"
namespace roo_windows::material3 {
/// Contained emphasis for labelled action and connected selection entries.
enum class ButtonGroupButtonStyle : uint8_t {
  kFilled,
  kFilledTonal,
  kOutlined,
  kElevated
};
/// Contained emphasis for icon-only group entries.
enum class ButtonGroupIconStyle : uint8_t { kFilled, kFilledTonal, kOutlined };
/// Selection policy independent of keyboard focus.
enum class ConnectedButtonGroupSelectionMode : uint8_t { kSingle, kMultiple };
/// Single-line group entry borrowing its label and optional leading icon.
/// Detached entries are configurable but inert, including in generic layouts.
class ButtonGroupButton : public internal::ButtonGroupLabelEntry {
 public:
  /// Creates an entry borrowing @p label in @p context with @p style.
  explicit ButtonGroupButton(
      ApplicationContext& context, roo::string_view label = {},
      ButtonGroupButtonStyle style = ButtonGroupButtonStyle::kFilled);
  /// Returns the entry's contained emphasis.
  ButtonGroupButtonStyle style() const {
    return static_cast<ButtonGroupButtonStyle>(style_);
  }
  /// Changes emphasis and reserved outline space without changing selection.
  void setStyle(ButtonGroupButtonStyle style);
};
/// Uniform-width group entry borrowing non-null icon artwork.
class ButtonGroupIconButton : public internal::ButtonGroupIconEntry {
 public:
  /// Creates an icon entry borrowing @p icon in @p context with @p style.
  explicit ButtonGroupIconButton(
      ApplicationContext& context, const MonoIcon& icon,
      ButtonGroupIconStyle style = ButtonGroupIconStyle::kFilled);
  /// Returns the entry's contained emphasis.
  ButtonGroupIconStyle style() const {
    return static_cast<ButtonGroupIconStyle>(style_);
  }
  /// Changes emphasis and reserved outline space without changing selection.
  void setStyle(ButtonGroupIconStyle style);
};
/// Stable connected selection row with at most 32 typed entries.
/// Uses small/round/LTR, single/required defaults. Labels, icons and borrowed
/// entries must outlive attachment. Configure on the UI thread between frames.
/// Interactive changes emit this group's callback once; programmatic setters,
/// add and clear are silent. A callback may clear or destroy the containing UI.
class ConnectedButtonGroup : public internal::ButtonGroupBase {
 public:
  /// Creates an empty single/required selector in @p context.
  explicit ConnectedButtonGroup(ApplicationContext& context);
  /// Detaches borrowed entries and destroys adopted entries without callbacks.
  ~ConnectedButtonGroup() override;
  /// Returns the single or multiple selection policy.
  ConnectedButtonGroupSelectionMode selectionMode() const {
    return static_cast<ConnectedButtonGroupSelectionMode>(mode_);
  }
  /// Changes policy; single mode retains the lowest selected attachment bit.
  void setSelectionMode(ConnectedButtonGroupSelectionMode mode);
  /// Reports whether a nonempty group requires at least one selected entry.
  bool selectionRequired() const { return selection_required_ != 0; }
  /// Changes the empty-selection policy; required mode repairs to index zero.
  void setSelectionRequired(bool required);
  /// Returns canonical bits indexed by attachment order, including gone
  /// entries.
  uint32_t selectionMask() const { return selection_mask_; }
  /// Silently normalizes and projects @p mask, ignoring out-of-range bits.
  void setSelectionMask(uint32_t mask);
  /// Returns the sole selected attachment index, or -1 for zero/multiple bits.
  int selectedIndex() const;
  /// Reports canonical selection; invalid attachment indices return false.
  bool isSelected(int index) const;
  /// Borrows @p entry until clear/destruction; false only at capacity.
  /// An already parented or cross-context entry is a checked programmer error.
  bool add(ButtonGroupButton& entry);
  /// Borrows @p entry with the same bounded admission contract.
  bool add(ButtonGroupIconButton& entry);
  /// Consumes @p entry, adopting it on success; null/capacity returns false.
  /// Rejected owned input is destroyed. Invalid membership is checked first.
  bool add(std::unique_ptr<ButtonGroupButton> entry);
  /// Consumes icon ownership with the same admission contract.
  bool add(std::unique_ptr<ButtonGroupIconButton> entry);
  /// Silently clears selection and detaches all entries, retaining capacity.
  void clear();

 protected:
  bool connected() const override { return true; }
  bool selected(int index) const override { return isSelected(index); }
  void entryAdded() override;
  void activateEntry(internal::ButtonGroupEntry& entry) override;

 private:
  /// Produces the canonical valid mask for the current policy and membership.
  uint32_t normalize(uint32_t mask) const;
  uint32_t selection_mask_ = 0;
  uint8_t mode_ : 1;
  uint8_t selection_required_ : 1;
};
}  // namespace roo_windows::material3
