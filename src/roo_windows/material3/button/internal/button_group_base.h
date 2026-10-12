#pragma once
#include "roo_windows/core/container.h"
#include "roo_windows/core/layout_direction.h"
#include "roo_windows/material3/button/internal/button_group_entry.h"
namespace roo_windows::material3::internal {
/// Bounded typed membership and transparent one-row composition.
class ButtonGroupBase : public Container {
 public:
  /// Bounds masks and layout scratch without reserving capacity eagerly.
  static constexpr int kMaxEntries = 32;
  /// Creates small, round, LTR geometry with press morphing enabled.
  explicit ButtonGroupBase(ApplicationContext& context);
  /// Requires concrete policies to clear entries before their destruction.
  ~ButtonGroupBase() override;
  /// Returns the shared expressive size tier.
  ButtonSize size() const { return static_cast<ButtonSize>(size_); }
  /// Changes descendant metrics between completed UI frames.
  void setSize(ButtonSize size);
  /// Returns the shared resting shape.
  ButtonShape shape() const { return static_cast<ButtonShape>(shape_); }
  /// Changes resting contours and restores old decoration.
  void setShape(ButtonShape shape);
  /// Returns the shared press morphing policy.
  ButtonShapeMorph shapeMorph() const {
    return static_cast<ButtonShapeMorph>(morph_);
  }
  /// Enables or suppresses press width/corner morphing.
  void setShapeMorph(ButtonShapeMorph morph);
  /// Returns placement direction; attachment order is independent.
  LayoutDirection layoutDirection() const {
    return static_cast<LayoutDirection>(direction_);
  }
  /// Mirrors intervals and leading artwork without changing masks.
  void setLayoutDirection(LayoutDirection direction);
  /// Returns attached count, including invisible and gone entries.
  int entryCount() const { return entries_.size(); }
  /// Keeps ancestor content visible through gaps and rounded corners.
  Color background() const override { return roo_display::color::Transparent; }
  /// Introduces no surface role of its own.
  ColorToken containerRole() const override { return ColorToken::kNone; }
  /// Introduces no rectangular opaque coverage.
  bool fullyCoversBoundsWithOpaqueColors() const override { return false; }
  /// Introduces no area or click interaction layer.
  OverlayType getOverlayType() const override { return OVERLAY_NONE; }
  /// Child palettes resolve Material disabling once.
  bool useAutomaticDisabledStyle() const override { return false; }
  /// Groups have no semantic action or focus target.
  bool isClickable() const override { return false; }
  /// Entry shadows can overlap adjacent entries.
  bool respectsChildrenBoundaries() const override { return false; }
  /// Callers can opt mutable entries into unclipped painting.
  bool mayHaveUnclippedChildren() const override { return true; }
  /// Cancels child clicks without overwriting local enabled flags.
  void notifyStateChanged(uint16_t state_diff) override;
  /// Moves focus with physical arrows and logical Home/End, without selection.
  bool onKeyEvent(const KeyEvent& event) override;

 protected:
  /// Validates membership before appending and transfers temporary ownership.
  bool addEntry(ButtonGroupEntry* entry, WidgetRef reference);
  /// Severs all typed links before reverse structural detachment.
  void clearEntries();
  /// Distinguishes stable connected geometry from standard redistribution.
  virtual bool connected() const = 0;
  /// Returns canonical selection; standard groups return false.
  virtual bool selected(int index) const { return false; }
  /// Normalizes selection after structural attachment.
  virtual void entryAdded() {}
  /// Delivers an eligible entry activation according to the concrete policy.
  virtual void activateEntry(ButtonGroupEntry& entry) = 0;
  /// Projects a committed mask and restores changed old contours.
  void projectSelection(uint32_t previous, uint32_t current);
  /// Locates an entry's attachment index by bounded scan.
  int indexOf(const ButtonGroupEntry& entry) const;
  /// Resolves natural strip dimensions without retained geometry.
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  /// Measures exact frames before placing the stable row.
  void onLayout(bool changed, const Rect& rect) override;
  /// The transparent owner never excludes gaps from ancestor painting.
  Rect getDirectPaintExclusionBounds() const override {
    return Rect(0, 0, -1, -1);
  }
  int getChildrenCount() const override { return entryCount(); }
  const Widget& getChild(int index) const override { return *entries_[index]; }
  Widget& getChild(int index) override { return *entries_[index]; }
  std::vector<ButtonGroupEntry*> entries_;
  uint8_t size_ : 3;
  uint8_t shape_ : 1;
  uint8_t direction_ : 1;
  uint8_t morph_ : 2;

 private:
  friend class ButtonGroupEntry;
  /// Reports entry feedback through a no-allocation policy hook.
  void entryFeedbackChanged();
  /// Computes widths, optional content budgets and maximum row height.
  Dimensions measureStrip(int16_t* widths, int16_t* budgets = nullptr) const;
};
}  // namespace roo_windows::material3::internal
