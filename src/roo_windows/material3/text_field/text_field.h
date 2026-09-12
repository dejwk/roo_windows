#pragma once

#include "roo_windows/core/basic_surface_widget.h"
#include "roo_windows/core/layout_direction.h"
#include "roo_windows/internal/text_edit_target.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows::material3 {
enum class TextFieldVariant : uint8_t { kFilled, kOutlined };

// Single-line owner-painted field. Only text() is owned. All slot strings and
// icon pointers must outlive their assignment. Input must be valid single-line
// UTF-8. Editing uses the owning Task, never a per-field editor or child
// widget.
class TextField : public BasicSurfaceWidget, private internal::TextEditTarget {
 public:
  explicit TextField(ApplicationContext& context, roo::string_view label,
                     TextFieldVariant variant = TextFieldVariant::kFilled);
  ~TextField() override;
  TextFieldVariant variant() const;
  void setVariant(TextFieldVariant variant);
  const std::string& text() const { return value_; }
  void setText(std::string value);
  roo::string_view label() const { return label_; }
  void setLabel(roo::string_view value);
  roo::string_view supportingText() const { return supporting_; }
  void setSupportingText(roo::string_view value);
  roo::string_view errorText() const { return error_; }
  // Activates error state even if value is empty. clearError restores support.
  void setErrorText(roo::string_view value);
  void clearError();
  bool hasError() const { return flags_ & kError; }
  roo::string_view prefixText() const { return prefix_; }
  void setPrefixText(roo::string_view value);
  roo::string_view suffixText() const { return suffix_; }
  void setSuffixText(roo::string_view value);
  const MonoIcon* leadingIcon() const { return leading_; }
  void setLeadingIcon(const MonoIcon* icon);
  const MonoIcon* trailingIcon() const { return trailing_; }
  void setTrailingIcon(const MonoIcon* icon);
  bool readOnly() const { return flags_ & kReadOnly; }
  void setReadOnly(bool value);
  // Like other childless M3 components, direction is explicit and packed.
  LayoutDirection layoutDirection() const;
  void setLayoutDirection(LayoutDirection direction);
  bool isEdited() const;
  void edit();
  bool useAutomaticDisabledStyle() const override { return false; }
  bool isClickable() const override { return true; }
  OverlayType getOverlayType() const override { return OVERLAY_NONE; }
  bool useOverlayOnPress() const override { return false; }
  ClickActivationPolicy getClickActivationPolicy() const override {
    return ClickActivationPolicy::kImmediateNoAnimation;
  }
  bool fullyCoversBoundsWithOpaqueColors() const override { return false; }
  Dimensions getSuggestedMinimumDimensions() const override;
  PreferredSize getPreferredSize() const override;
  void paint(PaintContext& ctx) const override;
  void onClicked() override;
  void onSingleTapUp(XDim x, YDim y) override;
  void onCancel() override;
  bool onKeyEvent(const KeyEvent& event) override;
  void onFocusChanged(bool focused) override;

 protected:
  virtual void onTextChanged() {}
  void onEditFinished(bool confirmed) override {}
  virtual bool onLeadingAffordanceClicked() { return false; }
  virtual bool onTrailingAffordanceClicked() { return false; }
  virtual const MonoIcon* effectiveTrailingIcon() const;
  bool obscureText() const override { return false; }
  void maskingChanged();
  void onLayout(bool changed, const Rect& rect) override;
  void onAnimationFrame(AnimationTag tag,
                        const AnimationSample& sample) override;
  void notifyStateChanged(uint16_t diff) override;

 private:
  struct Slots;
  Slots slots() const;
  bool floated() const;
  roo::string_view assistiveText() const;
  void geometryChanged();
  void startEditing(bool show_keyboard);
  void updateScroll();
  Widget& editWidget() override { return *this; }
  const Widget& editWidget() const override { return *this; }
  std::string& textBuffer() override { return value_; }
  const std::string& textBuffer() const override { return value_; }
  const roo_display::Font& textFont() const override {
    return text_style_body_large().font();
  }
  roo_display::Font::Options textFontOptions() const override {
    return text_style_body_large().fontOptions();
  }
  void notifyEditVisualChange() override;
  void notifyTextChanged() override;
  enum : uint8_t {
    kOutlined = 1,
    kReadOnly = 2,
    kError = 4,
    kRtl = 8,
    kLastEdited = 16,
    kActivationKey = 32,
    kLeadingTap = 64,
    kTrailingTap = 128
  };
  std::string value_;
  roo::string_view label_, supporting_, error_, prefix_, suffix_;
  const MonoIcon* leading_ = nullptr;
  const MonoIcon* trailing_ = nullptr;
  uint8_t flags_;
};

// Includes the edit-target vptr, string ABI, slot views and packed state.
static_assert(sizeof(TextField) <=
                  sizeof(BasicSurfaceWidget) + sizeof(std::string) +
                      5 * sizeof(roo::string_view) + 4 * sizeof(void*),
              "TextField must not acquire per-instance editor, child or "
              "callback storage");
}  // namespace roo_windows::material3
