#pragma once
#include "roo_windows/material3/text_field/text_field.h"

namespace roo_windows::material3 {
/// A masked single-line field with a visibility affordance.
///
/// The trailing slot is reserved for visibility, including in error state.
/// setTrailingIcon() affects the stored base slot, not this effective
/// affordance.
class SecureTextField : public TextField {
 public:
  /// Creates a masked field; the trailing slot is reserved for reveal.
  explicit SecureTextField(ApplicationContext& context, roo::string_view label,
                           TextFieldVariant variant = TextFieldVariant::kFilled)
      : TextField(context, label, variant), revealed_(false) {}

  /// Returns whether the full value is visible.
  bool revealed() const { return revealed_; }

  /// Toggles visibility while preserving selection and the edit session.
  void setRevealed(bool value);

 protected:
  bool obscureText() const override { return !revealed_; }
  const MonoIcon* effectiveTrailingIcon() const override;
  bool onTrailingAffordanceClicked() override;

 private:
  bool revealed_ : 1;
};
static_assert(sizeof(SecureTextField) <= sizeof(TextField) + sizeof(void*),
              "SecureTextField adds only one reveal bit and ABI alignment");
}  // namespace roo_windows::material3
