#include "roo_windows/material3/button/button_group.h"

namespace roo_windows::material3 {

ButtonGroupButton::ButtonGroupButton(ApplicationContext& context,
                                     roo::string_view label,
                                     ButtonGroupButtonStyle style)
    : ButtonGroupLabelEntry(context, label,
                            static_cast<internal::GroupStyle>(style)) {}

void ButtonGroupButton::setStyle(ButtonGroupButtonStyle style) {
  changeStyle(static_cast<internal::GroupStyle>(style));
}

ButtonGroupIconButton::ButtonGroupIconButton(ApplicationContext& context,
                                             const MonoIcon& icon,
                                             ButtonGroupIconStyle style)
    : ButtonGroupIconEntry(context, icon,
                           static_cast<internal::GroupStyle>(style)) {}

void ButtonGroupIconButton::setStyle(ButtonGroupIconStyle style) {
  changeStyle(static_cast<internal::GroupStyle>(style));
}

ConnectedButtonGroup::ConnectedButtonGroup(ApplicationContext& context)
    : ButtonGroupBase(context),
      mode_(static_cast<uint8_t>(ConnectedButtonGroupSelectionMode::kSingle)),
      selection_required_(1) {}

ConnectedButtonGroup::~ConnectedButtonGroup() { clear(); }

uint32_t ConnectedButtonGroup::normalize(uint32_t mask) const {
  uint32_t valid =
      entryCount() == 32 ? UINT32_MAX : (uint32_t{1} << entryCount()) - 1;
  mask &= valid;
  if (selectionMode() == ConnectedButtonGroupSelectionMode::kSingle)
    mask &= (~mask + 1);
  if (mask == 0 && selectionRequired() && entryCount() != 0) mask = 1;
  return mask;
}

void ConnectedButtonGroup::setSelectionMask(uint32_t mask) {
  mask = normalize(mask);
  if (mask == selection_mask_) return;
  uint32_t previous = selection_mask_;
  selection_mask_ = mask;
  projectSelection(previous, mask);
}

void ConnectedButtonGroup::setSelectionMode(
    ConnectedButtonGroupSelectionMode mode) {
  if (selectionMode() == mode) return;
  mode_ = static_cast<uint8_t>(mode);
  setSelectionMask(selection_mask_);
}

void ConnectedButtonGroup::setSelectionRequired(bool required) {
  if (selectionRequired() == required) return;
  selection_required_ = required ? 1 : 0;
  setSelectionMask(selection_mask_);
}

int ConnectedButtonGroup::selectedIndex() const {
  if (selection_mask_ == 0 || (selection_mask_ & (selection_mask_ - 1)) != 0)
    return -1;
  return __builtin_ctz(selection_mask_);
}

bool ConnectedButtonGroup::isSelected(int index) const {
  return index >= 0 && index < entryCount() &&
         (selection_mask_ & (uint32_t{1} << index)) != 0;
}

void ConnectedButtonGroup::entryAdded() {
  uint32_t previous = selection_mask_;
  selection_mask_ = normalize(selection_mask_);
  projectSelection(previous, selection_mask_);
}

void ConnectedButtonGroup::activateEntry(internal::ButtonGroupEntry& entry) {
  int index = indexOf(entry);
  if (index < 0 || !entry.eligible()) return;
  uint32_t bit = uint32_t{1} << index;
  uint32_t next = selection_mask_;
  if (selectionMode() == ConnectedButtonGroupSelectionMode::kMultiple) {
    next ^= bit;
  } else {
    next = (next & bit) != 0 ? 0 : bit;
  }
  if (next == 0 && selectionRequired()) return;
  uint32_t previous = selection_mask_;
  setSelectionMask(next);
  if (previous == selection_mask_) return;
  triggerInteractiveChange();
}

bool ConnectedButtonGroup::add(ButtonGroupButton& entry) {
  return addEntry(&entry, WidgetRef(entry));
}

bool ConnectedButtonGroup::add(ButtonGroupIconButton& entry) {
  return addEntry(&entry, WidgetRef(entry));
}

bool ConnectedButtonGroup::add(std::unique_ptr<ButtonGroupButton> entry) {
  internal::ButtonGroupEntry* raw = entry.get();
  return addEntry(raw, WidgetRef(std::move(entry)));
}

bool ConnectedButtonGroup::add(std::unique_ptr<ButtonGroupIconButton> entry) {
  internal::ButtonGroupEntry* raw = entry.get();
  return addEntry(raw, WidgetRef(std::move(entry)));
}

void ConnectedButtonGroup::clear() {
  selection_mask_ = 0;
  clearEntries();
}

ButtonGroup::ButtonGroup(ApplicationContext& context)
    : ButtonGroupBase(context) {}

ButtonGroup::~ButtonGroup() { clear(); }

bool ButtonGroup::add(ButtonGroupButton& entry) {
  return addEntry(&entry, WidgetRef(entry));
}

bool ButtonGroup::add(ButtonGroupIconButton& entry) {
  return addEntry(&entry, WidgetRef(entry));
}

bool ButtonGroup::add(std::unique_ptr<ButtonGroupButton> entry) {
  internal::ButtonGroupEntry* raw = entry.get();
  return addEntry(raw, WidgetRef(std::move(entry)));
}

bool ButtonGroup::add(std::unique_ptr<ButtonGroupIconButton> entry) {
  internal::ButtonGroupEntry* raw = entry.get();
  return addEntry(raw, WidgetRef(std::move(entry)));
}

void ButtonGroup::clear() { clearEntries(); }

void ButtonGroup::activateEntry(internal::ButtonGroupEntry& entry) {
  if (indexOf(entry) < 0 || !entry.eligible()) return;
  entry.Widget::onClicked();
}
}  // namespace roo_windows::material3
