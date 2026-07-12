#pragma once

#include <stdint.h>

#include "roo_backport/string_view.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/widget_ref.h"

namespace roo_windows::material3 {

/// Selects the title-based top-app-bar geometry and typography treatment.
enum class AppBarVariant : uint8_t { kSmall, kMediumFlexible, kLargeFlexible };

/// Selects whether the title block begins at the content edge or is centered.
enum class AppBarTitleAlignment : uint8_t { kLeading, kCentered };

/// Selects the flat or content-separated app-bar surface treatment.
enum class AppBarSurfaceState : uint8_t { kFlat, kScrolled };

/// Phase-1 declaration of the Material 3 title-based top-app-bar family.
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
  roo::string_view title() const { return title_; }

  /// Replaces the non-owning subtitle text view.
  void setSubtitle(roo::string_view subtitle);

  /// Returns the non-owning subtitle text view.
  roo::string_view subtitle() const { return subtitle_; }

  /// Replaces or clears the optional leading child slot.
  void setLeading(WidgetRef widget);

  /// Replaces or clears one of the two trailing child slots.
  void setTrailing(uint8_t index, WidgetRef widget);

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;

 private:
  void replaceSlot(Widget*& slot, WidgetRef widget);
  roo::string_view title_;
  roo::string_view subtitle_;
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

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;

 private:
  void replaceSlot(Widget*& slot, WidgetRef widget);
  roo::string_view display_text_;
  Widget* leading_;
  Widget* trailing_[2];
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

 protected:
  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;

 private:
  void replaceSlot(Widget*& slot, WidgetRef widget);
  roo::string_view display_text_;
  Widget* leading_;
  Widget* inner_trailing_[2];
  Widget* trailing_[2];
  AppBarSurfaceState surface_state_;
};

}  // namespace roo_windows::material3
