#pragma once

#include "roo_windows/fonts/Roboto_Light/120.h"
#include "roo_windows/fonts/Roboto_Light/144.h"
#include "roo_windows/fonts/Roboto_Light/192.h"
#include "roo_windows/fonts/Roboto_Light/45.h"
#include "roo_windows/fonts/Roboto_Light/60.h"
#include "roo_windows/fonts/Roboto_Light/72.h"
#include "roo_windows/fonts/Roboto_Light/90.h"
#include "roo_windows/fonts/Roboto_Light/96.h"
#include "roo_windows/fonts/Roboto_Medium/11.h"
#include "roo_windows/fonts/Roboto_Medium/14.h"
#include "roo_windows/fonts/Roboto_Medium/15.h"
#include "roo_windows/fonts/Roboto_Medium/20.h"
#include "roo_windows/fonts/Roboto_Medium/21.h"
#include "roo_windows/fonts/Roboto_Medium/28.h"
#include "roo_windows/fonts/Roboto_Medium/30.h"
#include "roo_windows/fonts/Roboto_Medium/40.h"
#include "roo_windows/fonts/Roboto_Regular/8.h"
#include "roo_windows/fonts/Roboto_Regular/9.h"
#include "roo_windows/fonts/Roboto_Regular/10.h"
#include "roo_windows/fonts/Roboto_Regular/11.h"
#include "roo_windows/fonts/Roboto_Regular/12.h"
#include "roo_windows/fonts/Roboto_Regular/14.h"
#include "roo_windows/fonts/Roboto_Regular/15.h"
#include "roo_windows/fonts/Roboto_Regular/16.h"
#include "roo_windows/fonts/Roboto_Regular/18.h"
#include "roo_windows/fonts/Roboto_Regular/20.h"
#include "roo_windows/fonts/Roboto_Regular/21.h"
#include "roo_windows/fonts/Roboto_Regular/24.h"
#include "roo_windows/fonts/Roboto_Regular/26.h"
#include "roo_windows/fonts/Roboto_Regular/28.h"
#include "roo_windows/fonts/Roboto_Regular/32.h"
#include "roo_windows/fonts/Roboto_Regular/34.h"
#include "roo_windows/fonts/Roboto_Regular/36.h"
#include "roo_windows/fonts/Roboto_Regular/48.h"
#include "roo_windows/fonts/Roboto_Regular/51.h"
#include "roo_windows/fonts/Roboto_Regular/68.h"
#include "roo_windows/fonts/Roboto_Regular/72.h"
#include "roo_windows/fonts/Roboto_Regular/96.h"

#ifndef ROO_WINDOWS_ZOOM

#ifdef ROO_DISPLAY_DPI

#if (ROO_DISPLAY_DPI >= 270)
#define ROO_WINDOWS_ZOOM 200
#elif (ROO_DISPLAY_DPI >= 200)
#define ROO_WINDOWS_ZOOM 150
#elif (ROO_DISPLAY_DPI >= 135)
#define ROO_WINDOWS_ZOOM 100
#else
#define ROO_WINDOWS_ZOOM 75
#endif

#else  // ROO_DISPLAY_DPI not defined

#define ROO_WINDOWS_ZOOM 100

#endif
#endif

// ROO_WINDOWS_ICON_SIZE defines the size of icons from roo_material_icons, used
// in clickable UI elements such as buttons.

#if (ROO_WINDOWS_ZOOM >= 200)
#define ROO_WINDOWS_ICON_SIZE 48
#define ROO_WINDOWS_TOOLBAR_ICON_SIZE 36

namespace roo_windows {
template <typename T>
T constexpr Scaled(T in) {
  return in * 2;
}
}  // namespace roo_windows

