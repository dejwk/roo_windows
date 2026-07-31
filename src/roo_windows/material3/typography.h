#pragma once

#include "roo_fonts/NotoSans_Medium/10_5.h"
#include "roo_fonts/NotoSans_Medium/11.h"
#include "roo_fonts/NotoSans_Medium/12.h"
#include "roo_fonts/NotoSans_Medium/14.h"
#include "roo_fonts/NotoSans_Medium/16.h"
#include "roo_fonts/NotoSans_Medium/16_5.h"
#include "roo_fonts/NotoSans_Medium/18.h"
#include "roo_fonts/NotoSans_Medium/21.h"
#include "roo_fonts/NotoSans_Medium/22.h"
#include "roo_fonts/NotoSans_Medium/24.h"
#include "roo_fonts/NotoSans_Medium/28.h"
#include "roo_fonts/NotoSans_Medium/32.h"
#include "roo_fonts/NotoSans_Medium/8_25.h"
#include "roo_fonts/NotoSans_Medium/9.h"
#include "roo_fonts/NotoSans_Regular/10_5.h"
#include "roo_fonts/NotoSans_Regular/114.h"
#include "roo_fonts/NotoSans_Regular/12.h"
#include "roo_fonts/NotoSans_Regular/14.h"
#include "roo_fonts/NotoSans_Regular/16.h"
#include "roo_fonts/NotoSans_Regular/16_5.h"
#include "roo_fonts/NotoSans_Regular/18.h"
#include "roo_fonts/NotoSans_Regular/21.h"
#include "roo_fonts/NotoSans_Regular/22.h"
#include "roo_fonts/NotoSans_Regular/24.h"
#include "roo_fonts/NotoSans_Regular/27.h"
#include "roo_fonts/NotoSans_Regular/28.h"
#include "roo_fonts/NotoSans_Regular/32.h"
#include "roo_fonts/NotoSans_Regular/33.h"
#include "roo_fonts/NotoSans_Regular/34.h"
#include "roo_fonts/NotoSans_Regular/36.h"
#include "roo_fonts/NotoSans_Regular/42.h"
#include "roo_fonts/NotoSans_Regular/44.h"
#include "roo_fonts/NotoSans_Regular/48.h"
#include "roo_fonts/NotoSans_Regular/54.h"
#include "roo_fonts/NotoSans_Regular/56.h"
#include "roo_fonts/NotoSans_Regular/64.h"
#include "roo_fonts/NotoSans_Regular/68.h"
#include "roo_fonts/NotoSans_Regular/72.h"
#include "roo_fonts/NotoSans_Regular/86.h"
#include "roo_fonts/NotoSans_Regular/90.h"
#include "roo_windows/config.h"
#include "roo_windows/core/text_style.h"

namespace roo_windows::material3 {

#define ROO_M3_STYLE(name, font_expr, height, tracking)                      \
  inline const TextStyle& text_style_##name() {                              \
    static const TextStyle style =                                           \
        roo_windows::internal::MakeTextStyle((font_expr), height, tracking); \
    return style;                                                            \
  }

