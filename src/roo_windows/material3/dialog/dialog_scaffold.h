#pragma once

#include <stdint.h>

#include <string>

#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/focus_manager.h"
#include "roo_windows/core/layout_direction.h"
#include "roo_windows/core/transient_surface_host.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/dialog/dialog_types.h"
#include "roo_windows/widgets/divider.h"
#include "roo_windows/widgets/text_block.h"

namespace roo_windows::material3::internal {

/// Selects the shape and chrome layout used by a dialog scaffold.
enum class DialogScaffoldVariant : uint8_t { kBasic, kFullScreen };

/// Identifies one derived-owned chrome child attached to the scaffold.
enum class DialogChromeSlot : uint8_t { kPrimary, kSecondary };

class DialogActionDelegate {
 public:
  /// Destroys the lifetime-coupled presenter interface.
  virtual ~DialogActionDelegate() = default;

  /// Routes an enabled fixed action to its owning dialog presenter.
  virtual void invokeDialogAction(uint8_t id, DialogActionRole role) = 0;
};

/// Shared pinned-chrome layout for transient and navigation-backed dialogs.
///
/// The body remains attached when a presentation is dismissed, preserving
/// caller state across reopen. Derived destructors must call
/// `prepareForDerivedDestruction()` before inline body or chrome storage dies.
class DialogScaffold : public Container {
 public:
  ~DialogScaffold() override;

  /// Publishes the Material 3 surface-container-high semantic role.
  ColorToken containerRole() const override;

  /// Resolves the scaffold fill from the active Material 3 theme.
  Color background() const override;

  /// Returns a 28dp basic shape or a rectangular full-screen shape.
  BorderStyle getBorderStyle() const override;

  /// Paints the optional basic-dialog icon above the owned title.
  void paint(PaintContext& ctx) const override;

  /// Keeps the scaffold itself out of the focus traversal.
  bool isFocusable() const override { return false; }

  /// Opens with an empty focus state; keyboard traversal enters on Tab.
  Widget* preferredFocusChild() override { return nullptr; }

 protected:
  DialogScaffold(ApplicationContext& context, WidgetRef body,
                 DialogScaffoldVariant variant);

  /// Attaches a lifetime-coupled, derived-owned chrome widget.
  void attachDerivedChrome(DialogChromeSlot slot, Widget& chrome);

  /// Detaches an attached derived-owned chrome widget.
  void detachDerivedChrome(DialogChromeSlot slot);

  /// Replaces the persistent body, honoring `WidgetRef` ownership.
  void setDialogBody(WidgetRef body);

  /// Replaces the owned title/headline string.
  void setDialogTitle(std::string title);

  /// Sets or clears the borrowed basic-dialog icon.
  void setDialogIcon(const MonoIcon* icon);

  /// Returns whether the scaffold has a borrowed icon configured.
  bool hasDialogIcon() const { return icon_ != nullptr; }

  /// Returns the currently attached body, or null.
  Widget* dialogBody() { return body_; }
  const Widget* dialogBody() const { return body_; }

  /// Returns the owning title widget for variant-specific configuration.
  TextBlock& dialogTitle() { return title_; }

  /// Sets the explicit logical layout direction.
  void setDialogLayoutDirection(LayoutDirection direction);

  /// Returns the explicit logical layout direction.
  LayoutDirection dialogLayoutDirection() const { return direction_; }

  /// Detaches persistent body and derived chrome before their storage dies.
  virtual void prepareForDerivedDestruction();

  /// Hook for presenters that retain focus across structural detachment.
  virtual void clearDialogRememberedFocus() {}

 private:
  class DialogBodyScroller final : public SimpleScrollablePanel {
   public:
    DialogBodyScroller(ApplicationContext& context, DialogScaffold& owner)
        : SimpleScrollablePanel(context), owner_(owner) {}

    /// Leaves viewport gaps to the scaffold-owned dialog surface.
    void paint(PaintContext& ctx) const override { (void)ctx; }

    /// Inherits the scaffold's effective background without owning a fill.
    Color background() const override {
      return roo_display::color::Transparent;
    }

    /// Reports that unpainted viewport pixels remain transparent.
    bool fullyCoversBoundsWithOpaqueColors() const override { return false; }

    /// Updates conditional divider visibility after scrolling.
    void onScrollPositionChanged() override { owner_.updateDividers(); }

   protected:
    /// Claims no direct surface pixels; descendants publish their own ink.
    Rect getDirectPaintExclusionBounds() const override { return Rect(); }

   private:
    DialogScaffold& owner_;
  };

  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

  void clearDialogBody();
  void updateDividers();

  DialogScaffoldVariant variant_;
  LayoutDirection direction_ = LayoutDirection::kLeftToRight;
  TextBlock title_;
  HorizontalDivider top_divider_;
  DialogBodyScroller body_scroller_;
  HorizontalDivider bottom_divider_;
  Widget* body_ = nullptr;
  Widget* chrome_[2] = {nullptr, nullptr};
  const MonoIcon* icon_ = nullptr;
  int16_t content_inset_ = 0;
  int16_t icon_height_ = 0;
  YDim title_height_ = 0;
  XDim chrome_width_[2] = {0, 0};
  YDim chrome_height_[2] = {0, 0};
};

/// Basic-dialog layout with single-slot transient presentation.
class DialogScaffoldBase : public DialogScaffold {
 public:
  ~DialogScaffoldBase() override;