#define ROO_WINDOWS_FONT_SIZE_H1 192
#define ROO_WINDOWS_FONT_SIZE_H2 120
#define ROO_WINDOWS_FONT_SIZE_H3 96
#define ROO_WINDOWS_FONT_SIZE_H4 68
#define ROO_WINDOWS_FONT_SIZE_H5 48
#define ROO_WINDOWS_FONT_SIZE_H6 40
#define ROO_WINDOWS_FONT_SIZE_SUBTITLE1 32
#define ROO_WINDOWS_FONT_SIZE_SUBTITLE2 28
#define ROO_WINDOWS_FONT_SIZE_BODY1 32
#define ROO_WINDOWS_FONT_SIZE_BODY2 28
#define ROO_WINDOWS_FONT_SIZE_BUTTON 28
#define ROO_WINDOWS_FONT_SIZE_CAPTION 24
#define ROO_WINDOWS_FONT_SIZE_OVERLINE 20

#elif (ROO_WINDOWS_ZOOM >= 150)
#define ROO_WINDOWS_ICON_SIZE 36
#define ROO_WINDOWS_TOOLBAR_ICON_SIZE 24

namespace roo_windows {
template <typename T>
constexpr T Scaled(T in) {
  return in * 3 / 2;
}
}  // namespace roo_windows

#define ROO_WINDOWS_FONT_SIZE_H1 144
#define ROO_WINDOWS_FONT_SIZE_H2 90
#define ROO_WINDOWS_FONT_SIZE_H3 72
#define ROO_WINDOWS_FONT_SIZE_H4 51
#define ROO_WINDOWS_FONT_SIZE_H5 36
#define ROO_WINDOWS_FONT_SIZE_H6 30
#define ROO_WINDOWS_FONT_SIZE_SUBTITLE1 24
#define ROO_WINDOWS_FONT_SIZE_SUBTITLE2 21
#define ROO_WINDOWS_FONT_SIZE_BODY1 24
#define ROO_WINDOWS_FONT_SIZE_BODY2 21
#define ROO_WINDOWS_FONT_SIZE_BUTTON 21
#define ROO_WINDOWS_FONT_SIZE_CAPTION 18
#define ROO_WINDOWS_FONT_SIZE_OVERLINE 15

#elif (ROO_WINDOWS_ZOOM >= 100)
#define ROO_WINDOWS_ICON_SIZE 24
#define ROO_WINDOWS_TOOLBAR_ICON_SIZE 18

#define ROO_WINDOWS_FONT_SIZE_H1 96
#define ROO_WINDOWS_FONT_SIZE_H2 60
#define ROO_WINDOWS_FONT_SIZE_H3 48
#define ROO_WINDOWS_FONT_SIZE_H4 34
#define ROO_WINDOWS_FONT_SIZE_H5 24
#define ROO_WINDOWS_FONT_SIZE_H6 20
#define ROO_WINDOWS_FONT_SIZE_SUBTITLE1 16
#define ROO_WINDOWS_FONT_SIZE_SUBTITLE2 14
#define ROO_WINDOWS_FONT_SIZE_BODY1 16
#define ROO_WINDOWS_FONT_SIZE_BODY2 14
#define ROO_WINDOWS_FONT_SIZE_BUTTON 14
#define ROO_WINDOWS_FONT_SIZE_CAPTION 12
#define ROO_WINDOWS_FONT_SIZE_OVERLINE 10

namespace roo_windows {
template <typename T>
T constexpr Scaled(T in) {
  return in;
}
}  // namespace roo_windows

#else
#define ROO_WINDOWS_ICON_SIZE 18
#define ROO_WINDOWS_TOOLBAR_ICON_SIZE 24

namespace roo_windows {
template <typename T>
T constexpr Scaled(T in) {
  return in * 3 / 4;
}
}  // namespace roo_windows

