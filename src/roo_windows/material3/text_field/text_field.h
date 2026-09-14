#pragma once

#include "roo_windows/core/basic_surface_widget.h"
#include "roo_windows/core/layout_direction.h"
#include "roo_windows/internal/text_edit_target.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows::material3 {
/// Material 3 single-line container treatment.
enum class TextFieldVariant : uint8_t { kFilled, kOutlined };

/// Single-line owner-painted field. Only text() is owned. All slot strings and
/// icon pointers must outlive their assignment. Input must be valid single-line
/// UTF-8. Editing uses the owning Task, never a per-field editor or child
/// widget.
class TextField : public BasicSurfaceWidget, private internal::TextEditTarget {
 public:
  /// Creates a field with a borrowed label and an empty owned value.
  explicit TextField(ApplicationContext& context, roo::string_view label,
                     TextFieldVariant variant = TextFieldVariant::kFilled);

  /// Ends an active edit session before destroying the target.
  ~TextField() override;

  /// Returns the container treatment.
  TextFieldVariant variant() const;

  /// Changes the container treatment and remeasures its label clearance.
  void setVariant(TextFieldVariant variant);

  /// Returns the live value; cancel does not revert edits.
  const std::string& text() const { return value_; }

  /// Replaces the valid single-line UTF-8 value; notifies once on change.
  void setText(std::string value);

  /// Returns the borrowed resting/floating label.
  roo::string_view label() const { return label_; }

  /// Assigns a borrowed resting/floating label.
  void setLabel(roo::string_view value);

  /// Returns the borrowed supporting message.
  roo::string_view supportingText() const { return supporting_; }

  /// Assigns supporting text, shown when error state is inactive.
  void setSupportingText(roo::string_view value);

  /// Returns the borrowed error message.
  roo::string_view errorText() const { return error_; }

  /// Assigns an error message and activates error state, even when empty.
  void setErrorText(roo::string_view value);

  /// Clears error state and restores supporting text; retains the error view.
  void clearError();

  /// Returns whether error state is active.
  bool hasError() const { return flags_ & kError; }

  /// Returns the borrowed input prefix.
  roo::string_view prefixText() const { return prefix_; }

  /// Assigns a borrowed prefix shown with floated-label content.
  void setPrefixText(roo::string_view value);

  /// Returns the borrowed input suffix.
  roo::string_view suffixText() const { return suffix_; }

  /// Assigns a borrowed suffix shown with floated-label content.
  void setSuffixText(roo::string_view value);

  /// Returns the borrowed leading icon.
  const MonoIcon* leadingIcon() const { return leading_; }

  /// Assigns the leading affordance, or nullptr to remove it.
  void setLeadingIcon(const MonoIcon* icon);

  /// Returns the assigned base icon; secure fields override the effective slot.
  const MonoIcon* trailingIcon() const { return trailing_; }

  /// Assigns the base trailing affordance, or nullptr for error fallback.
  void setTrailingIcon(const MonoIcon* icon);

  /// Returns whether activation avoids editing the value.
  bool readOnly() const { return flags_ & kReadOnly; }

  /// Changes editability; making an active field read-only ends its session.
  void setReadOnly(bool value);

  /// Returns the explicit direction used to place leading/trailing slots.
  LayoutDirection layoutDirection() const;

  /// Mirrors slot layout without reordering the logical UTF-8 value.
  void setLayoutDirection(LayoutDirection direction);

  /// Returns whether the owning task editor is bound to this field.
  bool isEdited() const;

  /// Requests focus and editing with the software keyboard when eligible.
  void edit();

  /// Uses the field palette to resolve disabled colors once.
  bool useAutomaticDisabledStyle() const override { return false; }

  /// Participates in normal widget activation.
  bool isClickable() const override { return true; }

  /// Paints state layers as part of the final field surface.
  OverlayType getOverlayType() const override { return OVERLAY_NONE; }

  /// Keeps press coloring in the owner-painted palette.
  bool useOverlayOnPress() const override { return false; }

  /// Settles affordance activation without a separate click animation.
  ClickActivationPolicy getClickActivationPolicy() const override {
    return ClickActivationPolicy::kImmediateNoAnimation;
  }

  /// Leaves corners and outlined label clearance to the ancestor surface.
  bool fullyCoversBoundsWithOpaqueColors() const override { return false; }

  /// Returns the fixed container and optional assistive-row minimum.
  Dimensions getSuggestedMinimumDimensions() const override;

  /// Expands horizontally and wraps the required vertical space.
  PreferredSize getPreferredSize() const override;

  /// Paints slots, editing state, and container with foreground exclusions.
  void paint(PaintContext& ctx) const override;

  /// Dispatches a pending affordance or activates the field.
  void onClicked() override;

  /// Selects an occupied owner-local affordance before click settlement.
  void onSingleTapUp(XDim x, YDim y) override;

  /// Clears pending affordance and activation-key state.
  void onCancel() override;

  /// Handles explicit edit activation and active single-line editing keys.
  bool onKeyEvent(const KeyEvent& event) override;

  /// Updates the floating label and ends editing on blur.
  void onFocusChanged(bool focused) override;

 protected:
  /// Notifies subclasses after the owned value changes.
  virtual void onTextChanged() {}

  /// Notifies subclasses when the active edit session finishes.
  void onEditFinished(bool confirmed) override {}

  /// Handles a leading-affordance activation and reports whether it consumed
  /// it.
  virtual bool onLeadingAffordanceClicked() { return false; }

  /// Handles a trailing-affordance activation and reports whether it consumed
  /// it.
  virtual bool onTrailingAffordanceClicked() { return false; }

  /// Returns the trailing icon after applying subclass-specific fallbacks.
  virtual const MonoIcon* effectiveTrailingIcon() const;

  /// Reports whether editor rendering should mask the owned value.
  bool obscureText() const override { return false; }

  /// Refreshes active editor metrics after the masking policy changes.
  void maskingChanged();

  /// Retains focus while editing and updates the horizontal viewport scroll.
  void onLayout(bool changed, const Rect& rect) override;

  /// Accepts the active editor's sparse cursor-animation samples.
  void onAnimationFrame(AnimationTag tag,
                        const AnimationSample& sample) override;

  /// Cancels editing when a state transition disables the field.
  void notifyStateChanged(uint16_t diff) override;

 private:
  /// Owner-local rectangles for the field's painted regions.
  struct Slots;

  /// Computes the clipped rectangles for the current content and direction.
  Slots slots() const;

  /// Reports whether the label is displayed above the input content.
  bool floated() const;

  /// Selects the error or supporting message for the assistive row.
  roo::string_view assistiveText() const;

  /// Invalidates, relayouts, and preserves the active caret viewport.
  void geometryChanged();

  /// Focuses and binds the task editor when this field is eligible.
  void startEditing(bool show_keyboard);

  /// Keeps the active caret within the computed input viewport.
  void updateScroll();
  Widget& editWidget() override { return *this; }
  std::string& textBuffer() override { return value_; }
  roo::string_view value() const override { return value_; }
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
  roo::string_view label_;
  roo::string_view supporting_;
  roo::string_view error_;
  roo::string_view prefix_;
  roo::string_view suffix_;
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
