// Target-ABI measurements; this translation unit has no runtime behavior.
#if defined(ROO_WINDOWS_STANDALONE_ABI_PROBE)
#include "benchmarks/standalone_abi_logging_stub.h"
#endif
#include "roo_windows/material3/date_picker/date_picker_internal.h"
#include "roo_windows/material3/date_picker/docked_date_picker_field.h"

#define PROBE(type, name) \
  [[gnu::used]] unsigned char roo_windows_sizeof_##name[sizeof(type)] = {}

PROBE(roo_windows::material3::ModalDatePicker, modal_date_picker);
PROBE(roo_windows::material3::DockedDatePickerField, docked_date_picker_field);
PROBE(roo_windows::material3::TextField, text_field);
PROBE(roo_windows::material3::internal::DatePickerSession, date_picker_session);
PROBE(roo_windows::material3::internal::DatePickerPanel, date_picker_panel);
PROBE(roo_windows::material3::internal::DatePickerBody, date_picker_body);
PROBE(roo_windows::material3::internal::DatePickerHeader, date_picker_header);