#define ROO_WINDOWS_FONT_SIZE_H1 72
#define ROO_WINDOWS_FONT_SIZE_H2 45
#define ROO_WINDOWS_FONT_SIZE_H3 36
#define ROO_WINDOWS_FONT_SIZE_H4 26
#define ROO_WINDOWS_FONT_SIZE_H5 18
#define ROO_WINDOWS_FONT_SIZE_H6 15
#define ROO_WINDOWS_FONT_SIZE_SUBTITLE1 12
#define ROO_WINDOWS_FONT_SIZE_SUBTITLE2 11
#define ROO_WINDOWS_FONT_SIZE_BODY1 12
#define ROO_WINDOWS_FONT_SIZE_BODY2 11
#define ROO_WINDOWS_FONT_SIZE_BUTTON 11
#define ROO_WINDOWS_FONT_SIZE_CAPTION 9
#define ROO_WINDOWS_FONT_SIZE_OVERLINE 8

#endif

#define __ROO_CONCAT6__(a, b, c, d, e, f) a##b##c##d##e##f
#define __ROO_CONCAT6(a, b, c, d, e, f) __ROO_CONCAT6__(a, b, c, d, e, f)

#define __ROO_CONCAT2__(a, b) a##b
#define __ROO_CONCAT2(a, b) __ROO_CONCAT2__(a, b)

#define SCALED_ROO_ICON(family, name) \
  __ROO_CONCAT6(ic_, family, _, ROO_WINDOWS_ICON_SIZE, _, name)()

// NOTE: fonts (particularly, large fonts) take up a lot of PROGMEM space, which
// is why we resolve them at compile time here, so that only the fonts that are
// actually referenced get compiled into the program.

namespace roo_windows {

inline const roo_display::Font& font_h1() {
  return __ROO_CONCAT2(roo_display::font_Roboto_Light_,
                       ROO_WINDOWS_FONT_SIZE_H1)();
}

inline const roo_display::Font& font_h2() {
  return __ROO_CONCAT2(roo_display::font_Roboto_Light_,
                       ROO_WINDOWS_FONT_SIZE_H2)();
}

inline const roo_display::Font& font_h3() {
  return __ROO_CONCAT2(roo_display::font_Roboto_Regular_,
                       ROO_WINDOWS_FONT_SIZE_H3)();
}

inline const roo_display::Font& font_h4() {
  return __ROO_CONCAT2(roo_display::font_Roboto_Regular_,
                       ROO_WINDOWS_FONT_SIZE_H4)();
}

inline const roo_display::Font& font_h5() {
  return __ROO_CONCAT2(roo_display::font_Roboto_Regular_,
                       ROO_WINDOWS_FONT_SIZE_H5)();
}

inline const roo_display::Font& font_h6() {
  return __ROO_CONCAT2(roo_display::font_Roboto_Medium_,
                       ROO_WINDOWS_FONT_SIZE_H6)();
}

inline const roo_display::Font& font_subtitle1() {
  return __ROO_CONCAT2(roo_display::font_Roboto_Regular_,
                       ROO_WINDOWS_FONT_SIZE_SUBTITLE1)();
}

inline const roo_display::Font& font_subtitle2() {
  return __ROO_CONCAT2(roo_display::font_Roboto_Medium_,
                       ROO_WINDOWS_FONT_SIZE_SUBTITLE2)();
}

inline const roo_display::Font& font_body1() {
  return __ROO_CONCAT2(roo_display::font_Roboto_Regular_,
                       ROO_WINDOWS_FONT_SIZE_BODY1)();
}

inline const roo_display::Font& font_body2() {
  return __ROO_CONCAT2(roo_display::font_Roboto_Regular_,
                       ROO_WINDOWS_FONT_SIZE_BODY2)();
}

inline const roo_display::Font& font_button() {
  return __ROO_CONCAT2(roo_display::font_Roboto_Medium_,
                       ROO_WINDOWS_FONT_SIZE_BUTTON)();
}

inline const roo_display::Font& font_caption() {
  return __ROO_CONCAT2(roo_display::font_Roboto_Regular_,
                       ROO_WINDOWS_FONT_SIZE_CAPTION)();
}

inline const roo_display::Font& font_overline() {
  return __ROO_CONCAT2(roo_display::font_Roboto_Regular_,
                       ROO_WINDOWS_FONT_SIZE_OVERLINE)();
}

}  // namespace roo_windows