#pragma once

#include <stdint.h>

#include "roo_backport/string_view.h"
#include "roo_windows/core/surface_widget.h"

namespace roo_windows::material3 {

enum class FabSize : uint8_t { kSmall, kRegular, kMedium, kLarge };
enum class ExtendedFabSize : uint8_t { kSmall, kMedium, kLarge };
enum class FabColorStyle : uint8_t {
  kPrimaryContainer,
  kSecondaryContainer,
  kTertiaryContainer,
  kPrimary,
  kSecondary,
  kTertiary,
  kSurface,
};

/// Icon-only Material 3 floating action button. The icon is borrowed.
class FloatingActionButton : public SurfaceWidget {
 public:
  explicit FloatingActionButton(
      ApplicationContext& context, const MonoIcon& icon,
      FabSize size = FabSize::kRegular,
      FabColorStyle color_style = FabColorStyle::kPrimaryContainer);

  const MonoIcon& icon() const { return *icon_; }
  void setIcon(const MonoIcon& icon);
  FabSize size() const { return static_cast<FabSize>(size_); }
  void setSize(FabSize size);
  FabColorStyle colorStyle() const {
    return static_cast<FabColorStyle>(color_style_);
  }
  void setColorStyle(FabColorStyle style);

  bool isClickable() const override { return true; }
  bool useAutomaticDisabledStyle() const override { return false; }
  uint8_t getOverlayOpacity() const override {
    return isEnabled() ? SurfaceWidget::getOverlayOpacity() : 0;
  }
  ColorToken containerRole() const override;
  Color background() const override;
  BorderStyle getBorderStyle() const override;
  uint8_t getElevation() const override;
  bool usesHighlighterColor() const override;
  Dimensions getSuggestedMinimumDimensions() const override;
  Dimensions getNaturalDimensions() const override;
  void paint(PaintContext& ctx) const override;

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void notifyStateChanged(uint16_t state_diff) override;

 private:
  const MonoIcon* icon_;
  uint8_t size_ : 2;
  uint8_t color_style_ : 3;
};

/// Label-bearing Material 3 FAB. Icon and label storage are borrowed.
class ExtendedFloatingActionButton : public SurfaceWidget {
 public:
  explicit ExtendedFloatingActionButton(
      ApplicationContext& context, roo::string_view label, const MonoIcon& icon,
      ExtendedFabSize size = ExtendedFabSize::kSmall,
      FabColorStyle color_style = FabColorStyle::kPrimaryContainer);

  roo::string_view label() const { return label_; }
  void setLabel(roo::string_view label);
  const MonoIcon& icon() const { return *icon_; }
  void setIcon(const MonoIcon& icon);
  ExtendedFabSize size() const { return static_cast<ExtendedFabSize>(size_); }
  void setSize(ExtendedFabSize size);
  FabColorStyle colorStyle() const {
    return static_cast<FabColorStyle>(color_style_);
  }
  void setColorStyle(FabColorStyle style);
  bool expanded() const { return expanded_; }
  void setExpanded(bool expanded);

  bool isClickable() const override { return true; }
  bool useAutomaticDisabledStyle() const override { return false; }
  uint8_t getOverlayOpacity() const override {
    return isEnabled() ? SurfaceWidget::getOverlayOpacity() : 0;
  }
  ColorToken containerRole() const override;
  Color background() const override;
  BorderStyle getBorderStyle() const override;
  uint8_t getElevation() const override;
  bool usesHighlighterColor() const override;
  Dimensions getSuggestedMinimumDimensions() const override;
  Dimensions getNaturalDimensions() const override;
  void paint(PaintContext& ctx) const override;

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void notifyStateChanged(uint16_t state_diff) override;

 private:
  roo::string_view label_;
  const MonoIcon* icon_;
  uint8_t size_ : 2;
  uint8_t color_style_ : 3;
  uint8_t expanded_ : 1;
};

}  // namespace roo_windows::material3
