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

  /// Returns the semantic container role owned by this surface widget. This
  /// is the surface-facing public accessor. Returning kUndefined means that
  /// this surface does not introduce a new role and instead inherits the
  /// effective role from its ancestors.
  virtual ColorRole containerRole() const { return ColorRole::kUndefined; }

  /// Resolves the inherited or surface-owned container role for theme
  /// lookups.
  ColorRole effectiveContainerRole() const override;

  /// Returns the effective background color of this widget. If this widget
  /// has a non-opaque background, it is returned. If this widget has a
  /// fully transparent background, the parent's effective background is
  /// returned. Otherwise, if this widget has a semi-transparent background,
  /// the returned color is the alpha-blend of the parent's effective
  /// background and this widget's background.
  virtual Color effectiveBackground() const;

  /// Returns the outline color. Has no effect when `getBorderStyle()`
  /// reports `outline_width = 0`.
  virtual Color getOutlineColor() const { return theme().color.primary; }

  /// Returns the border style of this widget. By default, surface widgets
  /// have sharp corners and no outline. Subclasses can override.
  virtual BorderStyle getBorderStyle() const { return BorderStyle(); }

  /// Returns the current elevation of the widget. Non-zero elevation causes
  /// a shadow to be drawn. The elevation can be in the range [0-31]. By
  /// default, surface widgets have elevation = 0. Subclasses can override.
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

  void invalidateInterior() override;

  void invalidateInterior(const Rect& rect) override;

 protected:
  // Should be called by a child whose elevation has changed, and the child
  // wants the shadow to be redrawn. The argument should indicate the higher of
  // {before, after} elevations.
  void elevationChanged(int higherElevation);

  void emitPersistentDecoration(PaintContext& ctx) const override;

  Canvas prepareCanvas(const Canvas& in) override;

  Canvas prepareContentsCanvas(const Canvas& in) override;

  // Surface widgets refine the generic exclusion contract to the surface
  // interior they actually own.
  Rect getDirectPaintExclusionBounds() const override;

};

}  // namespace roo_windows