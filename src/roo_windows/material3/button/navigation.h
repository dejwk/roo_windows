#pragma once

#include "roo_windows/material3/button/icon_button.h"

namespace roo_windows {
namespace material3 {

/// Convenience subclass of `IconButton` that uses a Material 3 navigation
/// icon and a small expressive size. The button is round and does not morph
/// when pressed.
class NavigationButton : public IconButton {
 public:
  /// Creates a navigation button that borrows @p icon, which must outlive it.
  explicit NavigationButton(ApplicationContext& context, const MonoIcon& icon);
};

/// Creates a navigation button that issues semantic 'back' events.
class BackButton : public NavigationButton {
 public:
  /// Creates a back button using @p context for its theme and scheduling.
  explicit BackButton(ApplicationContext& context);

  /// Notifies click listeners, then sends Back through the containing task.
  /// Uses the navigation-button source; detached buttons only notify listeners.
  void onClicked() override;
};

/// Creates a navigation button with the outlined arrow back icon.
NavigationButton NavigationButtonArrowBack(ApplicationContext& context);

/// Creates a navigation button with the outlined close icon.
NavigationButton NavigationButtonClose(ApplicationContext& context);

/// Creates a navigation button with the outlined menu icon.
NavigationButton NavigationButtonMenu(ApplicationContext& context);

/// Creates a navigation button with the outlined expand more icon.
NavigationButton NavigationButtonExpandMore(ApplicationContext& context);

/// Creates a navigation button with the outlined chevron right icon.
NavigationButton NavigationButtonChevronRight(ApplicationContext& context);

/// Creates a navigation button with the outlined cancel icon.
NavigationButton NavigationButtonCancel(ApplicationContext& context);

/// Creates a navigation button with the outlined arrow forward iOS icon.
NavigationButton NavigationButtonArrowForwardIos(ApplicationContext& context);

/// Creates a navigation button with the outlined arrow back iOS icon.
NavigationButton NavigationButtonArrowBackIos(ApplicationContext& context);

/// Creates a navigation button with the outlined arrow drop down icon.
NavigationButton NavigationButtonArrowDropDown(ApplicationContext& context);

/// Creates a navigation button with the outlined more vert icon.
NavigationButton NavigationButtonMoreVert(ApplicationContext& context);

/// Creates a navigation button with the outlined arrow forward icon.
NavigationButton NavigationButtonArrowForward(ApplicationContext& context);

/// Creates a navigation button with the outlined chevron left icon.
NavigationButton NavigationButtonChevronLeft(ApplicationContext& context);

/// Creates a navigation button with the outlined check icon.
NavigationButton NavigationButtonCheck(ApplicationContext& context);

/// Creates a navigation button with the outlined expand less icon.
NavigationButton NavigationButtonExpandLess(ApplicationContext& context);

/// Creates a navigation button with the outlined more horiz icon.
NavigationButton NavigationButtonMoreHoriz(ApplicationContext& context);

/// Creates a navigation button with the outlined refresh icon.
NavigationButton NavigationButtonRefresh(ApplicationContext& context);

/// Creates a navigation button with the outlined apps icon.
NavigationButton NavigationButtonApps(ApplicationContext& context);

/// Creates a navigation button with the outlined arrow upward icon.
NavigationButton NavigationButtonArrowUpward(ApplicationContext& context);

/// Creates a navigation button with the outlined arrow downward icon.
NavigationButton NavigationButtonArrowDownward(ApplicationContext& context);

/// Creates a navigation button with the outlined arrow right icon.
NavigationButton NavigationButtonArrowRight(ApplicationContext& context);

/// Creates a navigation button with the outlined menu open icon.
NavigationButton NavigationButtonMenuOpen(ApplicationContext& context);

/// Creates a navigation button with the outlined fullscreen icon.
NavigationButton NavigationButtonFullscreen(ApplicationContext& context);

/// Creates a navigation button with the outlined arrow drop up icon.
NavigationButton NavigationButtonArrowDropUp(ApplicationContext& context);

/// Creates a navigation button with the outlined unfold more icon.
NavigationButton NavigationButtonUnfoldMore(ApplicationContext& context);

/// Creates a navigation button with the outlined double arrow icon.
NavigationButton NavigationButtonDoubleArrow(ApplicationContext& context);

/// Creates a navigation button with the outlined expand circle down icon.
NavigationButton NavigationButtonExpandCircleDown(ApplicationContext& context);

/// Creates a navigation button with the outlined arrow left icon.
NavigationButton NavigationButtonArrowLeft(ApplicationContext& context);

/// Creates a navigation button with the outlined first page icon.
NavigationButton NavigationButtonFirstPage(ApplicationContext& context);

/// Creates a navigation button with the outlined fullscreen exit icon.
NavigationButton NavigationButtonFullscreenExit(ApplicationContext& context);

/// Creates a navigation button with the outlined arrow drop down circle icon.
NavigationButton NavigationButtonArrowDropDownCircle(
    ApplicationContext& context);

/// Creates a navigation button with the outlined last page icon.
NavigationButton NavigationButtonLastPage(ApplicationContext& context);

/// Creates a navigation button with the outlined unfold less icon.
NavigationButton NavigationButtonUnfoldLess(ApplicationContext& context);

/// Creates a navigation button with the outlined subdirectory arrow right icon.
NavigationButton NavigationButtonSubdirectoryArrowRight(
    ApplicationContext& context);

/// Creates a navigation button with the outlined legend toggle icon.
NavigationButton NavigationButtonLegendToggle(ApplicationContext& context);

/// Creates a navigation button with the outlined app settings alt icon.
NavigationButton NavigationButtonAppSettingsAlt(ApplicationContext& context);

/// Creates a navigation button with the outlined subdirectory arrow left icon.
NavigationButton NavigationButtonSubdirectoryArrowLeft(
    ApplicationContext& context);

/// Creates a navigation button with the outlined switch left icon.
NavigationButton NavigationButtonSwitchLeft(ApplicationContext& context);

/// Creates a navigation button with the outlined switch right icon.
NavigationButton NavigationButtonSwitchRight(ApplicationContext& context);

}  // namespace material3
}  // namespace roo_windows
