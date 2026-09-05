#pragma once

#include <vector>

#include "roo_display.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/core/presentation_pin.h"
#include "roo_windows/core/transient_presentation.h"
#include "roo_windows/core/transient_surface_host.h"
#include "roo_windows/widgets/scrim.h"

namespace roo_windows {

class Application;
class ApplicationTextInput;
class DisplayWindow;

/// Root container and shared UI services owner for an Application.
class MainWindow : public Container {
 public:
  MainWindow(Application& app, const roo_display::Box& bounds);

  ~MainWindow() override;

  /// Drives the shared click-animation forward one tick and invalidates
  /// regions that changed.
  void refreshClickAnimation();

  /// Returns true while a deadline-interrupted logical paint is awaiting
  /// continuation with its completed foreground state preserved.
  bool hasPaintContinuation() const { return paint_continuation_; }

  /// Drops retained interrupted-paint state before the window tears down.
  void cancelPaintContinuation();

  /// Applies any pending layout requests in the widget tree.
  void updateLayout();

  MainWindow* getMainWindow() override { return this; }
  const MainWindow* getMainWindow() const override { return this; }

  /// Performs a single paint pass onto the supplied display surface, bounded
  /// by `deadline`. Returns false when the deadline interrupted painting.
  bool paintWindow(const roo_display::Surface& s, roo_time::Uptime deadline);

  Application& app() const;
  const Theme& theme() const override;

  /// Adds a regular task layer child. Tasks render behind popups and
  /// dialogs.
  void addTask(WidgetRef child, const Rect& rect);

  /// Adds a popup layer child. Popups render above tasks but below dialogs.
  void addPopup(WidgetRef child, const Rect& rect);

  /// Detaches a regular task layer child before its owner is destroyed.
  void removeTask(Widget& child);

  /// Detaches a popup layer child before its owner is destroyed.
  void removePopup(Widget& child);

  /// Returns the shared click-animation controller for this window.
  ClickAnimation& click_animation() { return click_animation_; }

  /// Returns the shared click-animation controller for this window.
  const ClickAnimation& click_animation() const { return click_animation_; }

  /// Returns the single slot for root interactive transient presentations.
  TransientPresentationSlot& transient_presentation_slot() {
    return transient_presentation_slot_;
  }

  /// Returns the single slot for root interactive transient presentations.
  const TransientPresentationSlot& transient_presentation_slot() const {
    return transient_presentation_slot_;
  }

 protected:
  int getChildrenCount() const override {
    return static_cast<int>(tasks_.size()) + static_cast<int>(popups_.size()) +
           (host_layer_.parent() != nullptr ? 1 : 0);
  }

  const Widget& getChild(int idx) const override {
    int task_count = static_cast<int>(tasks_.size());
    if (idx < task_count) return *tasks_[idx];
    idx -= task_count;
    int popup_count = static_cast<int>(popups_.size());
    if (idx < popup_count) return *popups_[idx];
    idx -= popup_count;
    return host_layer_;
  }

  Widget& getChild(int idx) override {
    int task_count = static_cast<int>(tasks_.size());
    if (idx < task_count) return *tasks_[idx];
    idx -= task_count;
    int popup_count = static_cast<int>(popups_.size());
    if (idx < popup_count) return *popups_[idx];
    idx -= popup_count;
    return host_layer_;
  }

  void propagateDirty(const Widget* child, const Rect& rect) override;

  void childInvalidatedRegion(const Widget* child, Rect rect) override;

  /// Paints root children with their eligible pins immediately before each
  /// child, preserving the generic container traversal for ordinary trees.
  void paintChildren(PaintContext& ctx) override;

 private:
  friend class ApplicationTextInput;
  friend class DisplayWindow;
  friend class Container;
  friend class Widget;
  friend class Task;
  friend class internal::TransientSurfaceHost;
  friend internal::TransientSurfaceHost& internal::GetTransientSurfaceHost(
      Task& interaction_owner);
  friend bool internal::CaptureTransientSourceGeometry(
      Task& owner, const Widget& source,
      internal::TransientSourceGeometry& output);

  /// Permanently closes transient admission and finishes any active surface.
  void beginShutdown();

  /// Detaches root-owned content while display-local services still exist.
  void prepareForDestruction();

  void attachTransientHostLayer();
  void detachTransientHostLayer();

  /// Cancels Enter/Space activation armed in any task covered by the host.
  void cancelTaskKeyActivationForDisplayCoverage();

  /// Removes gesture-detector references before `subtree` loses parent links.
  void gestureTargetSubtreeDetaching(Widget& subtree);

  /// Delivers a completed outside activation after touch dispatch unwinds.
  void flushPendingOutsideInteraction();

  PresentationPinShowResult showPresentationPin(
      Widget& anchor, std::unique_ptr<PresentationPin> pin);

  bool hasPresentationPin(const Widget& anchor) const;
  void setPresentationPinDirty(const Widget& anchor);
  void hidePresentationPin(const Widget& anchor);

  /// Adopts one hosted pin and returns its stable non-owning identity.
  PresentationPinShowResult showHostedPresentationPin(
      Widget& owner_root, std::unique_ptr<PresentationPin> pin,
      PresentationPin*& active_pin);

  /// Invalidates a hosted pin previously returned by the show helper.
  void setHostedPresentationPinDirty(PresentationPin& active_pin);

  /// Unlinks a hosted pin and clears its caller-owned non-owning identity.
  void hideHostedPresentationPin(PresentationPin*& active_pin);

  /// Schedules a full root-tree repaint of the affected pin region.
  void invalidatePresentationRegion(const Rect& rect);

  /// Removes pins before an anchor subtree loses its parent chain.
  void presentationAnchorSubtreeDetaching(Widget& subtree);

  /// Detects visibility and geometry changes before choosing the redraw clip.
  void preparePresentationPinsForPaint();

  /// Settles pins scoped to `root` before that root paints.
  void paintPinsBeforeScopeRoot(Widget& root, PaintContext& ctx);

  /// Commits conservative pin envelopes after a complete paint.
  void commitPresentationPinBounds();

  void addToLayer(std::vector<Widget*>& layer, WidgetRef child,
                  const Rect& rect);

  void removeLastFromLayer(std::vector<Widget*>& layer);

  void removeFromLayer(std::vector<Widget*>& layer, Widget& child);

  Application& app_;

  // Regular application tasks rendered behind popup overlays such as the
  // keyboard.
  std::vector<Widget*> tasks_;

  // Popup tasks rendered above regular tasks but below modal dialogs.
  std::vector<Widget*> popups_;

  ClickAnimation click_animation_;

  // Stored as instance variable, to avoid vector reallocation on each paint.
  internal::ClipperState clipper_state_;

  // Maintains the area that encapsulates all content that needs to be
  // (re)drawn.
  Rect redraw_bounds_;

  bool paint_continuation_ = false;
  Rect continuation_invalid_bounds_ = Rect(0, 0, -1, -1);

  bool initialized_ = false;

  Scrim scrim_;

  internal::TransientHostLayer host_layer_;
  internal::TransientSurfaceHost transient_surface_host_;

  std::unique_ptr<PresentationPin> active_pins_;

  // Kept last so it clears presenter reachability before other window
  // resources are destroyed.
  TransientPresentationSlot transient_presentation_slot_;
};

}  // namespace roo_windows
