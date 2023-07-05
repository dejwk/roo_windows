#pragma once

#include "roo_display/image/image.h"
#include "roo_windows/core/basic_widget.h"

namespace roo_windows {

class Icon : public BasicWidget {
 public:
  Icon(const Environment& env, Color color = roo_display::color::Transparent)
      : BasicWidget(env), icon_(nullptr), color_(color) {}

  Icon(const Environment& env, const roo_display::Pictogram& def,
       Color color = roo_display::color::Transparent)
      : BasicWidget(env), icon_(&def), color_(color) {}

  void paint(const Canvas& canvas) const override;

  const roo_display::Pictogram& icon() const { return *icon_; }

  void setIcon(const roo_display::Pictogram& icon);

  Dimensions getSuggestedMinimumDimensions() const override;

  roo_display::Color color() const { return color_; }

  void setColor(roo_display::Color color);

  OverlayType getOverlayType() const override { return OVERLAY_POINT; }

 private:
  const roo_display::Pictogram* icon_;
  roo_display::Color color_;
};

}  // namespace roo_windows
