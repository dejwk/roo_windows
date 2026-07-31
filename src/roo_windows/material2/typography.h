#pragma once

#include "roo_fonts/NotoSans_Light/120.h"
#include "roo_fonts/NotoSans_Light/144.h"
#include "roo_fonts/NotoSans_Light/192.h"
#include "roo_fonts/NotoSans_Light/45.h"
#include "roo_fonts/NotoSans_Light/60.h"
#include "roo_fonts/NotoSans_Light/72.h"
#include "roo_fonts/NotoSans_Light/90.h"
#include "roo_fonts/NotoSans_Light/96.h"
#include "roo_fonts/NotoSans_Medium/11.h"
#include "roo_fonts/NotoSans_Medium/14.h"
#include "roo_fonts/NotoSans_Medium/15.h"
#include "roo_fonts/NotoSans_Medium/20.h"
#include "roo_fonts/NotoSans_Medium/21.h"
#include "roo_fonts/NotoSans_Medium/28.h"
#include "roo_fonts/NotoSans_Medium/30.h"
#include "roo_fonts/NotoSans_Medium/40.h"
#include "roo_fonts/NotoSans_Regular/10.h"
#include "roo_fonts/NotoSans_Regular/11.h"
#include "roo_fonts/NotoSans_Regular/12.h"
#include "roo_fonts/NotoSans_Regular/14.h"
#include "roo_fonts/NotoSans_Regular/15.h"
#include "roo_fonts/NotoSans_Regular/16.h"
#include "roo_fonts/NotoSans_Regular/18.h"
#include "roo_fonts/NotoSans_Regular/20.h"
#include "roo_fonts/NotoSans_Regular/21.h"
#include "roo_fonts/NotoSans_Regular/24.h"
#include "roo_fonts/NotoSans_Regular/26.h"
#include "roo_fonts/NotoSans_Regular/28.h"
#include "roo_fonts/NotoSans_Regular/32.h"
#include "roo_fonts/NotoSans_Regular/34.h"
#include "roo_fonts/NotoSans_Regular/36.h"
#include "roo_fonts/NotoSans_Regular/48.h"
#include "roo_fonts/NotoSans_Regular/51.h"
#include "roo_fonts/NotoSans_Regular/68.h"
#include "roo_fonts/NotoSans_Regular/72.h"
#include "roo_fonts/NotoSans_Regular/8.h"
#include "roo_fonts/NotoSans_Regular/9.h"
#include "roo_fonts/NotoSans_Regular/96.h"
#include "roo_windows/config.h"
#include "roo_windows/core/text_style.h"

namespace roo_windows::material2 {
// Material 2 has a different font ladder per role, so accessors spell out the
// selected font while sharing the exact role line-height and tracking tokens.
#define ROO_M2_STYLE(name, font_expr, height, tracking)                      \
  inline const TextStyle& text_style_##name() {                              \
    static const TextStyle style =                                           \
        roo_windows::internal::MakeTextStyle((font_expr), height, tracking); \
    return style;                                                            \
  }

