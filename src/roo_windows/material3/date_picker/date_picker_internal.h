#pragma once

#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/basic_widget.h"
#include "roo_windows/core/transient_surface_host.h"
#include "roo_windows/material3/button/button.h"
#include "roo_windows/material3/date_picker/date_picker.h"
#include "roo_windows/material3/text_field/text_field.h"

namespace roo_windows::material3::internal {

/// Modes share one host, focus scope, and draft.
enum class DatePickerMode : uint8_t { kDays, kMonths, kYears, kInput };

/// Shared calendar surface, defined below the leaf controls.
class DatePickerPanel;

/// Owner-painted header with five keyboard/touch controls.
class DatePickerHeader final : public BasicWidget {
 public:
  /// Borrows the session panel for header state and actions.
  DatePickerHeader(ApplicationContext& context, DatePickerPanel& panel);

  /// Reports whether this node participates in keyboard focus.
  bool isFocusable() const override { return true; }

  /// Accepts tap gestures for arithmetic hit testing.
  bool supportsTap() const override { return true; }

  /// Activates the control at the widget-local touch position.
  void onSingleTapUp(XDim x, YDim y) override;

  /// Handles cursor movement and activation within the current mode.
  bool onKeyEvent(const KeyEvent& event) override;

  /// Paints final foreground and background through the shared clipper.
  void paint(PaintContext& ctx) const override;

  /// Returns a header-local hit rectangle for one of the five controls.
  Rect controlBounds(int control) const;

  /// Marks one control (0–4) or the draft headline (5) as changed.
  void dirtyPart(int part);

  /// Returns fixed target geometry without measuring children.
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(Scaled(208), Scaled(112));
  }

 protected:
  Rect getDirectPaintExclusionBounds() const override { return Rect(); }

 private:
  DatePickerPanel& panel_;
  uint8_t focused_control_ = 0;
  mutable uint8_t dirty_parts_ = 0;
};

/// Arithmetic calendar/list cells, without child objects per cell.
class DatePickerBody final : public BasicSurfaceWidget {
 public:
  /// Borrows the panel for arithmetic cell content and selection.
  DatePickerBody(ApplicationContext& context, DatePickerPanel& panel);

  /// Reports whether this node participates in keyboard focus.
  bool isFocusable() const override { return true; }

  /// Accepts tap gestures for arithmetic hit testing.
  bool supportsTap() const override { return true; }

  /// Activates the control at the widget-local touch position.
  void onSingleTapUp(XDim x, YDim y) override;

  /// Handles cursor movement and activation within the current mode.
  bool onKeyEvent(const KeyEvent& event) override;

  /// Paints final foreground and background through the shared clipper.
  void paint(PaintContext& ctx) const override;

  /// Resolves the enclosing picker surface color.
  Color background() const override;

  /// Returns fixed target geometry without measuring children.
  Dimensions getSuggestedMinimumDimensions() const override;

  /// Selects the draft/month cursor without changing scroll during layout.
  void resetCursor();

  /// Maps a cell index to content-local coordinates.
  Rect cellBounds(int index) const;

  /// Bounds work by the current mode and the final civil year.
  int cellCount() const;

  /// Scrolls the current keyboard cell into the viewport.
  void revealCursor();

 private:
  /// Applies an enabled day or changes the month/year body selection.
  void activate(int index);
  DatePickerPanel& panel_;
  int16_t cursor_ = 0;
  // A completed paint snapshot collapses repeated edits without a cell bitmap.
  mutable roo_time::CivilDay painted_draft_;
  mutable int16_t painted_cursor_ = -1;
  mutable bool painted_focused_ = false;
};

/// Scrollable content leaves surface fill to the panel.
class DatePickerViewport final : public SimpleScrollablePanel {
 public:
  /// Owns scrolling geometry while the enclosing panel paints its background.
  DatePickerViewport(ApplicationContext& context, Widget& body)
      : SimpleScrollablePanel(context, WidgetRef(body), Direction::kBoth) {}

  /// Resolves the enclosing picker surface color.
  Color background() const override { return roo_display::color::Transparent; }

  /// Leaves background coverage to the enclosing picker surface.
  bool fullyCoversBoundsWithOpaqueColors() const override { return false; }

  /// Leaves viewport background painting to the enclosing panel.
  void paint(PaintContext&) const override {}

 protected:
  Rect getDirectPaintExclusionBounds() const override { return Rect(); }
};

/// Surface with pinned chrome and one scrollable, owner-painted body.
class DatePickerPanel final : public Container {
 public:
  /// Constructs fixed chrome and arithmetic body; numeric input stays lazy.
  DatePickerPanel(ApplicationContext& context, DatePickerSession& session);

  /// Detaches borrowed children before their member storage is destroyed.
  ~DatePickerPanel() override;

  /// Resolves the enclosing picker surface color.
  Color background() const override;

  /// Uses the Material 3 high surface-container role.
  ColorToken containerRole() const override;

  /// Rounds floating panels and leaves full-screen corners square.
  BorderStyle getBorderStyle() const override;

