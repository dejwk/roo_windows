#pragma once

#include "roo_windows/core/surface_widget.h"
#include "roo_windows/material3/button/internal/button_group_appearance.h"

namespace roo_windows::material3::internal {

class ButtonGroupBase;

/// Surface entry with a typed, non-owning relationship to its group.
class ButtonGroupEntry : public SurfaceWidget {
 public:
  /// Creates an inert entry until typed group attachment.
  ButtonGroupEntry(ApplicationContext& context, GroupStyle style);

  /// Reports effective local and group enablement.
  bool effectivelyEnabled() const;

  /// Reports visibility and enablement through the complete ancestor path.
  bool eligible() const;

  /// Resolves exact natural dimensions and the compression content floor.
  Dimensions entryDimensions(int16_t* content_width = nullptr) const;

  /// Entries require live group membership to accept clicks or focus.
  bool isClickable() const override;

  /// Resolves the entry's semantic surface role.
  ColorToken containerRole() const override;

  /// Resolves contained colors from the group policy.
  Color background() const override;

  /// Resolves outlines independently of their reserved content space.
  Color getOutlineColor() const override;

  /// Resolves group position, selection and press corners.
  BorderStyle getBorderStyle() const override;

  /// Resolves group-aware elevation.
  uint8_t getElevation() const override;

  /// Suppresses overlays when effectively disabled.
  OverlayType getOverlayType() const override;

  /// Selects the content treatment for surface interaction layers.
  ColorToken effectiveOverlayColorRole() const override;

  /// Elevated entries use the primary interaction color.
  bool usesHighlighterColor() const override;

  /// Selection is encoded in the group palette.
  bool useOverlayOnSelection() const override { return false; }

  /// Persistent activation never alters the group treatment.
  bool useOverlayOnActivation() const override { return false; }

  /// Material disabled colors are already resolved once.
  bool useAutomaticDisabledStyle() const override { return false; }

  /// Avoids text measurement in suggested minimum queries.
  Dimensions getSuggestedMinimumDimensions() const override { return {0, 0}; }

  /// Leaves exact content measurement to onMeasure().
  PreferredSize getPreferredSize() const override;

  /// Prevents transparent surfaces from claiming opaque rectangular coverage.
  bool fullyCoversBoundsWithOpaqueColors() const override { return false; }

  /// Uses only the actual entry frame for sloppy touch bounds.
  Rect getSloppyTouchParentBounds() const override { return parent_bounds(); }

  /// Settles text, icons and padding in disjoint final-color tiles.
  void paint(PaintContext& context) const override;

  /// Revalidates delayed actions before dispatch to the group policy.
  void onClicked() override;

  /// Connected entries notify immediately and retain click feedback.
  ClickActivationPolicy getClickActivationPolicy() const override;

  /// Requests group geometry before painting a sampled click.
  void onClickAnimationChanged(float previous_progress) override;

  /// Repairs connected selected projections and reports shape damage.
  void notifyStateChanged(uint16_t state_diff) override;

 protected:
  /// Supplies borrowed text for the common metric and paint path.
  virtual roo::string_view contentLabel() const { return {}; }

  /// Supplies borrowed icon content, or nullptr for label-only entries.
  virtual const MonoIcon* contentIcon() const = 0;

  /// Chooses icon-only uniform-width geometry.
  virtual bool iconOnly() const { return false; }

  /// Changes appearance and restores prior corners and shadows.
  void changeStyle(GroupStyle style);

  /// Invalidates stable content and requests group measurement.
  void contentChanged();

  /// Resolves exact natural dimensions under the supplied constraints.
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  ButtonGroupBase* group_ = nullptr;
  uint8_t style_ : 2;

 private:
  friend class ButtonGroupBase;

  /// Resolves old and new contours without retained animation state.
  BorderStyle resolveBorder(bool selected, bool pressed, float progress,
                            bool enabled) const;

  /// Repaints the canonical selection transition's old coverage.
  void selectionChanged(bool previous_selected);

  /// Restores decorations affected by group enablement changes.
  void groupEnabledChanged(bool previous_effectively_enabled);

  /// Returns the family's reserved outline inset, including selected entries.
  int outlineInset() const;
};

/// Internal borrowed label/icon entry used before publishing the public family.
class ButtonGroupLabelEntry : public ButtonGroupEntry {
 public:
  /// Creates a label entry borrowing @p label.
  ButtonGroupLabelEntry(ApplicationContext& context,
                        roo::string_view label = {},
                        GroupStyle style = GroupStyle::kFilled);

  /// Returns the borrowed single-line label.
  roo::string_view label() const { return label_; }

  /// Replaces borrowed text without restarting click feedback.
  void setLabel(roo::string_view label);

  /// Returns the borrowed leading icon, or nullptr.
  const MonoIcon* icon() const { return icon_; }

  /// Replaces the borrowed leading icon.
  void setIcon(const MonoIcon* icon);

 protected:
  roo::string_view contentLabel() const override { return label_; }
  const MonoIcon* contentIcon() const override { return icon_; }

 private:
  roo::string_view label_;
  const MonoIcon* icon_ = nullptr;
};

/// Internal icon-only entry with no independently retained geometry.
class ButtonGroupIconEntry : public ButtonGroupEntry {
 public:
  /// Creates a uniform-width entry borrowing @p icon.
  ButtonGroupIconEntry(ApplicationContext& context, const MonoIcon& icon,
                       GroupStyle style = GroupStyle::kFilled);

  /// Returns the borrowed artwork.
  const MonoIcon& icon() const { return *icon_; }

  /// Replaces borrowed artwork without restarting click feedback.
  void setIcon(const MonoIcon& icon);

 protected:
  const MonoIcon* contentIcon() const override { return icon_; }
  bool iconOnly() const override { return true; }

 private:
  const MonoIcon* icon_;
};
}  // namespace roo_windows::material3::internal
