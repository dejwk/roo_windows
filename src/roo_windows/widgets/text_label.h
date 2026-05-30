#pragma once

#include "roo_backport/string_view.h"
#include "roo_display/color/color.h"
#include "roo_display/font/font.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/gravity.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

/// Static text widget that owns its string content.
///
/// Use this when the label may change over time (the contained `std::string`
/// supports `setText()` and `setTextf()`). For labels backed by a stable
/// string buffer that the caller already keeps alive, prefer the cheaper
/// `StringViewLabel`.
class TextLabel : public BasicWidget {
 public:
  TextLabel(const Environment& env, std::string value,
            const roo_display::Font& font);

  TextLabel(const Environment& env, std::string value,
            const roo_display::Font& font, Gravity gravity);

  TextLabel(const Environment& env, std::string value,
            const roo_display::Font& font, roo_display::Color color,
            Gravity gravity);

  /// Paints the owned string in a single line, with the configured gravity
  /// applied within the widget bounds.
  void paint(PaintContext& ctx) const override;

  /// Reports ink insets matching the rendered text rectangle within the
  /// widget bounds (so partial redraws only touch the glyphs).
  Insets getInkInsets() const override;

  /// Reports the font-measured width/height of the current string.
  Dimensions getSuggestedMinimumDimensions() const override;

  const std::string& content() const { return value_; }

  const std::string& text() const { return value_; }

  /// Replaces the label string and invalidates the widget.
  void setText(std::string value);

  /// Convenience overload taking a C string.
  void setText(const char* value);

  /// Convenience overload taking a `roo::string_view`.
  void setText(roo::string_view value);

  /// Formats a new label using printf-style arguments.
  void setTextf(const char* format, ...);

  /// va_list variant of `setTextf()`.
  void setTextvf(const char* format, va_list arg);

  /// Empties the label.
  void clearText();

  const roo_display::Font& font() const { return font_; }

 private:
  std::string value_;
  const roo_display::Font& font_;
  roo_display::Color color_;
  Gravity gravity_;
};

/// Static text widget backed by a non-owning `roo::string_view`.
///
/// Avoids the per-instance allocation that `TextLabel` carries, at the cost
/// of requiring the caller to keep the underlying buffer alive for the widget's
/// lifetime. Ideal for constant or theme-supplied strings.
class StringViewLabel : public BasicWidget {
 public:
  StringViewLabel(const Environment& env, roo::string_view value,
                  const roo_display::Font& font);

  StringViewLabel(const Environment& env, roo::string_view value,
                  const roo_display::Font& font, Gravity gravity);

  StringViewLabel(const Environment& env, roo::string_view value,
                  const roo_display::Font& font, roo_display::Color color,
                  Gravity gravity);

  /// Paints the referenced string in a single line, with the configured
  /// gravity applied within the widget bounds.
  void paint(PaintContext& ctx) const override;

  /// Reports ink insets matching the rendered text rectangle.
  Insets getInkInsets() const override;

  /// Reports the font-measured width/height of the current string.
  Dimensions getSuggestedMinimumDimensions() const override;

  roo::string_view content() const { return value_; }

  roo::string_view text() const { return value_; }

  /// Rebinds the label to a different string view. The underlying buffer
  /// must outlive this widget.
  void setText(roo::string_view value);

  /// Resets the label to an empty view.
  void clearText();

  const roo_display::Font& font() const { return font_; }

 private:
  roo::string_view value_;
  const roo_display::Font& font_;
  roo_display::Color color_;
  Gravity gravity_;
};

}  // namespace roo_windows
