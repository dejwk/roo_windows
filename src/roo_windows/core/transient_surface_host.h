#pragma once

#include <cstdint>

#include "roo_windows/core/container.h"
#include "roo_windows/core/focus_manager.h"
#include "roo_windows/core/rect.h"
#include "roo_windows/core/transient_presentation.h"

namespace roo_windows {

class ApplicationTextInput;
class MainWindow;
class Scrim;
class Task;
class TextFieldEditor;

/// Selects the barrier paint behind a hosted transient surface.
enum class TransientBarrierPaint : uint8_t { kTransparent, kScrim };

/// Selects how a request treats an existing hosted presentation.
enum class TransientAdmissionPolicy : uint8_t {
  kRejectIfBusy,
  kReplaceReplaceable,
};

/// Selects how pointer interaction outside the hosted root is handled.
enum class OutsideInteractionPolicy : uint8_t {
  kAbsorb,
  kDismiss,
  kPresenterHandled,
};

/// Complete immutable policy copied during synchronous host admission.
struct TransientSurfaceSpec {
  /// Creates an explicit policy with no popup- or modal-oriented defaults.
  constexpr TransientSurfaceSpec(TransientBarrierPaint barrier,
                                 TransientAdmissionPolicy admission,
                                 OutsideInteractionPolicy outside,
                                 TransientPresentationPolicy back,
                                 bool replaceable)
      : barrier(barrier),
        admission(admission),
        outside(outside),
        back(back),
        replaceable(replaceable) {}

  /// Selects whether the host inserts its reusable scrim behind the root.
  TransientBarrierPaint barrier;

  /// Selects whether this request may replace an eligible hosted occupant.
  TransientAdmissionPolicy admission;

  /// Records the outside-interaction behavior used by input isolation.
  OutsideInteractionPolicy outside;

  /// Selects which semantic Back requests may dismiss the registration.
  TransientPresentationPolicy back;

  /// Allows a later replace-policy request to replace this presentation.
  bool replaceable;
};

namespace internal {

/// Source geometry copied in receiving-window coordinates.
struct TransientSourceGeometry {
  /// Full source bounds in receiving-window coordinates.
  Rect bounds_in_window;
};

/// Synchronously copies geometry from a source physically owned by `owner`.
///
/// The source must be effectively visible, non-empty, and attached below the
/// owner's exact top-level task panel without crossing a transient host layer.
/// Failure leaves `output` unchanged.
bool CaptureTransientSourceGeometry(Task& owner, const Widget& source,
                                    TransientSourceGeometry& output);

class TransientSurfaceHost;

/// Adapts a detached root whose final bounds require session preparation.
///
/// The host retains no adapter pointer after the synchronous show operation.
class TransientSurfacePreparation {
 public:
  /// Destroys the non-retained preparation adapter.
  virtual ~TransientSurfacePreparation() = default;

 private:
  friend class TransientSurfaceHost;

  /// Creates session resources and resolves final window-coordinate bounds.
  virtual bool createAndResolveBounds(Rect& root_bounds_in_window) = 0;

  /// Balances creation when the prepared presentation does not commit.
  virtual void deleteAfterFailedAdmission() = 0;
};

/// Reusable full-window structural layer for one borrowed transient root.
class TransientHostLayer : public Container {
 public:
  /// Creates the reusable, initially detached layer for one window.
  explicit TransientHostLayer(ApplicationContext& context)
      : Container(context) {}

  /// Resolves descendants through the explicitly supplied interaction owner.
  Task* getTask() override { return owner_; }

  /// @copydoc getTask()
  const Task* getTask() const override { return owner_; }

  /// Keeps the composite layer itself visually transparent.
  Color background() const override { return roo_display::color::Transparent; }

  /// Reports that lower content remains visible through this layer.
  bool fullyCoversBoundsWithOpaqueColors() const override { return false; }

  /// Tests the borrowed root first and otherwise retains a host-only barrier
  /// path so lower MainWindow children are not considered.
  bool fillTouchTargetPath(XDim x, YDim y, std::vector<Widget*>& path) override;

  /// Returns whether the current host-only hit is eligible for a tap role.
  bool supportsTap() const override;

  /// Defers one completed outside activation until terminal dispatch unwinds.
  void onSingleTapUp(XDim x, YDim y) override;

 protected:
  /// Returns the optional scrim followed by the borrowed root.
  int getChildrenCount() const override;

  /// Returns the indexed child in paint order.
  const Widget& getChild(int index) const override;