  /// Reports whether this node participates in keyboard focus.
  bool isFocusable() const override { return false; }

  /// Focuses the active calendar body or numeric input on admission.
  Widget* preferredFocusChild() override;

  /// Paints final foreground and background through the shared clipper.
  void paint(PaintContext& ctx) const override;

  /// Returns fixed target geometry without measuring children.
  Dimensions getSuggestedMinimumDimensions() const override;

  /// Returns the active model borrowed by this panel.
  DatePickerSession& session() { return session_; }

  /// Returns the active model borrowed by this panel.
  const DatePickerSession& session() const { return session_; }

  /// Returns the currently visible body.
  DatePickerMode mode() const { return mode_; }

  /// Returns the first year of the bounded year page.
  int firstYear() const { return first_year_; }

  /// Dispatches previous, month, year, next or input-toggle activation.
  void activateHeader(int control);

  /// Changes body while preserving one draft; invalid input vetoes departure.
  void setMode(DatePickerMode mode);

  /// Updates only an enabled draft date.
  void selectDate(roo_time::CivilDay day);

  /// Jumps to a month, preserving the day when possible and clearing otherwise.
  void selectMonth(int year, int month);

  /// Parses numeric input and updates validation plus confirmation eligibility.
  void inputChanged();

  /// Refreshes headline, confirmation state and body cursor.
  void updateSelection();

  /// Removes outer corner rounding for compact window-filling presentation.
  void setFullScreen(bool value) { full_screen_ = value; }

  /// Reveals a content-local target without reducing its touch size.
  void scrollTo(const Rect& cell);

  /// Returns the lazy input widget, or null before input is first requested.
  TextField* input() { return input_.get(); }

 protected:
  int getChildrenCount() const override { return input_ ? 5 : 4; }
  Widget& getChild(int index) override;
  const Widget& getChild(int index) const override;
  Dimensions onMeasure(WidthSpec width, HeightSpec height) override;
  void onLayout(bool changed, const Rect& rect) override;

 private:
  /// Fixed Cancel or OK action borrowing the active panel.
  class Action final : public Button {
   public:
    /// Binds one fixed terminal action to the active panel.
    Action(ApplicationContext& context, DatePickerPanel& panel, bool confirm);

    /// Confirms an enabled draft or dismisses without committing.
    void onClicked() override;

   private:
    DatePickerPanel& panel_;
    bool confirm_;
  };

  /// Lazy numeric editor sharing the panel draft and validation.
  class Input final : public TextField {
   public:
    /// Uses the shared codec hint and reports edits to the active panel.
    Input(ApplicationContext& context, DatePickerPanel& panel);

   protected:
    void onTextChanged() override { panel_.inputChanged(); }

   private:
    DatePickerPanel& panel_;
  };

  DatePickerSession& session_;
  DatePickerHeader header_;
  DatePickerBody body_;
  DatePickerViewport viewport_;
  Action cancel_;
  Action confirm_;
  std::unique_ptr<Input> input_;
  DatePickerMode mode_ = DatePickerMode::kDays;
  int16_t first_year_ = 1;
  bool full_screen_ : 1;
  bool syncing_input_ : 1;
  bool reveal_pending_ : 1;
  bool focus_pending_ : 1;
};

/// Active-only state. Destruction explicitly cancels before scope/panel
/// teardown.
class DatePickerSession final : public TransientPresentationRegistration {
 private:
  ModalDatePicker& owner_;
  FocusScope scope_;

 public:
  /// Seeds the draft and visible month from closed presenter configuration.
  explicit DatePickerSession(ModalDatePicker& owner);

  /// Silently cancels the registration before destroying scope and panel.
  ~DatePickerSession() override;

  /// Resolves anchored/centered/compact placement and admits one hosted root.
  PresentationStartResult show(Task& task, const Rect* anchor);

  /// Combines civil validity, bounds and the application filter.
  bool enabled(roo_time::CivilDay day) const;

  /// Finishes with an action only if the current draft remains enabled.
  void accept();

  /// Borrows the presenter locale table.
  const DatePickerStrings& strings() const {
    return owner_.datePickerStrings();
  }

  /// Borrows the presenter numeric codec.
  const DateTextCodec& codec() const { return owner_.dateTextCodec(); }

  /// Returns the explicitly configured today marker.
  roo_time::CivilDay today() const { return owner_.today_; }

  /// Uncommitted selection; invalid means confirmation is disabled.
  roo_time::CivilDay draft;

  /// First day of the visible month, independent of the committed value.
  roo_time::CivilDay month;

  /// Presenter-specific reason retained until terminal delivery.
  DatePickerDismissReason dismiss_reason =
      DatePickerDismissReason::kProgrammatic;

  /// Presentation-only widget storage owned by this registration.
  DatePickerPanel panel;

 protected:
  void detachPresentation(PresentationFinishReason) override {}
  void onFinished(PresentationFinishReason reason) override;
  BackResult onBackRequested(BackSource source) override;
};
}  // namespace roo_windows::material3::internal
