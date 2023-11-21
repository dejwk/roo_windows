#pragma once

#include "roo_windows/config.h"
#include "roo_windows/indicators/impl/wifi.h"
#include "roo_windows/indicators/18/wifi.h"
#include "roo_windows/indicators/24/wifi.h"
#include "roo_windows/indicators/36/wifi.h"
#include "roo_windows/indicators/48/wifi.h"

namespace roo_windows {

#if ROO_WINDOWS_ZOOM >= 200
using WifiIndicator = WifiIndicator48x48;
using WifiIndicatorLarge = WifiIndicator48x48;
#elif ROO_WINDOWS_ZOOM >= 150
using WifiIndicator = WifiIndicator36x36;
using WifiIndicatorLarge = WifiIndicator48x48;
#elif ROO_WINDOWS_ZOOM >= 100
using WifiIndicator = WifiIndicator24x24;
using WifiIndicatorLarge = WifiIndicator36x36;
#else
using WifiIndicator = WifiIndicator18x18;
using WifiIndicatorLarge = WifiIndicator24x24;
#endif

}  // namespace roo_windows