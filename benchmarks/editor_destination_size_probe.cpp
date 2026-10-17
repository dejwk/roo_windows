#include "roo_windows/core/application_context.h"
#include "roo_windows/core/environment.h"
#include "roo_windows/material2/text_field.h"
#include "roo_windows/material3/text_field/text_field.h"
#if __has_include("roo_windows/core/editor_destination.h")
#include "roo_windows/material2/editor_destination/editor_destination.h"
#include "roo_windows/material3/editor_destination/editor_destination.h"
#else
#include "roo_windows/keyboard/editor_destination.h"
#endif
extern "C" {
char editor_size_context[sizeof(roo_windows::ApplicationContext)];
char editor_size_environment[sizeof(roo_windows::Environment)];
char editor_size_widget[sizeof(roo_windows::Widget)];
char editor_size_m2_field[sizeof(roo_windows::material2::TextField)];
char editor_size_m3_field[sizeof(roo_windows::material3::TextField)];
#if __has_include("roo_windows/core/editor_destination.h")
char editor_size_m2_destination[sizeof(
    roo_windows::material2::EditorDestination)];
char editor_size_m3_destination[sizeof(
    roo_windows::material3::EditorDestination)];
#else
char editor_size_old_destination[sizeof(roo_windows::EditorDestination)];
#endif
}
