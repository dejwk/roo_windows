#pragma once

#include "roo_windows/config.h"
#include "roo_windows/indicators/impl/walltime.h"
#include "roo_windows/indicators/18/walltime.h"
#include "roo_windows/indicators/24/walltime.h"
// #include "roo_windows/indicators/36/Walltime.h"
// #include "roo_windows/indicators/48/Walltime.h"

namespace roo_windows {

#if ROO_WINDOWS_ZOOM >= 200
// using WalltimeIndicator = WalltimeIndicator48x48;
// using WalltimeIndicatorLarge = WalltimeIndicator48x48;
// #elif ROO_WINDOWS_ZOOM >= 150
// using WalltimeIndicator = WalltimeIndicator36x36;
// using WalltimeIndicatorLarge = WalltimeIndicator48x48;
#elif ROO_WINDOWS_ZOOM >= 100
using WalltimeIndicator = WalltimeIndicator24;
#else
using WalltimeIndicator = WalltimeIndicator18;
#endif

}  // namespace roo_windows