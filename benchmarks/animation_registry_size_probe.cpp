// Target-ABI probe only. Inspect the named symbols with nm; this translation
// unit intentionally has no executable behavior.
#include "roo_windows/containers/horizontal_page_host.h"
#include "roo_windows/containers/scrollable_panel.h"
#include "roo_windows/core/animation_registry.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/widget.h"
#include "roo_windows/material3/button/toggle_icon_button.h"
#include "roo_windows/material3/list/list.h"
#include "roo_windows/material3/snackbar/snackbar.h"
#include "roo_windows/material3/switch/switch.h"
#include "roo_windows/material3/tabs/tabs.h"
#include "roo_windows/widgets/progress_bar.h"
#include "roo_windows/widgets/switch.h"
#include "roo_windows/widgets/text_field.h"

namespace roo_windows::test {

struct AnimationRegistryTestAccess {
  static constexpr size_t channelKeySize() {
    return sizeof(AnimationRegistry::ChannelKey);
  }
  static constexpr size_t trackSize() {
    return sizeof(AnimationRegistry::Track);
  }
  static constexpr size_t dispatchItemSize() {
    return sizeof(AnimationRegistry::DispatchItem);
  }
};

}  // namespace roo_windows::test

#define ROO_WINDOWS_ANIMATION_SIZE_PROBE(type, name) \
  [[gnu::used]] unsigned char animation_sizeof_##name[sizeof(type)] = {}
#define ROO_WINDOWS_ANIMATION_VALUE_PROBE(value, name) \
  [[gnu::used]] unsigned char animation_sizeof_##name[(value)] = {}

ROO_WINDOWS_ANIMATION_SIZE_PROBE(roo_windows::AnimationRegistry, registry);
ROO_WINDOWS_ANIMATION_VALUE_PROBE(
    roo_windows::test::AnimationRegistryTestAccess::channelKeySize(),
    channel_key);
ROO_WINDOWS_ANIMATION_VALUE_PROBE(
    roo_windows::test::AnimationRegistryTestAccess::trackSize(), track);
ROO_WINDOWS_ANIMATION_VALUE_PROBE(
    roo_windows::test::AnimationRegistryTestAccess::dispatchItemSize(),
    dispatch_item);
ROO_WINDOWS_ANIMATION_SIZE_PROBE(roo_windows::ApplicationContext,
                                 application_context);
ROO_WINDOWS_ANIMATION_SIZE_PROBE(roo_windows::Widget, widget);
ROO_WINDOWS_ANIMATION_SIZE_PROBE(roo_windows::material3::ExpandablePanel,
                                 expandable_panel);
ROO_WINDOWS_ANIMATION_SIZE_PROBE(roo_windows::HorizontalPageHost,
                                 horizontal_page_host);
ROO_WINDOWS_ANIMATION_SIZE_PROBE(roo_windows::material3::Tabs, tabs);
ROO_WINDOWS_ANIMATION_SIZE_PROBE(roo_windows::material3::ScrollableTabs,
                                 scrollable_tabs);
ROO_WINDOWS_ANIMATION_SIZE_PROBE(roo_windows::SimpleScrollablePanel,
                                 simple_scrollable_panel);
ROO_WINDOWS_ANIMATION_SIZE_PROBE(roo_windows::material3::Switch,
                                 material3_switch);
ROO_WINDOWS_ANIMATION_SIZE_PROBE(roo_windows::Switch, legacy_switch);
ROO_WINDOWS_ANIMATION_SIZE_PROBE(roo_windows::material3::ToggleIconButton,
                                 toggle_icon_button);
ROO_WINDOWS_ANIMATION_SIZE_PROBE(roo_windows::ProgressBar, legacy_progress_bar);
ROO_WINDOWS_ANIMATION_SIZE_PROBE(roo_windows::TextFieldEditor,
                                 text_field_editor);
ROO_WINDOWS_ANIMATION_SIZE_PROBE(roo_windows::TextField, text_field);
ROO_WINDOWS_ANIMATION_SIZE_PROBE(roo_windows::material3::SnackbarPresenter,
                                 snackbar_presenter);
ROO_WINDOWS_ANIMATION_SIZE_PROBE(roo_windows::material3::SnackbarHost,
                                 snackbar_host);
