#pragma once

#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/editor_destination.h"
#include "roo_windows/material2/button.h"
#include "roo_windows/material2/text_field.h"

namespace roo_windows::material2 {

/// Full-screen Material 2 draft editor with Back and Clear actions.
/// Borrows its navigation host; remove it from history before destruction.
/// The app's default design system selects its lazy editorDestination().
class EditorDestination : public ::roo_windows::EditorDestination {
 public:
  /// Creates a destination using @p context and initial placeholder @p hint.
  explicit EditorDestination(ApplicationContext& context,
                             const std::string& hint = "");

  /// Returns the pane containing the system's field and actions.
  Widget& getContents() override { return main_pane_; }

 private:
  class EditedTextField : public TextField {
   public:
    /// Creates the destination-owned draft field.
    EditedTextField(ApplicationContext& context, EditorDestination& owner);
    /// Routes keyboard confirmation through detached destination completion.
    void onEditFinished(bool confirmed) override;
    /// Keeps destination actions reachable while the keyboard is open.
    Widget& editInteractionRoot() override { return owner_.getContents(); }
    /// Updates Clear visibility after keyboard edits.
    void notifyTextChanged() override;
    EditorDestination& owner_;
  };

  class Pane : public VerticalLayout {
   public:
    using VerticalLayout::VerticalLayout;
    Dimensions onMeasure(WidthSpec width, HeightSpec height) override {
      return VerticalLayout::onMeasure(width, AvailableHeight(*this, height));
    }
    Color background() const override {
      return theme().material2Theme().color.surface;
    }
    SurfaceColors surfaceColors() const override {
      const auto& colors = theme().material2Theme().color;
      return {colors.onSurface, colors.primary};
    }
  };

  void prepareDraft(const std::string& initial, const std::string& hint,
                    bool obscure) override;
  const std::string& draft() const override;
  void clearDraft() override;
  void editInPlace() override;
  void updateClearVisibility();

  class ActionButton : public SimpleButton {
   public:
    using SimpleButton::SimpleButton;
    PreferredSize getPreferredSize() const override {
      return {PreferredSize::ExactWidth(Scaled(40)),
              PreferredSize::ExactHeight(Scaled(40))};
    }
    Padding getPadding() const override { return Padding(Scaled(8)); }

   protected:
    Color defaultContentColor() const override {
      return theme().material2Theme().color.onSurface;
    }
  };

  class Row : public HorizontalLayout {
   public:
    using HorizontalLayout::HorizontalLayout;
    PreferredSize getPreferredSize() const override {
      return {PreferredSize::MatchParentWidth(),
              PreferredSize::WrapContentHeight()};
    }
  };

  ActionButton back_;
  EditedTextField text_;
  ActionButton clear_;
  // Containers detach their borrowed controls before those controls die.
  Row content_pane_;
  Pane main_pane_;
};

}  // namespace roo_windows::material2
