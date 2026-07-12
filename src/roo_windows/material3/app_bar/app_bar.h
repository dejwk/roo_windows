#pragma once

#include <stdint.h>

#include "roo_backport/string_view.h"
#include "roo_windows/core/border_style.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/core/widget_ref.h"
#include "roo_windows/widgets/icon.h"
#include "roo_windows/material3/app_bar/app_bar_tokens.h"

namespace roo_windows::material3 {

/// Selects the title-based top-app-bar geometry and typography treatment.
enum class AppBarVariant : uint8_t { kSmall, kMediumFlexible, kLargeFlexible };

/// Selects whether the title block begins at the content edge or is centered.
enum class AppBarTitleAlignment : uint8_t { kLeading, kCentered };

/// Selects the flat or content-separated app-bar surface treatment.
enum class AppBarSurfaceState : uint8_t { kFlat, kScrolled };

namespace internal {

// The entry-only search surface needs a bounded, non-interactive text child.
// It deliberately keeps only a view: editable query state belongs to the
// later focused-search work, not to this shell component.
class AppBarText final : public Widget {
 public:
  explicit AppBarText(ApplicationContext& context) : Widget(context) {}

  void setText(roo::string_view text) { text_ = text; }
  roo::string_view text() const { return text_; }

  /// Chooses the Material typography used by this bounded presentation child.
  void setFont(const roo_display::Font& font) { font_ = &font; }

  void setAlignment(roo_display::Alignment alignment) {
    alignment_ = alignment;
  }

  void setUseOnSurfaceVariant(bool value) { use_on_surface_variant_ = value; }

  Dimensions getSuggestedMinimumDimensions() const override;
  void paint(PaintContext& ctx) const override;

 private:
  roo::string_view text_;
  const roo_display::Font* font_ = nullptr;
  roo_display::Alignment alignment_ =
      roo_display::kLeft | roo_display::kMiddle;
  bool use_on_surface_variant_ = false;
};

}  // namespace internal

/// Material 3 title-based top-app-bar family.
class AppBar : public Container {
 public:
  /// Creates an app bar with the selected title-based variant.
  explicit AppBar(ApplicationContext& context,
                  AppBarVariant variant = AppBarVariant::kSmall);

  /// Detaches hosted child slots before their references are released.
  ~AppBar() override;

  /// Changes the title-based app-bar variant and requests layout.
  void setVariant(AppBarVariant variant);

  /// Returns the configured title-based app-bar variant.
  AppBarVariant variant() const { return variant_; }

  /// Changes title alignment and requests layout.
  void setTitleAlignment(AppBarTitleAlignment alignment);

  /// Returns the configured title alignment.
  AppBarTitleAlignment titleAlignment() const { return title_alignment_; }

  /// Changes the surface state and repaints the app-bar surface.
  void setSurfaceState(AppBarSurfaceState state);

  /// Returns the configured surface state.
  AppBarSurfaceState surfaceState() const { return surface_state_; }

  /// Replaces the non-owning title text view.
  void setTitle(roo::string_view title);

  /// Returns the non-owning title text view.
  roo::string_view title() const { return title_widget_.text(); }

  /// Replaces the non-owning subtitle text view.
  void setSubtitle(roo::string_view subtitle);

  /// Returns the non-owning subtitle text view.
  roo::string_view subtitle() const { return subtitle_widget_.text(); }

  /// Replaces or clears the optional leading child slot.
  void setLeading(WidgetRef widget);

  /// Replaces or clears one of the two trailing child slots.
  void setTrailing(uint8_t index, WidgetRef widget);

 protected:
  Color background() const override;
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;

  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  const roo_display::Font& titleFont() const;
  const internal::AppBarVariantTokens& tokens() const;
  int16_t containerHeightDp() const;
  void replaceSlot(Widget*& slot, WidgetRef widget);
  internal::AppBarText title_widget_;
  internal::AppBarText subtitle_widget_;
  Widget* leading_;
  Widget* trailing_[2];
  AppBarVariant variant_;
  AppBarTitleAlignment title_alignment_;
  AppBarSurfaceState surface_state_;
};

/// Phase-1 declaration of the standalone Material 3 search-entry surface.
class SearchBar : public Container {
 public:
  /// Creates a standalone Material 3 search entry surface.
  explicit SearchBar(ApplicationContext& context);

  /// Detaches hosted child slots before their references are released.
  ~SearchBar() override;

  /// Replaces the non-owning text displayed by the search entry surface.
  void setDisplayText(roo::string_view text);

  /// Returns the non-owning text displayed by the search entry surface.
  roo::string_view displayText() const { return display_text_; }

  /// Replaces or clears the optional leading child slot.
  void setLeading(WidgetRef widget);

  /// Replaces or clears one of the two trailing child slots.
  void setTrailing(uint8_t index, WidgetRef widget);

  /// Search entry surfaces are intrinsically clickable.
  bool isClickable() const override { return true; }

  /// Keeps interaction feedback out of the container surface paint path.
  /// Activation still follows the ordinary click and semantic-action path.
  bool useOverlayOnPress() const override { return false; }

  /// Resolves the standalone contained-search surface token.
  Color background() const override;
  BorderStyle getBorderStyle() const override;

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;

 private:
  void replaceSlot(Widget*& slot, WidgetRef widget);
  roo::string_view display_text_;
  internal::AppBarText display_text_widget_;
  Icon passive_search_icon_;
  Widget* leading_;
  Widget* trailing_[2];

  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;
};

/// Phase-1 declaration of the full-width Material 3 search app bar.
class SearchAppBar : public Container {
 public:
  /// Creates a full-width Material 3 search app bar.
  explicit SearchAppBar(ApplicationContext& context);

  /// Detaches hosted child slots before their references are released.
  ~SearchAppBar() override;

  /// Changes the outer app-bar surface state and repaints it.
  void setSurfaceState(AppBarSurfaceState state);

  /// Returns the configured outer app-bar surface state.
  AppBarSurfaceState surfaceState() const { return surface_state_; }

  /// Replaces the non-owning text displayed by the embedded search entry.
  void setDisplayText(roo::string_view text);

  /// Returns the non-owning text displayed by the embedded search entry.
  roo::string_view displayText() const { return display_text_; }

  /// Replaces or clears the optional outer leading child slot.
  void setLeading(WidgetRef widget);

  /// Replaces or clears one of the two embedded search trailing child slots.
  void setInnerTrailing(uint8_t index, WidgetRef widget);

  /// Replaces or clears one of the two outer trailing child slots.
  void setTrailing(uint8_t index, WidgetRef widget);

  /// Search entry surfaces are intrinsically clickable.
  bool isClickable() const override { return true; }

  /// The outer action strip remains interactive, but only the contained
  /// search-entry lane activates this app bar itself.
  bool fillTouchTargetPath(XDim x, YDim y,
                           std::vector<Widget*>& path) override;
  bool fillSloppyTouchTargetPath(XDim x, YDim y,
                                 std::vector<Widget*>& path) override;

  Color background() const override;
  void paint(PaintContext& ctx) const override;
  bool useOverlayOnPress() const override { return false; }

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;

 private:
  void replaceSlot(Widget*& slot, WidgetRef widget);
  roo::string_view display_text_;
  internal::AppBarText display_text_widget_;
  Icon passive_search_icon_;
  Widget* leading_;
  Widget* inner_trailing_[2];
  Widget* trailing_[2];
  AppBarSurfaceState surface_state_;
  Rect search_entry_bounds_;

  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;
};

}  // namespace roo_windows::material3
