#include "roo_windows/material3/button/navigation.h"

#include "roo_icons/outlined/navigation.h"
#include "roo_windows/core/task.h"

namespace roo_windows {
namespace material3 {

NavigationButton::NavigationButton(ApplicationContext& context,
                                   const MonoIcon& icon)
    : IconButton(context, icon) {
  setSize(ButtonSize::kSmall);
  setStyle(IconButtonStyle::kStandard);
  setShapeMorph(ButtonShapeMorph::kDisabled);
}

BackButton::BackButton(ApplicationContext& context)
    : NavigationButton(context,
                       SCALED_ROO_ICON(outlined, navigation_arrow_back)) {}

void BackButton::onClicked() {
  NavigationButton::onClicked();
  Task* task = getTask();
  if (task != nullptr) task->requestBack(BackSource::kNavigationButton);
}

NavigationButton NavigationButtonArrowBack(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_arrow_back));
}

NavigationButton NavigationButtonClose(ApplicationContext& context) {
  return NavigationButton(context, SCALED_ROO_ICON(outlined, navigation_close));
}

NavigationButton NavigationButtonMenu(ApplicationContext& context) {
  return NavigationButton(context, SCALED_ROO_ICON(outlined, navigation_menu));
}

NavigationButton NavigationButtonExpandMore(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_expand_more));
}

NavigationButton NavigationButtonChevronRight(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_chevron_right));
}

NavigationButton NavigationButtonCancel(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_cancel));
}

NavigationButton NavigationButtonArrowForwardIos(ApplicationContext& context) {
  return NavigationButton(
      context, SCALED_ROO_ICON(outlined, navigation_arrow_forward_ios));
}

NavigationButton NavigationButtonArrowBackIos(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_arrow_back_ios));
}

NavigationButton NavigationButtonArrowDropDown(ApplicationContext& context) {
  return NavigationButton(
      context, SCALED_ROO_ICON(outlined, navigation_arrow_drop_down));
}

NavigationButton NavigationButtonMoreVert(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_more_vert));
}

NavigationButton NavigationButtonArrowForward(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_arrow_forward));
}

NavigationButton NavigationButtonChevronLeft(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_chevron_left));
}

NavigationButton NavigationButtonCheck(ApplicationContext& context) {
  return NavigationButton(context, SCALED_ROO_ICON(outlined, navigation_check));
}

NavigationButton NavigationButtonExpandLess(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_expand_less));
}

NavigationButton NavigationButtonMoreHoriz(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_more_horiz));
}

NavigationButton NavigationButtonRefresh(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_refresh));
}

NavigationButton NavigationButtonApps(ApplicationContext& context) {
  return NavigationButton(context, SCALED_ROO_ICON(outlined, navigation_apps));
}

NavigationButton NavigationButtonArrowUpward(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_arrow_upward));
}

NavigationButton NavigationButtonArrowDownward(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_arrow_downward));
}

NavigationButton NavigationButtonArrowRight(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_arrow_right));
}

NavigationButton NavigationButtonMenuOpen(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_menu_open));
}

NavigationButton NavigationButtonFullscreen(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_fullscreen));
}

NavigationButton NavigationButtonArrowDropUp(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_arrow_drop_up));
}

NavigationButton NavigationButtonUnfoldMore(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_unfold_more));
}

NavigationButton NavigationButtonDoubleArrow(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_double_arrow));
}

NavigationButton NavigationButtonExpandCircleDown(ApplicationContext& context) {
  return NavigationButton(
      context, SCALED_ROO_ICON(outlined, navigation_expand_circle_down));
}

NavigationButton NavigationButtonArrowLeft(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_arrow_left));
}

NavigationButton NavigationButtonFirstPage(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_first_page));
}

NavigationButton NavigationButtonFullscreenExit(ApplicationContext& context) {
  return NavigationButton(
      context, SCALED_ROO_ICON(outlined, navigation_fullscreen_exit));
}

NavigationButton NavigationButtonArrowDropDownCircle(
    ApplicationContext& context) {
  return NavigationButton(
      context, SCALED_ROO_ICON(outlined, navigation_arrow_drop_down_circle));
}

NavigationButton NavigationButtonLastPage(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_last_page));
}

NavigationButton NavigationButtonUnfoldLess(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_unfold_less));
}

NavigationButton NavigationButtonSubdirectoryArrowRight(
    ApplicationContext& context) {
  return NavigationButton(
      context, SCALED_ROO_ICON(outlined, navigation_subdirectory_arrow_right));
}

NavigationButton NavigationButtonLegendToggle(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_legend_toggle));
}

NavigationButton NavigationButtonAppSettingsAlt(ApplicationContext& context) {
  return NavigationButton(
      context, SCALED_ROO_ICON(outlined, navigation_app_settings_alt));
}

NavigationButton NavigationButtonSubdirectoryArrowLeft(
    ApplicationContext& context) {
  return NavigationButton(
      context, SCALED_ROO_ICON(outlined, navigation_subdirectory_arrow_left));
}

NavigationButton NavigationButtonSwitchLeft(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_switch_left));
}

NavigationButton NavigationButtonSwitchRight(ApplicationContext& context) {
  return NavigationButton(context,
                          SCALED_ROO_ICON(outlined, navigation_switch_right));
}

}  // namespace material3
}  // namespace roo_windows
