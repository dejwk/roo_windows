#pragma once

#include <stddef.h>
#include <stdint.h>

#include <memory>

#include "roo_backport/string_view.h"
#include "roo_windows/core/surface_widget.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/density.h"
#include "roo_windows/material3/menu/menu.h"

namespace roo_windows::material3 {

/// A Material 3 button that selects one item from a borrowed string table.
/// The pointer array and strings must remain unchanged and alive until a
/// successful setItems() call replaces them or the button is destroyed.
class DropdownButton : public SurfaceWidget {
 public:
  static constexpr size_t kNoSelection = static_cast<size_t>(-1);

  /// Creates a selector borrowing @p items and their strings; starts at zero.
  /// Invalid tables assert in debug builds and become empty in release builds.
  DropdownButton(ApplicationContext& context, const char* const* items,
                 size_t count, ButtonVariant variant = ButtonVariant::kFilled);

  /// Deduces the count of a caller-owned constant pointer array.
  template <size_t N>
  DropdownButton(ApplicationContext& context, const char* const (&items)[N],
                 ButtonVariant variant = ButtonVariant::kFilled)
      : DropdownButton(context, items, N, variant) {}

  /// Cancels any menu and releases all borrowed choices.
  ~DropdownButton() override;

  /// Replaces the borrowed table and silently selects @p selected_index.
  /// Valid replacement releases old borrows before return; invalid input leaves
  /// the current choices and selection unchanged. Zero or kNoSelection is
  /// accepted for an empty replacement.
  bool setItems(const char* const* items, size_t count, size_t selected_index);

  /// Replaces choices from an array, selecting its first item by default.
  template <size_t N>
  bool setItems(const char* const (&items)[N], size_t selected_index = 0) {
    return setItems(items, N, selected_index);
  }

  /// Returns the number of choices in the currently borrowed table.
  size_t itemCount() const { return count_; }

  /// Returns the committed choice, or kNoSelection for an empty table.
  size_t selectedIndex() const { return selected_index_; }

  /// Returns a borrowed view of the selected string, or an empty view.
  roo::string_view selectedText() const;

  /// Silently selects a valid index and closes any active menu.
  /// Returns false without changing state when @p index is invalid.
  bool setSelectedIndex(size_t index);

  /// Returns the Material 3 size tier used for trigger geometry.
  ButtonSize size() const { return static_cast<ButtonSize>(size_); }
  /// Changes trigger size and closes an active menu.
  void setSize(ButtonSize size);

  /// Returns the resting corner family.
  ButtonShape shape() const { return static_cast<ButtonShape>(shape_); }
  /// Changes the resting corners and closes an active menu.
  void setShape(ButtonShape shape);

  /// Returns the trigger's Material 3 color variant.
  ButtonVariant variant() const { return static_cast<ButtonVariant>(variant_); }
  /// Changes trigger colors and closes an active menu.
  void setVariant(ButtonVariant variant);

  /// Returns the density policy shared by trigger and menu.
  DensityOverride densityOverride() const { return density_; }
  /// Changes both trigger and menu density; a default policy inherits theme.
  void setDensityOverride(DensityOverride density);

  /// Presents an anchored menu or returns a specific admission failure.
  MenuShowResult showMenu();

  /// Silently cancels an open menu or pending selection completion.
  void dismissMenu();

  /// Reports whether a menu is still presented, excluding pending cleanup.
  bool isMenuOpen() const;

  /// Uses the shared button padding at the resolved density.
  Padding getPadding() const override;
  /// Preserves the standard button's outer spacing.
  Margins getMargins() const override { return Margins(MarginSize::kRegular); }
  /// An empty catalog is never an interactive target.
  bool isClickable() const override { return count_ > 0; }
  /// Resolves the button's semantic container role.
  ColorToken containerRole() const override;
  /// Resolves the button's current surface fill.
  Color background() const override;
  /// Resolves the outline for the outlined variant.
  Color getOutlineColor() const override;
  /// Resolves the resting or pressed shape.
  BorderStyle getBorderStyle() const override;
  /// Returns variant elevation for the current state.
  uint8_t getElevation() const override;
  /// Avoids applying a second disabled palette.
  bool useAutomaticDisabledStyle() const override { return false; }
  /// Returns content dimensions before button padding.
  Dimensions getSuggestedMinimumDimensions() const override;
  /// Rebuilds longest-label metrics when shared typography or scale changes.
  void requestLayoutDescending() override;
  /// Opens the menu after a completed click.
  void onClicked() override;
  /// Opens on Down when the trigger owns keyboard focus.
  bool onKeyEvent(const KeyEvent& event) override;
  /// Cancels a menu when the trigger becomes disabled.
  void notifyStateChanged(uint16_t state_diff) override;
  /// Cancels a menu after hidden or detached presentation changes.
  void onPresentationChanged(const PresentationChange& change) override;
  /// Paints left-aligned text and a right-aligned chevron.
  void paint(PaintContext& ctx) const override;

 protected:
  /// Resolves natural dimensions against parent constraints.
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;

 private:
  class Session;

  static bool validItems(const char* const* items, size_t count);
  static Dimensions measureItems(const char* const* items, size_t count);
  void completeSession();
  int8_t resolvedDensityLevel() const;
  Dimensions iconSlot() const;

  const char* const* items_ = nullptr;
  size_t count_ = 0;
  size_t selected_index_ = kNoSelection;
  std::unique_ptr<Session> session_;
  Dimensions text_metrics_;
  DensityOverride density_;
  uint8_t variant_ : 3;
  uint8_t size_ : 3;
  uint8_t shape_ : 1;
};

}  // namespace roo_windows::material3
