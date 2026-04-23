#pragma once

#include <vector>

#include "roo_windows/core/cached_measure.h"
#include "roo_windows/core/margins.h"
#include "roo_windows/core/padding.h"
#include "roo_windows/core/panel.h"

namespace roo_windows {

/// @file
///
/// @brief A flexible box layout container modelled after the CSS Flexible Box
/// Model (Flexbox).
///
/// ---
/// ## Core concepts
///
/// ### Main axis and cross axis
///
/// A FlexLayout places its children along one primary direction called the
/// **main axis**. The perpendicular direction is the **cross axis**.
///
/// The main axis is selected by @ref FlexDirection:
/// - `kRow` / `kRowReverse`       — main axis is horizontal, cross is vertical.
/// - `kColumn` / `kColumnReverse` — main axis is vertical, cross is horizontal.
///
/// Each axis has a *start* edge and an *end* edge:
/// | Direction      | main-start | main-end | cross-start | cross-end |
/// |----------------|-----------|---------|------------|---------|
/// | kRow           | left      | right   | top        | bottom  |
/// | kRowReverse    | right     | left    | top        | bottom  |
/// | kColumn        | top       | bottom  | left       | right   |
/// | kColumnReverse | bottom    | top     | left       | right   |
///
/// ---
/// ### Flex lines
///
/// Children are grouped into one or more **flex lines** depending on
/// @ref FlexWrap:
///
/// - `kNowrap` (default): all children share a single line. If they do not
///   fit, they are shrunk (see `flex_shrink` in @ref FlexLayout::Params).
/// - `kWrap`: when a child does not fit on the current line, a new line is
///   started further along the cross axis.
/// - `kWrapReverse`: same as `kWrap` but new lines accumulate in the
///   reverse cross direction.
///
/// ---
/// ### Size resolution (per line, three steps)
///
/// 1. **Hypothesis** – every item is sized to its `flex_basis` (either the
///    widget's natural preferred size, or an explicit pixel value).
/// 2. **Grow** – if free main-axis space remains after step 1, items with
///    `flex_grow > 0` share it proportionally to their grow factors.
/// 3. **Shrink** – if items overflow the line, items with `flex_shrink > 0`
///    absorb the deficit. The reduction per item is weighted by
///    `flex_shrink × flex_basis` so that larger items shrink more than small
///    ones with the same factor.
///
/// ---
/// ### Alignment
///
/// - **justify-content** (@ref JustifyContent): how children and remaining
///   space are arranged along the *main* axis within each line.
/// - **align-items** (@ref AlignItems): how each child sits within its line
///   along the *cross* axis. Individual items may override this via
///   `align_self` in @ref FlexLayout::Params.
/// - **align-content** (@ref AlignContent): how multiple lines are
///   distributed along the cross axis (only relevant when wrapping).
///
/// ---
/// ### Relationship to existing layouts
///
/// | Configuration                    | Similar to          |
/// |----------------------------------|---------------------|
/// | `kRow`    + `kNowrap`            | HorizontalLayout    |
/// | `kColumn` + `kNowrap`            | VerticalLayout      |
///
/// FlexLayout additionally supports wrapping, all justify-content modes, and
/// richer per-item alignment.

// ---------------------------------------------------------------------------
// FlexDirection
// ---------------------------------------------------------------------------

/// @brief Selects the main axis and its direction.
///
/// The main axis is the direction in which children are placed one after
/// another. The cross axis is perpendicular to it.
enum class FlexDirection {
  kRow,         ///< Main axis left → right (default). Cross axis is vertical.
  kRowReverse,  ///< Main axis right → left. Cross axis is vertical.
  kColumn,      ///< Main axis top → bottom. Cross axis is horizontal.
  kColumnReverse,  ///< Main axis bottom → top. Cross axis is horizontal.
};

/// @brief Controls whether children wrap onto multiple flex lines.
///
/// When wrapping is enabled and a child does not fit on the current line,
/// it starts a new line in the cross-axis direction.
enum class FlexWrap {
  kNowrap,  ///< All children forced onto one line (default). Items may shrink
            ///< or overflow if the line's main-axis space is exhausted.
  kWrap,    ///< Children wrap onto additional lines in the cross-start →
            ///< cross-end direction (downward for kRow).
  kWrapReverse,  ///< Like kWrap but new lines accumulate toward the cross-start
                 ///< edge (upward for kRow).
};

/// @brief Controls how children and remaining free space are distributed
/// along the main axis within each flex line.
///
/// "Free space" is main-axis space left over after all items have been sized
/// (after grow/shrink). These modes determine where that surplus is placed.
enum class JustifyContent {
  kFlexStart,     ///< Children packed toward the main-start edge; surplus goes
                  ///< to the end (default).
  kFlexEnd,       ///< Children packed toward the main-end edge; surplus goes
                  ///< to the start.
  kCenter,        ///< Children clustered in the middle; surplus split equally
                  ///< between both edges.
  kSpaceBetween,  ///< Surplus divided equally into the gaps *between* children.
                  ///< No space before the first child or after the last.
  kSpaceAround,   ///< Each child gets equal space on both of its sides. The
                  ///< gap between adjacent children is therefore twice the gap
                  ///< at the edges.
  kSpaceEvenly,  ///< All gaps — between children and at both edges — are equal.
};

/// @brief Default cross-axis alignment applied to all children that do not
/// specify their own @ref AlignSelf.
///
/// Controls how each child is positioned within its flex line in the cross-
/// axis direction. For a kRow container the cross axis is vertical, so
/// these values control vertical positioning within the row.
enum class AlignItems {
  kStretch,    ///< Children stretched to fill the full cross-axis extent of
               ///< their line (default). A child must not have an exact
               ///< cross-axis preferred size to be stretched.
  kFlexStart,  ///< Children aligned to the cross-start edge of their line.
  kFlexEnd,    ///< Children aligned to the cross-end edge of their line.
  kCenter,     ///< Children centred on the cross axis of their line.
};

/// @brief Distributes flex lines along the cross axis when the container has
/// more cross-axis space than the lines collectively occupy.
///
/// Only has a visible effect when @ref FlexWrap is `kWrap` or `kWrapReverse`
/// and there are at least two flex lines. For a single-line container this
/// setting is ignored.
///
/// The semantics mirror @ref JustifyContent but apply to *lines* rather than
/// to individual items.
enum class AlignContent {
  kStretch,       ///< Lines stretched to fill the container's cross-axis
                  ///< extent; surplus distributed equally among all lines
                  ///< (default).
  kFlexStart,     ///< Lines packed toward the cross-start edge.
  kFlexEnd,       ///< Lines packed toward the cross-end edge.
  kCenter,        ///< Lines centred in the container on the cross axis.
  kSpaceBetween,  ///< Surplus divided equally between lines. No space before
                  ///< the first or after the last line.
  kSpaceAround,   ///< Each line gets equal space on both of its cross-axis
                  ///< sides.
  kSpaceEvenly,   ///< All gaps between lines, and at both edges, are equal.
};

/// @brief Per-item override for the container's @ref AlignItems.
///
/// Set in @ref FlexLayout::Params to make one child deviate from the
/// container-wide cross-axis alignment without affecting other children.
enum class AlignSelf {
  kAuto,       ///< Inherit the container's AlignItems setting (default).
  kStretch,    ///< Stretch to fill the line's full cross-axis extent.
  kFlexStart,  ///< Align to the cross-start edge of the line.
  kFlexEnd,    ///< Align to the cross-end edge of the line.
  kCenter,     ///< Centre on the cross axis of the line.
};

/// @brief The initial main-axis size of a flex item before grow/shrink.
///
/// This is the *starting point* used in step 1 of size resolution. After all
/// items in a line are sized to their basis, the engine grows or shrinks them
/// to fill the available space.
enum class FlexBasis : uint8_t {
  kAuto,  ///< Start from the widget's preferred main-axis size (CSS `auto`).
          ///< The widget is measured normally; the result becomes the
          ///< flex-basis, and grow/shrink are then applied on top of it.
  kZero,  ///< Start from zero, so *all* available space is free space to be
          ///< divided by `flex_grow`. Equivalent to the CSS shorthand `flex: N`
          ///< (e.g. `flex: 1`). Use this for equal-width / equal-height slots.
};

/// @brief Container that lays out children using the Flexible Box Model.
///
/// See the file-level documentation above for a full explanation of main axis,
/// cross axis, flex lines, size resolution, and alignment.
///
/// ### Quick-start
///
/// ```cpp
/// FlexLayout row(env, FlexDirection::kRow);
/// row.setAlignItems(AlignItems::kCenter);
/// row.setGap(Scaled(8));
///
/// // Icon: fixed size, never grows or shrinks.
/// row.add(WidgetRef(icon), {.flex_grow = 0, .flex_shrink = 0});
/// // Label: absorbs all remaining width.
/// row.add(WidgetRef(label), {.flex_grow = 1});
/// // Button: fixed size.
/// row.add(WidgetRef(btn),  {.flex_grow = 0, .flex_shrink = 0});
/// ```
class FlexLayout : public Panel {
 public:
  /// Per-child layout parameters (CSS flex item properties).
  /// Items are laid out in the order they are added via add().
  struct Params {
    /// CSS `flex-grow` factor. Dimensionless, non-negative integer ratio.
    ///
    /// After all items in a flex line have been sized to their flex-basis, any
    /// leftover main-axis space is distributed among items with a non-zero
    /// flex_grow. Each item receives a slice proportional to its own
    /// flex_grow divided by the sum of all flex_grow values in the line.
    ///
    /// Example: three items with flex_grow 1, 1, 2 receive 25 %, 25 %, 50 % of
    /// the free space respectively.
    ///
    /// 0 = does not grow (default).
    uint8_t flex_grow = 0;

