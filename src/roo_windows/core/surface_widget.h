#pragma once

#include "roo_windows/core/widget.h"

namespace roo_windows {

// Explicit branch for widgets that own surface semantics such as background,
// border, outline, elevation, and surface-specific exclusion geometry.
class SurfaceWidget : public Widget {
 public:
  using Widget::Widget;

  /// Returns this widget's background. Transparent by default. Normally
  /// overridden by panels, which usually have opaque backgrounds.
  virtual Color background() const { return roo_display::color::Transparent; }

  /// Returns the effective background color of this widget. If this widget
  /// has a non-opaque background, it is returned. If this widget has a
  /// fully transparent background, the parent's effective background is
  /// returned. Otherwise, if this widget has a semi-transparent background,
  /// the returned color is the alpha-blend of the parent's effective
  /// background and this widget's background.
  virtual Color effectiveBackground() const;

  /// Returns the outline color. Has no effect when `getBorderStyle()`
  /// reports `outline_width = 0`.
  virtual Color getOutlineColor() const {
    return theme().frameworkColor(FrameworkColorRole::kOutline);
  }

  /// Returns the border style of this widget. By default, surface widgets
  /// have sharp corners and no outline. Subclasses can override.
  /// Call borderStyleChanged() with the previous style after changing it.
  virtual BorderStyle getBorderStyle() const { return BorderStyle(); }

  /// Returns the current elevation of the widget. Non-zero elevation causes
  /// a shadow to be drawn. With Gaussian shadows, elevation is a half-dp
  /// index in [0, 31]: 1 means 0.5 dp, 12 means 6 dp, and 31 means 15.5 dp.
  /// Decoration scales the resulting
  /// geometry for zoom; do not pre-scale the index.
  /// By default, surface widgets have elevation = 0. Subclasses can override.
  /// When the reported elevation changes in the subclass,
  /// `elevationChanged()` must be called to ensure that the screen is
  /// properly redrawn.
  virtual uint8_t getElevation() const { return 0; }

  /// Surface widgets use an area-shaped overlay (mirroring their interior).
  OverlayType getOverlayType() const override { return OVERLAY_AREA; }

  /// Returns true when the surface fully covers its rectangular interior
  /// with opaque pixels (i.e. no rounded corners). Subclasses with
  /// translucent fills must override.
  bool fullyCoversBoundsWithOpaqueColors() const override {
    // By default, surface widgets are treated as opaquely covering their
    // rectangular interior whenever their shape is rectangular. This keeps the
    // generic attach-time invalidation path independent from background()
    // lookups. Surface widgets that intentionally use translucent fills must
    // override this hook explicitly.
    return !getBorderStyle().hasRoundedCorners();
  }

  bool hasDecorationOverflow() const override;

  Rect getParentDecorationBounds() const override;

  /// Repaints content and transient effects without damaging this surface's
  /// unchanged shadow. Containers retain their visible descendants' overflow.
  void invalidateInterior() override;

  /// Repaints the given local area, restoring the backdrop at rounded edges.
  void invalidateInterior(const Rect& rect) override;

 protected:
  /// Repaints old and new shadow coverage after an elevation change. Pass the
  /// higher of the previous and current elevations, including when lowering
  /// elevation to zero. Also restores the scene beneath that coverage.
  void elevationChanged(int higherElevation);

  /// Repaints changed border geometry and, when rendered corner radii change,
  /// its shadow. Pass the style from before the change. Elevation changes must
  /// additionally call elevationChanged(); no appearance cache is retained.
  void borderStyleChanged(const BorderStyle& previous_style);

  void emitPersistentDecoration(PaintContext& ctx) const override;

  Canvas prepareCanvas(const Canvas& in) override;

  Canvas prepareContentsCanvas(const Canvas& in) override;

  // Surface widgets refine the generic exclusion contract to the surface
  // interior they actually own.
  Rect getDirectPaintExclusionBounds() const override;
};

}  // namespace roo_windows