#if ROO_WINDOWS_ZOOM >= 200
#define ROO_M2_FONT_H1 roo_display::font_NotoSans_Light_192()
#define ROO_M2_FONT_H2 roo_display::font_NotoSans_Light_120()
#define ROO_M2_FONT_H3 roo_display::font_NotoSans_Regular_96()
#define ROO_M2_FONT_H4 roo_display::font_NotoSans_Regular_68()
#define ROO_M2_FONT_H5 roo_display::font_NotoSans_Regular_48()
#define ROO_M2_FONT_H6 roo_display::font_NotoSans_Medium_40()
#define ROO_M2_FONT_S1 roo_display::font_NotoSans_Regular_32()
#define ROO_M2_FONT_S2 roo_display::font_NotoSans_Medium_28()
#define ROO_M2_FONT_B1 roo_display::font_NotoSans_Regular_32()
#define ROO_M2_FONT_B2 roo_display::font_NotoSans_Regular_28()
#define ROO_M2_FONT_BUTTON roo_display::font_NotoSans_Medium_28()
#define ROO_M2_FONT_CAPTION roo_display::font_NotoSans_Regular_24()
#define ROO_M2_FONT_OVERLINE roo_display::font_NotoSans_Regular_20()
#define ROO_M2_SCALE 2
#define ROO_M2_TRACK_H1 -3
#define ROO_M2_TRACK_H2 -1
#define ROO_M2_TRACK_BODY1 1
#define ROO_M2_TRACK_BUTTON 2
#define ROO_M2_TRACK_CAPTION 1
#define ROO_M2_TRACK_OVERLINE 3
#elif ROO_WINDOWS_ZOOM >= 150
#define ROO_M2_FONT_H1 roo_display::font_NotoSans_Light_144()
#define ROO_M2_FONT_H2 roo_display::font_NotoSans_Light_90()
#define ROO_M2_FONT_H3 roo_display::font_NotoSans_Regular_72()
#define ROO_M2_FONT_H4 roo_display::font_NotoSans_Regular_51()
#define ROO_M2_FONT_H5 roo_display::font_NotoSans_Regular_36()
#define ROO_M2_FONT_H6 roo_display::font_NotoSans_Medium_30()
#define ROO_M2_FONT_S1 roo_display::font_NotoSans_Regular_24()
#define ROO_M2_FONT_S2 roo_display::font_NotoSans_Medium_21()
#define ROO_M2_FONT_B1 roo_display::font_NotoSans_Regular_24()
#define ROO_M2_FONT_B2 roo_display::font_NotoSans_Regular_21()
#define ROO_M2_FONT_BUTTON roo_display::font_NotoSans_Medium_21()
#define ROO_M2_FONT_CAPTION roo_display::font_NotoSans_Regular_18()
#define ROO_M2_FONT_OVERLINE roo_display::font_NotoSans_Regular_15()
#define ROO_M2_SCALE 3 / 2
#define ROO_M2_TRACK_H1 -2
#define ROO_M2_TRACK_H2 -1
#define ROO_M2_TRACK_BODY1 1
#define ROO_M2_TRACK_BUTTON 2
#define ROO_M2_TRACK_CAPTION 1
#define ROO_M2_TRACK_OVERLINE 2
#elif ROO_WINDOWS_ZOOM >= 100
#define ROO_M2_FONT_H1 roo_display::font_NotoSans_Light_96()
#define ROO_M2_FONT_H2 roo_display::font_NotoSans_Light_60()
#define ROO_M2_FONT_H3 roo_display::font_NotoSans_Regular_48()
#define ROO_M2_FONT_H4 roo_display::font_NotoSans_Regular_34()
#define ROO_M2_FONT_H5 roo_display::font_NotoSans_Regular_24()
#define ROO_M2_FONT_H6 roo_display::font_NotoSans_Medium_20()
#define ROO_M2_FONT_S1 roo_display::font_NotoSans_Regular_16()
#define ROO_M2_FONT_S2 roo_display::font_NotoSans_Medium_14()
#define ROO_M2_FONT_B1 roo_display::font_NotoSans_Regular_16()
#define ROO_M2_FONT_B2 roo_display::font_NotoSans_Regular_14()
#define ROO_M2_FONT_BUTTON roo_display::font_NotoSans_Medium_14()
#define ROO_M2_FONT_CAPTION roo_display::font_NotoSans_Regular_12()
#define ROO_M2_FONT_OVERLINE roo_display::font_NotoSans_Regular_10()
#define ROO_M2_SCALE 1
#define ROO_M2_TRACK_H1 -1
#define ROO_M2_TRACK_H2 0
#define ROO_M2_TRACK_BODY1 0
#define ROO_M2_TRACK_BUTTON 1
#define ROO_M2_TRACK_CAPTION 0
#define ROO_M2_TRACK_OVERLINE 1
#else
#define ROO_M2_FONT_H1 roo_display::font_NotoSans_Light_72()
#define ROO_M2_FONT_H2 roo_display::font_NotoSans_Light_45()
#define ROO_M2_FONT_H3 roo_display::font_NotoSans_Regular_36()
#define ROO_M2_FONT_H4 roo_display::font_NotoSans_Regular_26()
#define ROO_M2_FONT_H5 roo_display::font_NotoSans_Regular_18()
#define ROO_M2_FONT_H6 roo_display::font_NotoSans_Medium_15()
#define ROO_M2_FONT_S1 roo_display::font_NotoSans_Regular_12()
#define ROO_M2_FONT_S2 roo_display::font_NotoSans_Medium_11()
#define ROO_M2_FONT_B1 roo_display::font_NotoSans_Regular_12()
#define ROO_M2_FONT_B2 roo_display::font_NotoSans_Regular_11()
#define ROO_M2_FONT_BUTTON roo_display::font_NotoSans_Medium_11()
#define ROO_M2_FONT_CAPTION roo_display::font_NotoSans_Regular_9()
#define ROO_M2_FONT_OVERLINE roo_display::font_NotoSans_Regular_8()
#define ROO_M2_SCALE 3 / 4
#define ROO_M2_TRACK_H1 -1
#define ROO_M2_TRACK_H2 0
#define ROO_M2_TRACK_BODY1 0
#define ROO_M2_TRACK_BUTTON 1
#define ROO_M2_TRACK_CAPTION 0
#define ROO_M2_TRACK_OVERLINE 1
#endif

