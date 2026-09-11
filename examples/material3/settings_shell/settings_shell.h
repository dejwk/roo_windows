#pragma once

#include "roo_icons/outlined/24/action.h"
#include "roo_icons/outlined/24/device.h"
#include "roo_windows/containers/flex_layout.h"
#include "roo_windows/core/task.h"
#include "roo_windows/material3/app_bar/app_bar.h"
#include "roo_windows/material3/dialog/basic_dialog.h"
#include "roo_windows/material3/dialog/full_screen_dialog.h"
#include "roo_windows/material3/menu/menu.h"
#include "roo_windows/material3/navigation_bar/navigation_bar.h"
#include "roo_windows/material3/snackbar/snackbar.h"
#include "roo_windows/material3/typography.h"

namespace roo_windows::material3::examples {

/// Reference P1.11 shell shared by the sketch and interaction tests.
class SettingsShell final : public SnackbarHost {
 public:
  /// Builds general/network settings using only public component APIs.
  explicit SettingsShell(ApplicationContext& context)
      : SnackbarHost(context),
        top_(context),
        general_(context, "General", &ic_outlined_24_action_settings()),
        network_(context, "Network", &ic_outlined_24_device_wifi_tethering()),
        bar_(context, *this),
        status_(context, "Automatic mode", text_style_body_medium()),
        choose_(context, "Choose mode", ButtonVariant::kFilledTonal),
        reset_(context, "Reset settings", ButtonVariant::kText),
        details_(context, "Connection details", ButtonVariant::kFilledTonal),
        body_(context, FlexDirection::kColumn),
        automatic_("Automatic", *this),
        service_("Service", *this),
        automatic_row_(context),
        service_row_(context),
        group_(context),
        menu_(context),
        confirm_(context, *this),
        editor_body_(
            context,
            "Connection: controller.local\n\nChanges apply when you save.",
            text_style_body_medium()),
        editor_(context, editor_body_, *this) {
    top_.setTitle("General settings");
    bar_.add(general_);
    bar_.add(network_);
    bar_.setSelectedIndex(0);
    body_.setPadding(Padding(Scaled(8)));
    body_.setGap(Scaled(4));
    body_.add(status_);
    body_.add(choose_);
    body_.add(reset_);
    body_.add(details_);
    details_.setVisibility(Visibility::kGone);
    choose_.setOnInteractiveChange([this] { menu_.show(*getTask(), choose_); });
    reset_.setOnInteractiveChange([this] { confirm_.show(*getTask()); });
    details_.setOnInteractiveChange([this] { editor_.show(*getTask()); });
    automatic_row_.setMenuItem(automatic_);
    service_row_.setMenuItem(service_);
    group_.add(automatic_row_);
    group_.add(service_row_);
    menu_.addGroup(group_);
    setTopBar(top_);
    setBottomBar(bar_);
    setBody(body_);
  }

  /// Clears scaffold borrows before inline members are destroyed.
  ~SettingsShell() override {
    snackbars().clear();
    clearTopBar();
    clearBottomBar();
    setBody(WidgetRef());
  }

  /// Returns the public mode picker entry point.
  Button& modeButton() { return choose_; }

  /// Returns the reset confirmation entry point.
  Button& resetButton() { return reset_; }

  /// Returns the navigation-backed detail entry point.
  Button& detailsButton() { return details_; }

  /// Returns the bottom navigation for touch/keyboard interaction.
  NavigationBar& navigationBar() { return bar_; }

  /// Returns the menu for presentation-state inspection.
  Menu& modeMenu() { return menu_; }

  /// Returns the persistent confirmation for presentation-state inspection.
  AlertDialog& confirmation() { return confirm_; }

  /// Returns the navigation-backed editor.
  FullScreenDialog& editor() { return editor_; }

  /// Shows owned feedback, replacing stale feedback without bespoke popup code.
  void showFeedback(const char* text) {
    snackbars().clear();
    feedback_.configure(text);
    snackbars().show(feedback_);
  }

 private:
  class Bar final : public NavigationBar {
   public:
    Bar(ApplicationContext& context, SettingsShell& shell)
        : NavigationBar(context), shell_(shell) {}

   protected:
    void onDestinationInvoked(int index) override {
      shell_.selectScreen(index);
    }

   private:
    SettingsShell& shell_;
  };
  class Mode final : public StandardMenuItem {
   public:
    Mode(const char* title, SettingsShell& shell)
        : StandardMenuItem(StandardMenuItemInit{title}),
          shell_(shell),
          title_(title) {}
    void onInvoked() override { shell_.status_.setText(title_); }

   private:
    SettingsShell& shell_;
    const char* title_;
  };
  class Confirmation final : public AlertDialog {
   public:
    Confirmation(ApplicationContext& context, SettingsShell& shell)
        : AlertDialog(context, "Reset settings?",
                      "Restore controller defaults?", kActions, 2),
          shell_(shell) {}

   protected:
    void onActionInvoked(uint8_t id, DialogActionRole) override {
      shell_.showFeedback(id == 2 ? "Settings reset" : "Reset cancelled");
    }
    void onDismissed(DialogDismissReason) override {
      shell_.showFeedback("Reset cancelled");
    }

   private:
    inline static constexpr DialogActionSpec kActions[] = {
        {1, "Cancel", DialogActionRole::kDismiss},
        {2, "Reset", DialogActionRole::kConfirm}};
    SettingsShell& shell_;
  };
  class Editor final : public FullScreenDialog {
   public:
    Editor(ApplicationContext& context, Widget& body, SettingsShell& shell)
        : FullScreenDialog(context, body), shell_(shell) {
      setHeaderTitle("Connection details");
      setConfirmAction({3, "Save", DialogActionRole::kConfirm});
    }

   protected:
    void onConfirmed(uint8_t) override {
      shell_.showFeedback("Connection saved");
    }

   private:
    SettingsShell& shell_;
  };
  void selectScreen(int index) {
    snackbars().clear();
    const bool network = index == 1;
    top_.setTitle(network ? "Network settings" : "General settings");
    status_.setText(network ? "Controller online" : "Automatic mode");
    choose_.setVisibility(network ? Visibility::kGone : Visibility::kVisible);
    details_.setVisibility(network ? Visibility::kVisible : Visibility::kGone);
  }
  AppBar top_;
  NavigationBarDestination general_;
  NavigationBarDestination network_;
  Bar bar_;
  TextBlock status_;
  Button choose_, reset_, details_;
  FlexLayout body_;
  Mode automatic_, service_;
  MenuEntry automatic_row_, service_row_;
  MenuGroup group_;
  Menu menu_;
  SnackbarRequest feedback_;
  Confirmation confirm_;
  TextBlock editor_body_;
  Editor editor_;
};
}  // namespace roo_windows::material3::examples