    /// CSS `flex-shrink` factor. Dimensionless, non-negative integer ratio.
    ///
    /// When the combined flex-basis of all items on a line exceeds the
    /// container's main-axis size, the overflow is distributed as a reduction
    /// among items with a non-zero flex_shrink. Each item shrinks by an amount
    /// proportional to (flex_shrink × flex_basis) / Σ(flex_shrink_i ×
    /// flex_basis_i) across all shrinkable items in the line.
    ///
    /// The (flex_shrink × flex_basis) weighting means a larger item absorbs
    /// proportionally more of the deficit than a smaller one with the same
    /// flex_shrink value, matching CSS behaviour.
    ///
    /// 0 = does not shrink. 1 = shrinks proportionally (default).
    uint8_t flex_shrink = 1;

    /// CSS `flex-basis`: initial main-axis size before grow/shrink is applied.
    ///
    /// - `FlexBasis::kAuto` (default): use the widget's preferred main-axis
    ///   size as the starting point, then adjust via grow/shrink.
    /// - `FlexBasis::kZero`: start from zero, so *all* space is free space
    ///   divided by flex_grow — equivalent to the CSS shorthand `flex: N`
    ///   (e.g. `flex: 1`). Use this for equal-width/equal-height slots.
    FlexBasis flex_basis = FlexBasis::kAuto;

