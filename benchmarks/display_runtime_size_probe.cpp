// Target-ABI probe only. Inspect these named symbols with nm; this translation
// unit intentionally has no executable behavior.
#include "roo_windows/activities/keyboard.h"
#include "roo_windows/core/application.h"
#include "roo_windows/core/application_context.h"
#include "roo_windows/core/focus_manager.h"
#include "roo_windows/core/gesture_detector.h"
#include "roo_windows/core/main_window.h"
#include "roo_windows/core/task.h"
#include "roo_windows/core/touch_sensor.h"
#include "roo_windows/core/transient_presentation.h"
#include "roo_windows/core/widget_ref.h"
#include "roo_windows/widgets/text_field.h"

#define ROO_WINDOWS_SIZE_PROBE(type, name) \
  [[gnu::used]] unsigned char roo_windows_sizeof_##name[sizeof(type)] = {}

ROO_WINDOWS_SIZE_PROBE(roo_windows::Application, application);
ROO_WINDOWS_SIZE_PROBE(roo_windows::ApplicationContext, application_context);
ROO_WINDOWS_SIZE_PROBE(roo_windows::MainWindow, main_window);
ROO_WINDOWS_SIZE_PROBE(roo_windows::TouchSensor, touch_sensor);
ROO_WINDOWS_SIZE_PROBE(roo_windows::GestureDetector, gesture_detector);
ROO_WINDOWS_SIZE_PROBE(roo_windows::FocusManager, focus_manager);
ROO_WINDOWS_SIZE_PROBE(roo_windows::TextFieldEditor, text_field_editor);
ROO_WINDOWS_SIZE_PROBE(roo_windows::Keyboard, keyboard);
ROO_WINDOWS_SIZE_PROBE(roo_windows::Task, task);
ROO_WINDOWS_SIZE_PROBE(roo_windows::TaskPanel, task_panel);
ROO_WINDOWS_SIZE_PROBE(roo_windows::WidgetRef, widget_ref);
ROO_WINDOWS_SIZE_PROBE(roo_windows::KeyEvent, key_event);
ROO_WINDOWS_SIZE_PROBE(roo_windows::TransientPresentationSlot,
                       transient_presentation_slot);
