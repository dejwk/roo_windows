#pragma once

#include <string>

#include "roo_display/ui/string_printer.h"
#include "roo_windows/containers/aligned_layout.h"
#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/widgets/slider.h"
#include "roo_windows/widgets/text_label.h"

namespace roo_windows {
namespace menu {

/// Vertical menu row containing a caption, a value readout, and a slider.
///
/// Subclasses provide `formatValue()` to render the right-aligned readout
/// string from the slider's raw 16-bit position. Used to build settings menus
/// where each row exposes a single numeric/enumerated knob.
class BaseSliderWithCaption : public roo_windows::VerticalLayout {
 public:
  BaseSliderWithCaption(const roo_windows::Environment& env,
                        std::string caption);

  virtual ~BaseSliderWithCaption() = default;

  roo_windows::PreferredSize getPreferredSize() const override {
    return roo_windows::PreferredSize(
        roo_windows::PreferredSize::MatchParentWidth(),
        roo_windows::PreferredSize::WrapContentHeight());
  }

  /// Replaces the displayed caption text.
  void setCaption(std::string caption) { caption_.setText(std::move(caption)); }

  /// Returns the row's outer margins; subclasses may override to tighten or
  /// expand the vertical rhythm.
  virtual Margins getMargins() const { return Margins(0, Scaled(8)); }

 protected:
  /// Renders the value readout for the supplied raw 16-bit slider position.
  virtual std::string formatValue(uint16_t value) const = 0;

  roo_windows::AlignedLayout text_section_;
  roo_windows::TextLabel caption_;
  roo_windows::TextLabel value_;
  roo_windows::Slider slider_;
};

/// Concrete `BaseSliderWithCaption` that maps the slider position to a float
/// value in `[min, max]` (with optional step) and renders it using `printf`
/// formatting.
class NumericSliderWithCaption : public BaseSliderWithCaption {
 public:
  NumericSliderWithCaption(const roo_windows::Environment& env,
                           std::string caption);

  /// Sets the printf-style format used to render the readout (default
  /// `"%0.1f"`).
  void setFormat(std::string format) { format_ = std::move(format); }

  /// Configures the numeric range, initial value, and optional step size.
  void init(float min_val, float max_val, float val, float step = 0.0f);
  /// Updates the displayed value (clamped and snapped to the configured
  /// range/step).
  void setValue(float val);

  /// Returns the current value mapped from the slider's raw position.
  float getValue() const;

 protected:
  /// Formats the current numeric value using the configured printf format.
  std::string formatValue(uint16_t value) const;

 private:
  float trimValue(float val) const;

  float min_;
  float max_;
  float step_;
  std::string format_ = "%0.1f";
};

}  // namespace menu
}  // namespace roo_windows