    /// CSS `align-self`: per-item override for the container's align-items.
    /// `AlignSelf::kAuto` (default) delegates to the container's AlignItems.
    AlignSelf align_self = AlignSelf::kAuto;
  };

  /// @brief Stores per-child layout parameters together with its last cached
  /// measurement result.
  ///
  /// Kept in the same order as `Panel::children_`.
  class ChildMeasure {
   public:
    explicit ChildMeasure(Params params) : params_(params), latest_() {}

    const Params& params() const { return params_; }

    const CachedMeasure& latest() const { return latest_; }
    CachedMeasure& latest() { return latest_; }

   private:
    Params params_;
    CachedMeasure latest_;
  };

  /// @brief Constructs a FlexLayout with the given initial flex-direction.
  ///
  /// All other properties default to their CSS initial values:
  /// - `flex_wrap`       = `kNowrap`
  /// - `justify_content` = `kFlexStart`
  /// - `align_items`     = `kStretch`
  /// - `align_content`   = `kStretch`
  /// - `column_gap`      = 0
  /// - `row_gap`         = 0
  explicit FlexLayout(const Environment& env,
                      FlexDirection direction = FlexDirection::kRow);

  // -----------------------------------------------------------------------
  // Container-level properties
  // -----------------------------------------------------------------------

  /// @brief Sets the main axis direction.
  /// @see FlexDirection
  void setFlexDirection(FlexDirection direction);

  /// @returns The current flex-direction.
  FlexDirection flexDirection() const { return flex_direction_; }

  /// @brief Controls whether children may wrap onto multiple flex lines.
  /// @see FlexWrap
  void setFlexWrap(FlexWrap wrap);