#if ROO_WINDOWS_ZOOM >= 200
ROO_M3_STYLE(display_large, roo_display::font_NotoSans_Regular_114(), 128, 0)
ROO_M3_STYLE(display_medium, roo_display::font_NotoSans_Regular_90(), 104, 0)
ROO_M3_STYLE(display_small, roo_display::font_NotoSans_Regular_72(), 88, 0)
ROO_M3_STYLE(headline_large, roo_display::font_NotoSans_Regular_64(), 80, 0)
ROO_M3_STYLE(headline_medium, roo_display::font_NotoSans_Regular_56(), 72, 0)
ROO_M3_STYLE(headline_small, roo_display::font_NotoSans_Regular_48(), 64, 0)
ROO_M3_STYLE(title_large, roo_display::font_NotoSans_Regular_44(), 56, 0)
ROO_M3_STYLE(title_medium, roo_display::font_NotoSans_Medium_32(), 48, 0)
ROO_M3_STYLE(title_small, roo_display::font_NotoSans_Medium_28(), 40, 0)
ROO_M3_STYLE(body_large, roo_display::font_NotoSans_Regular_32(), 48, 1)
ROO_M3_STYLE(body_medium, roo_display::font_NotoSans_Regular_28(), 40, 0)
ROO_M3_STYLE(body_small, roo_display::font_NotoSans_Regular_24(), 32, 1)
ROO_M3_STYLE(label_large, roo_display::font_NotoSans_Medium_28(), 40, 0)
ROO_M3_STYLE(label_medium, roo_display::font_NotoSans_Medium_24(), 32, 1)
ROO_M3_STYLE(label_small, roo_display::font_NotoSans_Medium_22(), 32, 1)
#elif ROO_WINDOWS_ZOOM >= 150
ROO_M3_STYLE(display_large, roo_display::font_NotoSans_Regular_86(), 96, 0)
ROO_M3_STYLE(display_medium, roo_display::font_NotoSans_Regular_68(), 78, 0)
ROO_M3_STYLE(display_small, roo_display::font_NotoSans_Regular_54(), 66, 0)
ROO_M3_STYLE(headline_large, roo_display::font_NotoSans_Regular_48(), 60, 0)
ROO_M3_STYLE(headline_medium, roo_display::font_NotoSans_Regular_42(), 54, 0)
ROO_M3_STYLE(headline_small, roo_display::font_NotoSans_Regular_36(), 48, 0)
ROO_M3_STYLE(title_large, roo_display::font_NotoSans_Regular_33(), 42, 0)
ROO_M3_STYLE(title_medium, roo_display::font_NotoSans_Medium_24(), 36, 0)
ROO_M3_STYLE(title_small, roo_display::font_NotoSans_Medium_21(), 30, 0)
ROO_M3_STYLE(body_large, roo_display::font_NotoSans_Regular_24(), 36, 1)
ROO_M3_STYLE(body_medium, roo_display::font_NotoSans_Regular_21(), 30, 0)
ROO_M3_STYLE(body_small, roo_display::font_NotoSans_Regular_18(), 24, 1)
ROO_M3_STYLE(label_large, roo_display::font_NotoSans_Medium_21(), 30, 0)
ROO_M3_STYLE(label_medium, roo_display::font_NotoSans_Medium_18(), 24, 1)
ROO_M3_STYLE(label_small, roo_display::font_NotoSans_Medium_16_5(), 24, 1)
#elif ROO_WINDOWS_ZOOM >= 100
ROO_M3_STYLE(display_large, roo_display::font_NotoSans_Regular_56(), 64, 0)
ROO_M3_STYLE(display_medium, roo_display::font_NotoSans_Regular_44(), 52, 0)
ROO_M3_STYLE(display_small, roo_display::font_NotoSans_Regular_36(), 44, 0)
ROO_M3_STYLE(headline_large, roo_display::font_NotoSans_Regular_32(), 40, 0)
ROO_M3_STYLE(headline_medium, roo_display::font_NotoSans_Regular_28(), 36, 0)
ROO_M3_STYLE(headline_small, roo_display::font_NotoSans_Regular_24(), 32, 0)
ROO_M3_STYLE(title_large, roo_display::font_NotoSans_Regular_22(), 28, 0)
ROO_M3_STYLE(title_medium, roo_display::font_NotoSans_Medium_16(), 24, 0)
ROO_M3_STYLE(title_small, roo_display::font_NotoSans_Medium_14(), 20, 0)
ROO_M3_STYLE(body_large, roo_display::font_NotoSans_Regular_16(), 24, 0)
ROO_M3_STYLE(body_medium, roo_display::font_NotoSans_Regular_14(), 20, 0)
ROO_M3_STYLE(body_small, roo_display::font_NotoSans_Regular_12(), 16, 0)
ROO_M3_STYLE(label_large, roo_display::font_NotoSans_Medium_14(), 20, 0)
ROO_M3_STYLE(label_medium, roo_display::font_NotoSans_Medium_12(), 16, 0)
ROO_M3_STYLE(label_small, roo_display::font_NotoSans_Medium_11(), 16, 0)
#else
ROO_M3_STYLE(display_large, roo_display::font_NotoSans_Regular_42(), 48, 0)
ROO_M3_STYLE(display_medium, roo_display::font_NotoSans_Regular_34(), 39, 0)
ROO_M3_STYLE(display_small, roo_display::font_NotoSans_Regular_27(), 33, 0)
ROO_M3_STYLE(headline_large, roo_display::font_NotoSans_Regular_24(), 30, 0)
ROO_M3_STYLE(headline_medium, roo_display::font_NotoSans_Regular_21(), 27, 0)
ROO_M3_STYLE(headline_small, roo_display::font_NotoSans_Regular_18(), 24, 0)
ROO_M3_STYLE(title_large, roo_display::font_NotoSans_Regular_16_5(), 21, 0)
ROO_M3_STYLE(title_medium, roo_display::font_NotoSans_Medium_12(), 18, 0)
ROO_M3_STYLE(title_small, roo_display::font_NotoSans_Medium_10_5(), 15, 0)
ROO_M3_STYLE(body_large, roo_display::font_NotoSans_Regular_12(), 18, 0)
ROO_M3_STYLE(body_medium, roo_display::font_NotoSans_Regular_10_5(), 15, 0)
ROO_M3_STYLE(body_small, roo_display::font_NotoSans_Regular_9(), 12, 0)
ROO_M3_STYLE(label_large, roo_display::font_NotoSans_Medium_10_5(), 15, 0)
ROO_M3_STYLE(label_medium, roo_display::font_NotoSans_Medium_9(), 12, 0)
ROO_M3_STYLE(label_small, roo_display::font_NotoSans_Medium_8_25(), 12, 0)
#endif

#undef ROO_M3_STYLE

}  // namespace roo_windows::material3