 protected:
  DialogScaffoldBase(ApplicationContext& context, WidgetRef body,
                     DialogScaffoldVariant variant);

  /// Attempts to host this detached root at the supplied window bounds.
  DialogShowResult showDialogSurface(Task& interaction_owner,
                                     const Rect& bounds_in_window,
                                     TransientBarrierPaint barrier);

  /// Measures and centers a basic dialog inside the interaction owner's
  /// window during guarded host preparation.
  DialogShowResult showBasicDialogSurface(Task& interaction_owner);

  /// Returns whether this scaffold currently occupies the shared host.
  bool isDialogShowing() const { return registration_.isActive(); }

  /// Finishes an active scaffold with a shared presentation reason.
  void finishDialog(PresentationFinishReason reason);

  /// Disables input, cancels registration, and detaches body and chrome.
  ///
  /// Call this at the beginning of every derived destructor whose inline
  /// members are attached to the scaffold. It is safe to call repeatedly.
  void prepareForDerivedDestruction() override;

  /// Forgets an inactive descendant address before chrome replacement.
  void clearDialogRememberedFocus() override {
    focus_scope_.clearRememberedFocus();
  }

  /// Handles an eligible Back or Escape request while still attached.
  virtual BackResult onDialogBackRequested(BackSource source);

  /// Receives completion after the host has detached the root and gone idle.
  virtual void onDialogPresentationFinished(PresentationFinishReason reason) {
    (void)reason;
  }

 private:
  class Registration final : public TransientPresentationRegistration {
   public:
    explicit Registration(DialogScaffoldBase& owner) : owner_(owner) {}

    void cancelPresentation() { cancel(); }
    void disablePresentationInput() { disableHostedInput(); }

   protected:
    void detachPresentation(PresentationFinishReason reason) override;
    void onFinished(PresentationFinishReason reason) override;
    BackResult onBackRequested(BackSource source) override;

   private:
    DialogScaffoldBase& owner_;
  };

  FocusScope focus_scope_;
  // Cancel hosting before layout storage is destroyed.
  Registration registration_;
};

/// Fixed-capacity one-or-two action strip shared by basic dialogs.
class DialogActionStrip final : public Container {
 public:
  DialogActionStrip(ApplicationContext& context, DialogActionDelegate& owner,
                    const DialogActionSpec* actions, uint8_t action_count);
  ~DialogActionStrip() override;

  /// Enables or disables a confirming action by ID.
  ///
  /// Unknown IDs and attempts to disable acknowledgement or dismissal actions
  /// are programming errors and fail a `CHECK`.
  void setActionEnabled(uint8_t action_id, bool enabled);

  /// Selects logical horizontal ordering; vertical ordering is unchanged.
  void setLayoutDirection(LayoutDirection direction);

  /// Returns the copied descriptor for testing and presenter routing.
  const DialogActionSpec& action(uint8_t index) const;

  /// Returns one inline action button.
  Widget& actionButton(uint8_t index);

  /// Returns one inline action button.
  const Widget& actionButton(uint8_t index) const;

  /// Returns the number of active fixed slots.
  uint8_t actionCount() const { return action_count_; }

  /// Returns whether the last measurement selected vertical stacking.
  bool isStacked() const { return stacked_; }

  /// Emits no pixels; gaps reveal the owning dialog surface.
  void paint(PaintContext& ctx) const override { (void)ctx; }

  /// Reports a transparent surface for inherited-background resolution.
  Color background() const override { return roo_display::color::Transparent; }

  /// Reports that transparent gaps do not cover the parent surface.
  bool fullyCoversBoundsWithOpaqueColors() const override { return false; }

 protected:
  /// Claims no direct surface pixels; buttons publish their own exclusions.
  Rect getDirectPaintExclusionBounds() const override { return Rect(); }

  int getChildrenCount() const override;
  const Widget& getChild(int idx) const override;
  Widget& getChild(int idx) override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  class ActionButton final : public Button {
   public:
    ActionButton(ApplicationContext& context, DialogActionStrip& strip,
                 uint8_t slot);

    /// Routes activation through the fixed strip without callback storage.
    ClickActivationPolicy getClickActivationPolicy() const override {
      return ClickActivationPolicy::kAfterForcedFinalFrame;
    }

    void onClicked() override;

   private:
    DialogActionStrip& strip_;
    uint8_t slot_;
  };

  void invoke(uint8_t slot);

  DialogActionDelegate& owner_;
  DialogActionSpec actions_[2] = {};
  ActionButton buttons_[2];
  Dimensions button_dimensions_[2];
  uint8_t action_count_ = 0;
  LayoutDirection direction_ = LayoutDirection::kLeftToRight;
  bool stacked_ = false;
};

}  // namespace roo_windows::material3::internal