  /// @returns The current flex-wrap mode.
  FlexWrap flexWrap() const { return flex_wrap_; }

  /// @brief Sets how children and free space are distributed along the main
  /// axis within each line.
  /// @see JustifyContent
  void setJustifyContent(JustifyContent justify);

  /// @returns The current justify-content mode.
  JustifyContent justifyContent() const { return justify_content_; }

  /// @brief Sets the default cross-axis alignment for all children.
  ///
  /// Individual items may override this via `align_self` in @ref Params.
  /// @see AlignItems
  void setAlignItems(AlignItems align);

  /// @returns The current align-items mode.
  AlignItems alignItems() const { return align_items_; }

  /// @brief Sets how multiple flex lines are distributed along the cross axis.
  ///
  /// Only takes effect when @ref flexWrap() is `kWrap` or `kWrapReverse` and
  /// the container has more than one line.
  /// @see AlignContent
  void setAlignContent(AlignContent align);

  /// @returns The current align-content mode.
  AlignContent alignContent() const { return align_content_; }

  /// @brief Convenience setter — assigns the same value to both
  /// @ref setColumnGap() and @ref setRowGap().
  /// @param gap  Gap size in pixels.
  void setGap(int16_t gap);

  /// @brief Sets the spacing between adjacent items along the main axis, and
  /// between flex lines along the cross axis, for a horizontal main axis.
  ///
  /// The name follows the physical CSS convention: "column" refers to the
  /// horizontal dimension regardless of flex-direction. Use
  /// @ref mainAxisGap() / @ref crossAxisGap() inside the implementation to
  /// abstract over direction.
  ///
  /// @param gap  Gap size in pixels.
  void setColumnGap(int16_t gap);

  /// @returns The current column gap in pixels.
  int16_t columnGap() const { return column_gap_; }

  /// @brief Sets the spacing between flex lines along the cross axis, and
  /// between adjacent items along the main axis, for a vertical main axis.
  /// @param gap  Gap size in pixels.
  void setRowGap(int16_t gap);

  /// @returns The current row gap in pixels.
  int16_t rowGap() const { return row_gap_; }

  /// @brief Sets the container's inner padding (space between the border and
  /// the first/last flex line).
  void setPadding(Padding padding);

  /// @brief Sets the container's outer margins.
  void setMargins(Margins margins);

  Padding getPadding() const override { return padding_; }
  Margins getMargins() const override { return margins_; }

  PreferredSize getPreferredSize() const override {
    return PreferredSize(PreferredSize::WrapContentWidth(),
                         PreferredSize::WrapContentHeight());
  }

  Dimensions getSuggestedMinimumDimensions() const override {
    return min_dimensions_;
  }

  /// @brief Sets a minimum size the container reports even when its children
  /// would result in a smaller measurement.
  void setMinimumDimensions(Dimensions dimensions) {
    min_dimensions_ = dimensions;
  }

  // -----------------------------------------------------------------------
  // Child management
  // -----------------------------------------------------------------------

  /// @brief Appends a child widget with the given flex item parameters.
  ///
  /// Children are placed in the order they are added.
  ///
  /// @param child   The widget to add.
  /// @param params  Flex item parameters. Defaults to `Params{}` which matches
  ///                CSS initial values (no grow, shrink=1, auto basis).
  void add(WidgetRef child, Params params);
  void add(WidgetRef child) { add(std::move(child), Params{}); }

  bool respectsChildrenBoundaries() const override { return true; }

 protected:
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  // Returns the gap between adjacent items along the main axis.
  int16_t mainAxisGap() const;

  // Returns the gap between adjacent flex lines along the cross axis.
  int16_t crossAxisGap() const;

  FlexDirection flex_direction_;
  FlexWrap flex_wrap_;
  JustifyContent justify_content_;
  AlignItems align_items_;
  AlignContent align_content_;

  // Physical gaps: column_gap_ is horizontal, row_gap_ is vertical.
  // mainAxisGap() / crossAxisGap() map these to the current flex-direction.
  int16_t column_gap_;
  int16_t row_gap_;

  Padding padding_;
  Margins margins_;
  Dimensions min_dimensions_;

  // Parallel to Panel::children_: index i here corresponds to index i there.
  std::vector<ChildMeasure> child_measures_;
};

}  // namespace roo_windows
