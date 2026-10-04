#pragma once

#include <stdint.h>

#include "roo_backport/string_view.h"
#include "roo_windows/core/border_style.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/text_style.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/core/widget_ref.h"
#include "roo_windows/material3/app_bar/app_bar_scroll_behavior.h"
#include "roo_windows/material3/app_bar/app_bar_tokens.h"
#include "roo_windows/material3/container.h"
#include "roo_windows/widgets/icon.h"

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
class AppBarText : public Widget {
 public:
  explicit AppBarText(ApplicationContext& context) : Widget(context) {}

  void setText(roo::string_view text);
  roo::string_view text() const { return text_; }

  /// Chooses the Material typography used by this bounded presentation child.
  void setTextStyle(const TextStyle& text_style);

  void setAlignment(roo_display::Alignment alignment);

  void setUseOnSurfaceVariant(bool value) { use_on_surface_variant_ = value; }

  /// Reports painted ink independently of the logical layout rectangle.
  Insets getInkInsets() const override;

  Dimensions getSuggestedMinimumDimensions() const override;
  void paint(PaintContext& ctx) const override;

 protected:
  virtual roo_display::Font::Options fontOptions(const TextStyle& style) const {
    return style.fontOptions();
  }
  virtual roo_display::Color textColor(roo_display::Color background) const;

 private:
  roo::string_view text_;
  const TextStyle* text_style_ = nullptr;
  roo_display::Alignment alignment_ = roo_display::kLeft | roo_display::kMiddle;
  bool use_on_surface_variant_ = false;
};

// Titles share advance-width and ascent alignment with other app-bar text.
class AppBarTitle final : public AppBarText {
 public:
  using AppBarText::AppBarText;
  roo_display::Font::Options fontOptions(const TextStyle& style) const override;
};

// Supplemental text fades without storing an opacity on every text child.
class AppBarSubtitle final : public AppBarText {
 public:
  using AppBarText::AppBarText;

 protected:
  roo_display::Color textColor(roo_display::Color background) const override;
};

}  // namespace internal

/// Material 3 title-based top-app-bar family.
class AppBar : public Material3Container {
 public:
  /// Creates an app bar with the selected title-based variant.
  explicit AppBar(ApplicationContext& context,
                  AppBarVariant variant = AppBarVariant::kSmall);

  /// Detaches hosted child slots before their references are released.
  ~AppBar() override;

  /// Allows caller-provided leading and trailing slots to be unclipped.
  bool mayHaveUnclippedChildren() const override { return true; }

  /// Changes the title-based app-bar variant and requests layout.
  void setVariant(AppBarVariant variant);

  /// Returns the configured title-based app-bar variant.
  AppBarVariant variant() const { return variant_; }

  /// Changes title alignment and requests layout.
  void setTitleAlignment(AppBarTitleAlignment alignment);

  /// Returns the configured title alignment.
  AppBarTitleAlignment titleAlignment() const { return title_alignment_; }

  /// Sets the manual surface preference, used while no behavior is connected.
  void setSurfaceState(AppBarSurfaceState state);

  /// Returns the effective surface state, including a connected behavior.
  AppBarSurfaceState surfaceState() const;

  /// Connects without replacing the panel's application callback. Failed
  /// registration preserves the previous binding; endpoints share a context.
  ScrollConnectionStatus setScrollBehavior(SimpleScrollablePanel& panel,
                                           AppBarScrollBehavior behavior);
  /// Restores expanded geometry and the manually selected surface state.
  ScrollConnectionStatus clearScrollBehavior();
  /// Returns whether this bar has an active connection.
  bool hasScrollBehavior() const;

  /// Applies scroll settlement and suspends motion when presentation ends.
  void onAnimationFrame(AnimationTag tag,
                        const AnimationSample& sample) override;
  void onPresentationChanged(const PresentationChange& change) override;

  /// Returns the surface role for the current flat or scrolled state.
  ColorToken containerRole() const override;

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
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;

  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  friend class internal::AppBarTitle;
  const TextStyle& titleTextStyle() const;
  const TextStyle& expandedTitleTextStyle() const;
  const internal::AppBarVariantTokens& tokens() const;
  int16_t containerHeightDp() const;
  void replaceSlot(Widget*& slot, WidgetRef widget);
  internal::AppBarTitle title_widget_;
  internal::AppBarSubtitle subtitle_widget_;
  Widget* leading_;
  Widget* trailing_[2];
  AppBarVariant variant_;
  AppBarTitleAlignment title_alignment_;
  AppBarSurfaceState surface_state_;
};

