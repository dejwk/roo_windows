#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "roo_windows/core/layout_direction.h"
#include "roo_windows/core/widget_ref.h"
#include "roo_windows/material3/container.h"

namespace roo_windows::material3 {

/// Selects the toolbar surface without changing its children's appearance.
enum class ToolbarColorStyle : uint8_t { kStandard, kVibrant };

/// Selects a docked action's logical placement group.
enum class ToolbarSlot : uint8_t { kLeading, kCenter, kTrailing };

/// Selects how a docked toolbar distributes unused horizontal space.
enum class DockedToolbarArrangement : uint8_t {
  kEvenlySpaced,
  kCentered,
  kEdgeAnchored,
};

/// A full-width Material 3 page-action surface for a scaffold bottom bar.
/// Children keep their own actions, styles, focus, and ownership policy.
class DockedToolbar : public Material3Container {
 public:
  /// Creates an empty toolbar in @p context with evenly spaced, LTR actions.
  explicit DockedToolbar(ApplicationContext& context);

  /// Detaches all controls, deleting those adopted by this toolbar.
  ~DockedToolbar() override;

  /// Appends @p child to @p slot; null is ignored. The child must be detached,
  /// have this context, and have nonnegative margins. Adopted children are
  /// deleted on removal; borrowed children must outlive their attachment.
  void add(WidgetRef child, ToolbarSlot slot = ToolbarSlot::kTrailing);

  /// Detaches every child while retaining vector capacity for reuse.
  void clear();

  /// Detaches children in @p slot, preserving other children's order.
  void clear(ToolbarSlot slot);

  /// Counts all attached children, including invisible and gone controls.
  int childCount() const;

  /// Returns the selected toolbar surface treatment.
  ToolbarColorStyle colorStyle() const;

  /// Changes the surface treatment and invalidates transparent descendants.
  void setColorStyle(ToolbarColorStyle style);

  /// Returns the active horizontal arrangement.
  DockedToolbarArrangement arrangement() const;

  /// Changes placement of existing and future controls.
  void setArrangement(DockedToolbarArrangement arrangement);

  /// Returns whether occupied groups have separators.
  bool showsGroupDividers() const;

  /// Shows or hides separators without changing child positions.
  void setShowGroupDividers(bool show);

  /// Returns the explicit logical horizontal direction.
  LayoutDirection layoutDirection() const;

  /// Mirrors group placement without changing child direction or glyphs.
  void setLayoutDirection(LayoutDirection direction);

  /// Resolves the standard shared role or vibrant primary-container role.
  ColorToken containerRole() const override;

  /// Returns fixed 8 dp inner padding on both axes.
  Padding getPadding() const override;

  /// Fills bounded width and wraps height.
  PreferredSize getPreferredSize() const override;

  /// Allows caller-supplied child decoration to escape this surface.
  bool mayHaveUnclippedChildren() const override { return true; }

  /// Paints optional separators before unresolved surface pixels.
  void paint(PaintContext& ctx) const override;

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int index) const override;
  Widget& getChild(int index) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  struct Entry {
    Widget* widget;
    YDim measured_height;
    XDim measured_width;
    ToolbarSlot slot;
  };
  static_assert(sizeof(Entry) <= (sizeof(void*) == 4 ? 12 : 16),
                "Docked toolbar entries exceed the target RAM budget");

  struct GroupGeometry {
    int64_t width = 0;
    int count = 0;
    int64_t start = 0;
    int64_t end = 0;
  };

  void computeGroups(GroupGeometry (&groups)[3], int64_t& packed,
                     int& count) const;
  void placeGroups(GroupGeometry (&groups)[3], int64_t packed, int count) const;
  int64_t itemMainExtent(const Entry& entry) const;

  std::vector<Entry> children_;
  uint8_t color_style_ : 1;
  uint8_t arrangement_ : 2;
  uint8_t show_group_dividers_ : 1;
  uint8_t direction_ : 1;
};

/// Selects whether a floating toolbar stacks controls across or down a page.
enum class FloatingToolbarOrientation : uint8_t { kHorizontal, kVertical };

/// A content-sized Material 3 action strip with a rounded surface and shadow.
/// Its host supplies placement and keeps page content clear of the toolbar.
/// Each child's margin box is centered in a slot of at least 48 dp per axis.
/// The strip adds 8 dp padding and 4 dp between slots.
class FloatingToolbar : public Material3Container {
 public:
  /// Creates an empty horizontal, elevated, standard-color strip in @p context.
  explicit FloatingToolbar(ApplicationContext& context);

  /// Detaches all controls, deleting those adopted by this toolbar.
  ~FloatingToolbar() override;

  /// Appends @p child. Null is ignored; a non-null child must be detached,
  /// share this context, and have nonnegative margins. Borrowed children must
  /// outlive their attachment; adopted children are deleted on removal.
  void add(WidgetRef child);

  /// Detaches every child and retains vector capacity for reuse.
  void clear();

  /// Counts all attached children, including invisible and gone controls.
  int childCount() const;

  /// Returns the selected surface treatment.
  ToolbarColorStyle colorStyle() const;

  /// Changes the surface treatment and invalidates descendants.
  void setColorStyle(ToolbarColorStyle style);

  /// Returns whether the controls run horizontally or vertically.
  FloatingToolbarOrientation orientation() const;

  /// Changes the strip axis and requests layout.
  void setOrientation(FloatingToolbarOrientation orientation);

  /// Returns whether the floating surface casts its level-2 shadow.
  bool elevated() const;

  /// Enables or disables the level-2 shadow and restores affected pixels.
  void setElevated(bool elevated);

  /// Returns the explicit horizontal direction.
  LayoutDirection layoutDirection() const;

  /// Mirrors horizontal placement without changing child glyphs or direction.
  void setLayoutDirection(LayoutDirection direction);

  /// Resolves the shared standard role or vibrant primary-container role.
  ColorToken containerRole() const override;

  /// Returns fixed 8 dp padding on both axes.
  Padding getPadding() const override;

  /// Rounds the actual outer bounds with no outline.
  BorderStyle getBorderStyle() const override;

  /// Resolves level 2 when elevated, zero otherwise.
  uint8_t getElevation() const override;

  /// Allows caller-supplied child decoration outside the surface.
  bool mayHaveUnclippedChildren() const override { return true; }

  /// Clips ordinary child content to the rounded surface.
  bool clipsChildrenToRoundedBounds() const override { return true; }

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int index) const override;
  Widget& getChild(int index) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  struct Entry {
    Widget* widget;
    YDim measured_height;
    XDim measured_width;
  };
  static_assert(sizeof(Entry) <= (sizeof(void*) == 4 ? 12 : 16),
                "Floating toolbar entries exceed the target RAM budget");

  std::vector<Entry> children_;
  uint8_t color_style_ : 1;
  uint8_t orientation_ : 1;
  uint8_t elevated_ : 1;
  uint8_t direction_ : 1;
};

}  // namespace roo_windows::material3
