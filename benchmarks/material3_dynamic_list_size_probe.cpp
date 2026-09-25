// Compile with the ESP32 toolchain and inspect named symbols with nm.
#if defined(ROO_WINDOWS_STANDALONE_ABI_PROBE)
#include "benchmarks/standalone_abi_logging_stub.h"
#endif
#include "roo_windows/material3/list/dynamic_list.h"

#define SIZE_PROBE(type, name) \
  [[gnu::used]] unsigned char roo_windows_sizeof_##name[sizeof(type)] = {}

SIZE_PROBE(roo_windows::Widget, widget);
SIZE_PROBE(roo_windows::Container, container);
SIZE_PROBE(roo_windows::ListLayout, list_layout);
SIZE_PROBE(roo_windows::material3::List, list);
SIZE_PROBE(roo_windows::material3::ListEntry, list_entry);
SIZE_PROBE(roo_windows::material3::ListItem, list_item);
SIZE_PROBE(roo_windows::material3::DynamicListBase, dynamic_list_base);
SIZE_PROBE(roo_windows::material3::DynamicList<>, dynamic_list);
SIZE_PROBE(
    roo_windows::material3::ListRow<roo_windows::material3::HeadlineListItem>,
    headline_row);

SIZE_PROBE(
    roo_windows::material3::DynamicListModel<
        roo_windows::material3::ListRow<roo_windows::material3::RadioListItem>>,
    dynamic_model);
SIZE_PROBE(roo_windows::material3::DynamicSingleSelectionListModel<>,
           single_selection_model);
SIZE_PROBE(roo_windows::material3::InvokableListItemBase, invokable_item);
