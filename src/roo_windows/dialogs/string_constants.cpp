#include "roo_windows/config.h"
#include "roo_windows/dialogs/string_constants.h"

namespace roo_windows {

#if (ROO_WINDOWS_LANG == ROO_LANG_pl)

const char* kStrDialogOK = "OK";
const char* kStrDialogConfirm = "POTWIERDŹ";
const char* kStrDialogContinue = "KONTYNUUJ";

const char* kStrDialogAbort = "PRZERWIJ";
const char* kStrDialogCancel = "ANULUJ";
const char* kStrDialogExit = "WYJDŹ";

#else

const char* kStrDialogOK = "OK";
const char* kStrDialogConfirm = "CONFIRM";
const char* kStrDialogContinue = "CONTINUE";

const char* kStrDialogAbort = "ABORT";
const char* kStrDialogCancel = "CANCEL";
const char* kStrDialogExit = "EXIT";

#endif

}
