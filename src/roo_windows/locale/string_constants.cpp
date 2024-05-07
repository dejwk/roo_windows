#include "roo_windows/locale/string_constants.h"

#include "roo_windows/config.h"

namespace roo_windows {

#if (ROO_WINDOWS_LANG == ROO_WINDOWS_LANG_pl)

const char* kStrOK = "OK";
const char* kStrCancel = "Anuluj";

#else

const char* kStrOK = "OK";
const char* kStrCancel = "Cancel";

#endif

}  // namespace roo_windows