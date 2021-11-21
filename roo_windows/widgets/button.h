#pragma once

#include <string>

#include "roo_windows/core/widget.h"

namespace roo_windows {

class Button : public Widget {
 public:
  enum Style { CONTAINED, OUTLINED, TEXT };

  Button(const Environment& env, const MonoIcon& icon, Style style = CONTAINED)
      : Button(env, &icon, "", style) {}

  Button(const Environment& env, std::string label, Style style = CONTAINED)
      : Button(env, nullptr, label, style) {}

  Button(const Environment& env, const MonoIcon& icon, std::string label,
         Style style = CONTAINED)
      : Button(env, &icon, label, style) {}

  bool hasLabel() const { return !label_.empty(); }
  const std::string& label() const { return label_; }

  bool hasIcon() const { return icon_ != nullptr; }

  // Must not be called if hasIcon() returns true.
  const MonoIcon& icon() const { return *icon_; }

  Style style() const { return style_; }

  virtual const roo_display::Font& getFont() const {
    return *env_.theme().font.button;
  }

  roo_display::Color textColor() const { return textColor_; }

  void setTextColor(roo_display::Color color) {
    textColor_ = color;
    markDirty();
  }

  roo_display::Color interiorColor() const { return interiorColor_; }

  void setInteriorColor(roo_display::Color color) {
    interiorColor_ = color;
    markDirty();
  }

  roo_display::Color outlineColor() const { return outlineColor_; }

  void setOutlineColor(roo_display::Color color) {
    outlineColor_ = color;
    markDirty();
  }

  bool paint(const Surface& s) override;

  Dimensions getSuggestedMinimumDimensions() const override;
  Padding getDefaultPadding() const override;

  bool isClickable() const override { return true; }

 protected:
  const Environment& env() const { return env_; }

 private:
  Button(const Environment& env, const MonoIcon* icon, std::string label,
         Style style);

  const Environment& env_;
  Style style_;
  std::string label_;
  const MonoIcon* icon_;
  roo_display::Color outlineColor_;
  roo_display::Color interiorColor_;
  roo_display::Color textColor_;
};

}  // namespace roo_windows
