#pragma once

#include "roo_backport/string_view.h"
#include "roo_display/color/color.h"
#include "roo_display/font/font.h"
#include "roo_display/ui/tile.h"
#include "roo_windows/config.h"
#include "roo_windows/core/text_field_editor.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/internal/text_edit_target.h"
#include "roo_windows/material2/theme_mixin.h"

namespace roo_windows::material2 {

class TextField;

/// Eye / eye-slash style toggle used by `TextField` to reveal a masked value.
///
/// Acts as a compact on/off control: clicking toggles it, paint() chooses the
/// matching pictogram, and the consuming `TextField` reacts to the state
/// change.
class VisibilityToggle : public ThemeMixin<Widget> {
 public:
  /// Logical selection state exposed by the visibility toggle.
  enum class OnOffState : uint8_t { kOff, kOn };

  /// Creates the toggle in the default hidden-text state.
  VisibilityToggle(ApplicationContext& context)
      : ThemeMixin<Widget>(context), state_(OnOffState::kOff) {}

  /// Returns true when the text is shown.
  bool isOn() const { return state_ == OnOffState::kOn; }

  /// Returns true when the text is hidden.
  bool isOff() const { return !isOn(); }

  /// Returns the current logical state.
  OnOffState onOffState() const { return state_; }

  /// Sets the toggle to the shown-text state.
  void setOn() { setOnOffState(OnOffState::kOn); }

  /// Sets the toggle to the hidden-text state.
  void setOff() { setOnOffState(OnOffState::kOff); }

  /// Toggles between the shown-text and hidden-text states.
  void toggle() { isOn() ? setOff() : setOn(); }

  /// Updates the logical state and schedules a repaint when it changes.
  void setOnOffState(OnOffState state) {
    if (state_ == state) return;
    state_ = state;
    setDirty();
  }

  /// Reports the fixed icon-sized footprint used to draw the eye/eye-slash
  /// pictograms.
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(ROO_WINDOWS_ICON_SIZE, ROO_WINDOWS_ICON_SIZE);
  }

  bool isClickable() const override { return true; }

  ClickActivationPolicy getClickActivationPolicy() const override {
    return ClickActivationPolicy::kImmediateContinueAnimation;
  }

  /// Toggles the on/off state and then runs the base click behavior so the
  /// owning text field can react.
  void onClicked() override {
    toggle();
    Widget::onClicked();
  }

  /// Selects the eye or eye-slash pictogram based on the current state and
  /// paints it centered.
  void paint(PaintContext& ctx) const override;

 private:
  OnOffState state_;
};

/// Single-line editable text widget.
///
/// Holds the current value, hint, font, color hints, alignment, and an
/// optional decoration (e.g. underline). All editing state (cursor, selection,
/// scroll) is delegated to the shared `TextFieldEditor`, so individual fields
/// stay lightweight. Set `setStarred(true)` to render the value as bullet
/// characters for password-style fields.
class TextField : public ThemeMixin<Widget>, public internal::TextEditTarget {
 public:
  enum Decoration {
    NONE,
    UNDERLINE,
  };

  /// Creates a field using task-local editor resolution after attachment.
  TextField(ApplicationContext& context, const roo_display::Font& font,
            std::string hint, roo_display::Alignment alignment,
            Decoration decoration)
      : ThemeMixin<Widget>(context),
        decoration_(decoration),
        value_(""),
        hint_(std::move(hint)),
        starred_(false),
        font_(font),
        text_color_(roo_display::color::Transparent),
        highlight_color_(roo_display::color::Transparent),
        alignment_(alignment),
        editable_(true) {}

  ~TextField() override {
    if (isEdited()) editor().cancel();
  }

  bool isClickable() const override { return true; }

  Widget& editWidget() override { return *this; }
  std::string& textBuffer() override { return value_; }
  roo::string_view value() const override { return value_; }
  const roo_display::Font& textFont() const override { return font_; }
  bool obscureText() const override { return starred_; }
  void notifyEditVisualChange() override { invalidateInterior(); }