ROO_M2_STYLE(h1, ROO_M2_FONT_H1, 112 * ROO_M2_SCALE, ROO_M2_TRACK_H1)
ROO_M2_STYLE(h2, ROO_M2_FONT_H2, 72 * ROO_M2_SCALE, ROO_M2_TRACK_H2)
ROO_M2_STYLE(h3, ROO_M2_FONT_H3, 56 * ROO_M2_SCALE, 0)
ROO_M2_STYLE(h4, ROO_M2_FONT_H4, 40 * ROO_M2_SCALE, 0)
ROO_M2_STYLE(h5, ROO_M2_FONT_H5, 32 * ROO_M2_SCALE, 0)
ROO_M2_STYLE(h6, ROO_M2_FONT_H6, 28 * ROO_M2_SCALE, 0)
ROO_M2_STYLE(subtitle1, ROO_M2_FONT_S1, 24 * ROO_M2_SCALE, 0)
ROO_M2_STYLE(subtitle2, ROO_M2_FONT_S2, 20 * ROO_M2_SCALE, 0)
ROO_M2_STYLE(body1, ROO_M2_FONT_B1, 24 * ROO_M2_SCALE, ROO_M2_TRACK_BODY1)
ROO_M2_STYLE(body2, ROO_M2_FONT_B2, 20 * ROO_M2_SCALE, 0)
ROO_M2_STYLE(button, ROO_M2_FONT_BUTTON, 16 * ROO_M2_SCALE, ROO_M2_TRACK_BUTTON)
ROO_M2_STYLE(caption, ROO_M2_FONT_CAPTION, 16 * ROO_M2_SCALE,
             ROO_M2_TRACK_CAPTION)
ROO_M2_STYLE(overline, ROO_M2_FONT_OVERLINE, 16 * ROO_M2_SCALE,
             ROO_M2_TRACK_OVERLINE)

#undef ROO_M2_STYLE
#undef ROO_M2_FONT_H1
#undef ROO_M2_FONT_H2
#undef ROO_M2_FONT_H3
#undef ROO_M2_FONT_H4
#undef ROO_M2_FONT_H5
#undef ROO_M2_FONT_H6
#undef ROO_M2_FONT_S1
#undef ROO_M2_FONT_S2
#undef ROO_M2_FONT_B1
#undef ROO_M2_FONT_B2
#undef ROO_M2_FONT_BUTTON
#undef ROO_M2_FONT_CAPTION
#undef ROO_M2_FONT_OVERLINE
#undef ROO_M2_SCALE
#undef ROO_M2_TRACK_H1
#undef ROO_M2_TRACK_H2
#undef ROO_M2_TRACK_BODY1
#undef ROO_M2_TRACK_BUTTON
#undef ROO_M2_TRACK_CAPTION
#undef ROO_M2_TRACK_OVERLINE

}  // namespace roo_windows::material2
