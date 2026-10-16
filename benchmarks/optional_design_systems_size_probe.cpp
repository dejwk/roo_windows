// ESP32-C3 ABI size baseline. Inspect sizeof_* symbols with nm -S.
#include "benchmarks/standalone_abi_logging_stub.h"
#include "roo_windows/core/click_animation.h"
#include "roo_windows/core/clipper.h"
#include "roo_windows/core/container.h"
#include "roo_windows/core/overlay_spec.h"
#include "roo_windows/core/paint_context.h"
#include "roo_windows/core/paint_effect.h"
#include "roo_windows/core/rounded_clip.h"
#include "roo_windows/core/surface_widget.h"
#include "roo_windows/core/theme.h"
#include "roo_windows/core/widget.h"

using namespace roo_windows;

[[gnu::used]] char sizeof_widget[sizeof(Widget)];
[[gnu::used]] char sizeof_surface_widget[sizeof(SurfaceWidget)];
[[gnu::used]] char sizeof_container[sizeof(Container)];
[[gnu::used]] char sizeof_theme[sizeof(Theme)];
[[gnu::used]] char sizeof_material2_theme[sizeof(material2::Material2Theme)];
[[gnu::used]] char sizeof_material3_theme[sizeof(material3::Material3Theme)];
[[gnu::used]] char sizeof_click_animation[sizeof(ClickAnimation)];
[[gnu::used]] char sizeof_overlay_spec[sizeof(OverlaySpec)];
[[gnu::used]] char sizeof_paint_context[sizeof(PaintContext)];
[[gnu::used]] char
    sizeof_clipper_state[sizeof(roo_windows::internal::ClipperState)];
[[gnu::used]] char
    sizeof_paint_effect_stack[sizeof(roo_windows::internal::PaintEffectStack)];
[[gnu::used]] char
    sizeof_rounded_arena[sizeof(roo_windows::internal::RoundedPaintState)];