  /// @copydoc getChild(int) const
  Widget& getChild(int index) override;

  /// Publishes no settled rectangle because this layer paints no pixels.
  Rect getDirectPaintExclusionBounds() const override { return Rect(); }

  /// Leaves invalidated background pixels for lower window layers to paint.
  void paint(PaintContext& ctx) const override { (void)ctx; }

 private:
  friend class TransientSurfaceHost;

  void attachSurface(Task& owner, Widget& root, Scrim* scrim,
                     const Rect& root_bounds);
  void detachSurface();
  void enableInput();
  void disableInput();
  bool isInputEnabled() const;
  bool takePendingOutsideActivation();

  bool isTransientHostLayer() const override { return true; }

  Task* owner_ = nullptr;
  Widget* root_ = nullptr;
  Scrim* scrim_ = nullptr;
  uint8_t input_state_ = 0;
};

/// Coordinates one structurally hosted transient surface for a window.
class TransientSurfaceHost {
 public:
  /// Creates the coordinator embedded in `window`.
  explicit TransientSurfaceHost(MainWindow& window) : window_(window) {}

  /// Admits and attaches a presenter-owned root synchronously.
  ///
  /// The owner must be presentation-available in this window, `root` must be
  /// detached and use the owner's application context, `scope` must be
  /// admissible, and the root bounds must intersect the non-empty window.
  PresentationStartResult show(TransientPresentationRegistration& registration,
                               Task& owner, Widget& root,
                               const Rect& root_bounds_in_window,
                               FocusScope& scope,
                               const TransientSurfaceSpec& spec);

  /// Prepares, measures, and admits a root in one guarded transaction.
  ///
  /// Initial owner, registration, root, scope, and policy validation precedes
  /// preparation. Once preparation starts, every failure invokes balanced
  /// deletion exactly once.
  PresentationStartResult showPrepared(
      TransientPresentationRegistration& registration, Task& owner,
      Widget& root, FocusScope& scope, const TransientSurfaceSpec& spec,
      TransientSurfacePreparation& preparation);

 private:
  friend class ::roo_windows::MainWindow;
  friend class ::roo_windows::ApplicationTextInput;
  friend class ::roo_windows::Task;
  friend class ::roo_windows::TransientPresentationRegistration;
  friend class ::roo_windows::TransientPresentationSlot;

  /// Validates every caller-owned input without changing host state.
  PresentationStartResult preflight(
      TransientPresentationRegistration& registration, Task& owner,
      Widget& root, const Rect& root_bounds_in_window, FocusScope& scope,
      const TransientSurfaceSpec& spec, const FocusScope* replaced_scope,
      bool allow_admission_guard = false,
      bool validate_root_bounds = true) const;

  /// Returns whether this coordinator currently owns the canonical slot.
  bool isActive() const;

  /// Returns the current hosted registration, or null while inactive.
  TransientPresentationRegistration* activeRegistration() const;

  /// Returns whether the active host has completed input activation.
  bool isInputEnabled() const;

  /// Returns whether `task` supplies the active surface's interaction state.
  bool isInteractionOwner(const Task& task) const;

  /// Applies owner and hosted-subtree semantic-editor containment.
  bool allowsSemanticTextInput(const TextFieldEditor& editor) const;

  /// Delivers a pending outside action after terminal gesture dispatch.
  void flushPendingOutsideInteraction();

  /// Prevents new pointer and key work before component detachment begins.
  void disableHostedInput(TransientPresentationRegistration& registration);

  /// Attaches the admitted structure and activates its focus scope.
  void attachHostedSurface(Widget& root, const Rect& root_bounds_in_window,
                           Task& owner, FocusScope& scope,
                           const TransientSurfaceSpec& spec);

  /// Exits focus and removes all host-owned structural associations.
  void detachHostedSurface(TransientPresentationRegistration& registration,
                           PresentationFinishReason reason);

  /// Finishes the active surface before `owner` begins structural teardown.
  ///
  /// Task teardown marks the owner unavailable before calling this method, so
  /// completion cannot re-admit a surface that depends on the dying task.
  void interactionOwnerUnavailable(Task& owner);

  MainWindow& window_;
  FocusScope* active_scope_ = nullptr;
  uint8_t active_policy_ = 0;
};

/// Resolves the window-owned transient host for `interaction_owner`.
TransientSurfaceHost& GetTransientSurfaceHost(Task& interaction_owner);

}  // namespace internal
}  // namespace roo_windows