/// Phase-1 declaration of the standalone Material 3 search-entry surface.
class SearchBar : public Material3Container {
 public:
  /// Creates a standalone Material 3 search entry surface.
  explicit SearchBar(ApplicationContext& context);

  /// Detaches hosted child slots before their references are released.
  ~SearchBar() override;

  /// Allows caller-provided leading and trailing slots to be unclipped.
  bool mayHaveUnclippedChildren() const override { return true; }

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

  /// Returns the standalone contained-search surface role.
  ColorToken containerRole() const override;
  BorderStyle getBorderStyle() const override;

  bool fillTouchTargetPath(XDim x, YDim y, std::vector<Widget*>& path) override;
  bool fillSloppyTouchTargetPath(XDim x, YDim y,
                                 std::vector<Widget*>& path) override;

 protected:
  /// Selects geometry for standalone or embedded search-entry variants.
  virtual const internal::SearchEntryTokens& entryTokens() const;

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
class SearchAppBar : public Material3Container {
 public:
  /// Creates a full-width Material 3 search app bar.
  explicit SearchAppBar(ApplicationContext& context);

  /// Detaches hosted child slots before their references are released.
  ~SearchAppBar() override;

  /// Allows caller-provided outer and embedded slots to be unclipped.
  bool mayHaveUnclippedChildren() const override { return true; }

  /// Changes the outer app-bar surface state and repaints it.
  void setSurfaceState(AppBarSurfaceState state);

  /// Returns the configured outer app-bar surface state.
  AppBarSurfaceState surfaceState() const;

  /// Connects without replacing the panel's application callback. Failed
  /// registration preserves the previous binding; endpoints share a context.
  ScrollConnectionStatus setScrollBehavior(SimpleScrollablePanel& panel,
                                           AppBarScrollBehavior behavior);
  /// Restores expanded geometry and the manually selected surface state.
  ScrollConnectionStatus clearScrollBehavior();
  /// Returns whether this bar has an active connection.
  bool hasScrollBehavior() const;

  /// Applies scroll settlement and suspends motion when presentation ends.
  void onAnimationFrame(AnimationTag tag,
                        const AnimationSample& sample) override;
  void onPresentationChanged(const PresentationChange& change) override;

  /// Returns the surface role for the current flat or scrolled state.
  ColorToken containerRole() const override;

  /// Replaces the non-owning text displayed by the embedded search entry.
  void setDisplayText(roo::string_view text);

  /// Returns the non-owning text displayed by the embedded search entry.
  roo::string_view displayText() const { return search_entry_.displayText(); }

  /// Replaces or clears the optional outer leading child slot.
  void setLeading(WidgetRef widget);

  /// Replaces or clears one of the two embedded search trailing child slots.
  void setInnerTrailing(uint8_t index, WidgetRef widget);

  /// Replaces or clears one of the two outer trailing child slots.
  void setTrailing(uint8_t index, WidgetRef widget);

  /// Routes the embedded entry and outer affordances independently; the
  /// non-interactive outer surface never becomes a fallback touch target.
  bool fillTouchTargetPath(XDim x, YDim y, std::vector<Widget*>& path) override;
  bool fillSloppyTouchTargetPath(XDim x, YDim y,
                                 std::vector<Widget*>& path) override;

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;

 private:
  class EmbeddedSearchBar final : public SearchBar {
   public:
    explicit EmbeddedSearchBar(ApplicationContext& context)
        : SearchBar(context), surface_state_(AppBarSurfaceState::kFlat) {}

    void setSurfaceState(AppBarSurfaceState state);
    ::roo_windows::material3::ColorToken containerRole() const override;

   protected:
    const internal::SearchEntryTokens& entryTokens() const override;

   private:
    AppBarSurfaceState surface_state_;
  };

  void replaceSlot(Widget*& slot, WidgetRef widget);
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

  EmbeddedSearchBar search_entry_;
  Widget* leading_;
  Widget* trailing_[2];
  AppBarSurfaceState surface_state_;
};

}  // namespace roo_windows::material3
