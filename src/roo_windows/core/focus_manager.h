#pragma once

#include <cstdint>

namespace roo_windows {

class Widget;

/// Direction used for geometry-based focus traversal.
enum class FocusDirection : uint8_t { kUp, kDown, kLeft, kRight };

/// Intrusive focus record embedded by a focus-owning presenter.
///
/// While active, `root` identifies the presenter's live focus subtree and
/// `last_focused` may identify its current focus. While inactive, `root` is
/// null and `last_focused` may retain a descendant for the next entry. The
/// presenter must call `clearRememberedFocus()` before mutating an inactive
/// subtree in a way that can detach or destroy that remembered descendant.
struct FocusScope {
  FocusScope() = default;
  FocusScope(const FocusScope&) = delete;
  FocusScope& operator=(const FocusScope&) = delete;
  FocusScope(FocusScope&&) = delete;
  FocusScope& operator=(FocusScope&&) = delete;

  /// Forgets the presenter-local focus target retained for the next entry.
  ///
  /// Call this before replacing, detaching, or destroying descendants while
  /// this scope is inactive. Active subtree detachment is handled by the
  /// manager's ordinary focus-lifetime notification.
  void clearRememberedFocus() { last_focused = nullptr; }

  Widget* root = nullptr;
  Widget* last_focused = nullptr;

 private:
  friend class FocusManager;
  Widget* restore_focused_ = nullptr;
};

/// Application-owned focus state and focus-target lifetime tracking.
class FocusManager {
 public:
  /// Creates focus state restricted to `scope_root` when it is non-null.
  ///
  /// A task constructs its manager with its permanently attached task panel,
  /// which forms the implicit base scope. A context-level compatibility
  /// manager may use null to accept any otherwise eligible attached widget.
  explicit FocusManager(Widget* scope_root = nullptr)
      : scope_root_(scope_root) {}

  /// Returns the currently focused widget, or null when no widget has focus.
  Widget* focused() const { return focused_; }

  /// Returns the root of the current legal focus subtree.
  ///
  /// For an ordinary task this is its `TaskPanel`. While a transient presenter
  /// scope is active, it is that presenter's supplied root instead. Focus
  /// requests and Tab/directional traversal must remain descendants of this
  /// root. Task key dispatch bubbles through an explicit presenter root and
  /// stops there; ordinary task bubbling stops before the structural panel.
  /// A context-level compatibility manager may return null.
  Widget* scopeRoot() { return scope_root_; }

  /// @copydoc scopeRoot()
  const Widget* scopeRoot() const { return scope_root_; }

  /// Reports whether `incoming` can be activated for `base_root`.
  ///
  /// Admission succeeds only when `incoming` is inactive and the manager is
  /// currently at `base_root`. For a validated same-owner replacement, callers
  /// may pass the active outgoing `replaced_scope`; admission also succeeds
  /// when the manager is currently rooted at that exact scope's non-null root.
  /// It fails for an already-active incoming record, a manager rooted at an
  /// unrelated base or presenter, or a replacement record that does not match
  /// the manager's current root. This method changes no focus state.
  bool canAdmitScope(const FocusScope& incoming, const Widget& base_root,
                     const FocusScope* replaced_scope) const;

  /// Replaces the active `base_root` scope with presenter `root` and `scope`.
  ///
  /// Activation saves the current base-focus address for later restoration,
  /// clears base focus, records `root` in `scope`, and makes `root` the legal
  /// focus and task-key boundary. It then restores a still-live remembered
  /// presenter target or requests the root's synchronous preferred target. A
  /// null preference is valid and leaves the presenter scope active without a
  /// focused widget. `scope` must pass base-state `canAdmitScope()` preflight.
  void enterScope(FocusScope& scope, Widget& root, Widget& base_root);

  /// Deactivates `scope` and returns the manager to `base_root`.
  ///
  /// Exit remembers the current presenter target, clears presenter focus,
  /// reinstates `base_root` as the legal boundary, and restores the saved base
  /// target if it is still live and eligible. Otherwise it requests the base
  /// root's synchronous preferred target; null deliberately leaves base focus
  /// empty. Call this while both roots and their parent chains are still live.
  void exitScope(FocusScope& scope, Widget& base_root);

  /// Attempts to move focus to an eligible, attached widget.
  bool requestFocus(Widget& widget);

  /// Moves focus through focusable descendants of `root`, wrapping at ends.
  bool moveFocus(Widget& root, bool backwards);

  /// Moves focus to the best eligible descendant in `direction`. Unlike Tab
  /// traversal, directional movement does not wrap.
  bool moveFocusDirection(Widget& root, FocusDirection direction);

  /// Clears focus when `subtree` is about to detach from its parent.
  void onSubtreeDetaching(Widget& subtree);

  /// Clears an exact focus match while a widget is being destroyed.
  void onWidgetDestroying(Widget& widget);

  /// Clears focus if `widget` is no longer eligible after a state change.
  void onWidgetEligibilityChanging(Widget& widget);

 private:
  static bool IsDescendantOf(const Widget& widget, const Widget& ancestor);
  static bool ContainsAddress(Widget& root, const Widget* candidate);

  bool isEligible(const Widget& widget) const;
  void setFocused(Widget* widget);

  Widget* focused_ = nullptr;
  Widget* scope_root_ = nullptr;
};

static_assert(sizeof(FocusScope) == 3 * sizeof(void*),
              "FocusScope must remain a three-pointer presenter record");
static_assert(sizeof(FocusManager) == 2 * sizeof(void*),
              "FocusManager must remain a two-pointer task service");

}  // namespace roo_windows
