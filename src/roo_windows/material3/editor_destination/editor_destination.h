#pragma once

#include "roo_windows/containers/horizontal_layout.h"
#include "roo_windows/containers/vertical_layout.h"
#include "roo_windows/core/editor_destination.h"
#include "roo_windows/material3/button/icon_button.h"
#include "roo_windows/material3/text_field/text_field.h"

namespace roo_windows::material3 {

/// Full-screen Material 3 draft editor with Back and Clear actions.
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
    /// Binds the shared editor directly, even in a very small viewport.
    void edit() override;
    /// Allows the draft to fit the keyboard viewport even below its natural
    /// height.
    PreferredSize getPreferredSize() const override {
      return PreferredSize(PreferredSize::MatchParentWidth(),
                           PreferredSize::WrapContentHeight());
    }
    /// Separates the input from each adjacent action by 4dp.
    Margins getMargins() const override {
      return Margins::Horizontal(Scaled(4));
    }
    /// Keeps the draft compact independently of the application's density.
    DensityOverride densityOverride() const override {
      return DensityOverride::Explicit(Density::kMinus2);
    }
    bool obscureText() const override { return obscure; }
    bool obscure = false;
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
      return theme().material3Theme().color.surface;
    }
    SurfaceColors surfaceColors() const override {
      const auto& colors = theme().material3Theme().color;
      return {colors.onSurface, colors.primary};
    }
  };

  void prepareDraft(const std::string& initial, const std::string& hint,
                    bool obscure) override;
  const std::string& draft() const override;
  void clearDraft() override;
  void editInPlace() override;
  void updateClearVisibility();

  class Row : public HorizontalLayout {
   public:
    using HorizontalLayout::HorizontalLayout;
    Margins getMargins() const override { return Margins(Scaled(4)); }
    PreferredSize getPreferredSize() const override {
      return {PreferredSize::MatchParentWidth(),
              PreferredSize::WrapContentHeight()};
    }
  };

  std::string hint_;
  IconButton back_;
  EditedTextField text_;
  IconButton clear_;
  // Containers detach their borrowed controls before those controls die.
  Row content_pane_;
  Pane main_pane_;
};

}  // namespace roo_windows::material3