  // TODO: Replace this with an outline focus indication once text-field focus
  // styling is specified. A point overlay obscures the editing surface.
  OverlayType getOverlayType() const override { return OVERLAY_NONE; }

  ClickActivationPolicy getClickActivationPolicy() const override {
    return ClickActivationPolicy::kImmediateNoAnimation;
  }

  bool useOverlayOnPress() const override { return false; }

  /// Hook for subclasses; called when the editor finishes editing this field
  /// (`confirmed` is true on Enter, false on cancellation/blur).
  virtual void onEditFinished(bool confirmed) {}

  /// Routes a click into `edit()` so tapping the field starts editing.
  void onClicked() override {
    if (editable_) edit();
    Widget::onClicked();
  }

  void onFocusChanged(bool focused) override;
  bool onKeyEvent(const KeyEvent& event) override;

  /// Read-only fields remain focusable, but do not enter edit mode.
  bool isEditable() const { return editable_; }
  void setEditable(bool editable);

  /// Toggles password-style bullet rendering on the value.
  void setStarred(bool starred) {
    if (starred == starred_) return;
    starred_ = starred;
    if (isEdited()) editor().refreshMetrics();
    setDirty();
  }

  const std::string& hint() const { return hint_; }

  /// Replaces the placeholder text shown when the value is empty.
  void setHint(roo::string_view hint) {
    hint_ = std::string((const char*)hint.data(), hint.size());
  }

  /// Reports a match-parent width and a fixed height matching the font's max
  /// glyph height plus any decoration (e.g. underline).
  PreferredSize getPreferredSize() const override {
    Padding p = getPadding();
    int16_t preferred_height =
        font_.metrics().maxHeight() + p.top() + p.bottom();
    switch (decoration_) {
      case UNDERLINE: {
        preferred_height += 6;
        break;
      }
      default: {
      }
    }
    return PreferredSize(PreferredSize::MatchParentWidth(),
                         PreferredSize::ExactHeight(preferred_height));
  }

  /// Paints the value (or hint), selection highlight, and blinking caret
  /// when this field is the active editor target.
  void paint(PaintContext& ctx) const override;

  /// Reports the font-measured width and the same fixed height used by
  /// `getPreferredSize()`.
  Dimensions getSuggestedMinimumDimensions() const override {
    auto metrics = font_.getHorizontalStringMetrics(value_);
    int16_t preferred_height = metrics.height();
    switch (decoration_) {
      case UNDERLINE: {
        preferred_height += 6;
        break;
      }
      default: {
      }
    }
    return Dimensions(metrics.width(), preferred_height);
  }

  const std::string& content() const { return value_; }

  /// Replaces the value. Triggers an editor re-measure when this field is
  /// the active editor target.
  void setContent(std::string value) {
    if (value_ == value) return;
    value_ = std::move(value);
    if (isEdited()) editor().resetMetrics();
    setDirty();
  }

  const roo_display::Font& font() const { return font_; }

  /// Returns this field's task-local editor.
  TextFieldEditor& editor();

  /// Returns this field's task-local editor.
  const TextFieldEditor& editor() const;

 private:
  bool isEdited() const;
  bool isStarred() const { return starred_; }

 protected:
  /// Completes a focus request deferred until this newly attached field has
  /// non-empty layout bounds.
  void onLayout(bool changed, const Rect& rect) override;
  void onAnimationFrame(AnimationTag tag,
                        const AnimationSample& sample) override;

  static constexpr AnimationTag kCaret = 0;

 public:
  /// Binds this field to the shared editor and starts editing.
  void edit();

 private:
  Decoration decoration_;

  std::string value_;
  std::string hint_;
  bool starred_;
  const roo_display::Font& font_;
  roo_display::Color text_color_;
  roo_display::Color highlight_color_;
  roo_display::Alignment alignment_;
  bool editable_;
};

}  // namespace roo_windows::material2
