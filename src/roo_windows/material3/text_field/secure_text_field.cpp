#include "roo_windows/material3/text_field/secure_text_field.h"

#include "roo_icons/filled/action.h"

namespace roo_windows::material3 {
namespace {
const MonoIcon kVisibility = SCALED_ROO_ICON(filled, action_visibility);
const MonoIcon kVisibilityOff = SCALED_ROO_ICON(filled, action_visibility_off);
}  // namespace
void SecureTextField::setRevealed(bool value) {
  if (revealed_ == value) return;
  revealed_ = value;
  maskingChanged();
}
const MonoIcon* SecureTextField::effectiveTrailingIcon() const {
  return revealed_ ? &kVisibilityOff : &kVisibility;
}
bool SecureTextField::onTrailingAffordanceClicked() {
  setRevealed(!revealed_);
  return true;
}
}  // namespace roo_windows::material3
