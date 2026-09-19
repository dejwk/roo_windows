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
class DatePickerPanel;

/// Owner-painted header with five keyboard/touch controls.
class DatePickerHeader final : public BasicWidget {
 public:
  /// Borrows the session panel for header state and actions.
  DatePickerHeader(ApplicationContext& context, DatePickerPanel& panel);
  bool isFocusable() const override { return true; }
  bool supportsTap() const override { return true; }
  /// Repaints the keyboard cursor when focus enters or leaves this control.
  void onFocusChanged(bool) override { invalidateInterior(); }
  void onSingleTapUp(XDim x, YDim y) override;
  bool onKeyEvent(const KeyEvent& event) override;
  void paint(PaintContext& ctx) const override;
  /// Returns a header-local hit rectangle for one of the five controls.
  Rect controlBounds(int control) const;
  Dimensions getSuggestedMinimumDimensions() const override {
    return Dimensions(Scaled(208), Scaled(112));
  }

 protected:
  Rect getDirectPaintExclusionBounds() const override { return Rect(); }

 private:
  DatePickerPanel& panel_;
  uint8_t focused_control_ = 0;
};

/// Arithmetic calendar/list cells, without child objects per cell.
class DatePickerBody final : public BasicSurfaceWidget {
 public:
  /// Borrows the panel for arithmetic cell content and selection.
  DatePickerBody(ApplicationContext& context, DatePickerPanel& panel);
  bool isFocusable() const override { return true; }
  bool supportsTap() const override { return true; }
  /// Repaints the keyboard cursor when focus enters or leaves this control.
  void onFocusChanged(bool) override { invalidateInterior(); }
  void onSingleTapUp(XDim x, YDim y) override;
  bool onKeyEvent(const KeyEvent& event) override;
  void paint(PaintContext& ctx) const override;
  Color background() const override;
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
  void activate(int index);
  DatePickerPanel& panel_;
  int16_t cursor_ = 0;
};

/// Scrollable content leaves surface fill to the panel.
class DatePickerViewport final : public SimpleScrollablePanel {
 public:
  /// Owns scrolling geometry while the enclosing panel paints its background.
  DatePickerViewport(ApplicationContext& context, Widget& body)
      : SimpleScrollablePanel(context, WidgetRef(body), Direction::kBoth) {}
  Color background() const override { return roo_display::color::Transparent; }
  bool fullyCoversBoundsWithOpaqueColors() const override { return false; }
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
  Color background() const override;
  ColorToken containerRole() const override;
  BorderStyle getBorderStyle() const override;
  bool isFocusable() const override { return false; }
  /// Focuses the active calendar body or numeric input on admission.
  Widget* preferredFocusChild() override;
  void paint(PaintContext& ctx) const override;
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
  class Action final : public Button {
   public:
    Action(ApplicationContext& context, DatePickerPanel& panel, bool confirm);
    void onClicked() override;

   private:
    DatePickerPanel& panel_;
    bool confirm_;
  };

  class Input final : public TextField {
   public:
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
  bool full_screen_ = false;
  bool syncing_input_ = false;
  bool reveal_pending_ = true;
  bool focus_pending_ = false;
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
