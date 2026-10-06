# Accelerated scrolling (experimental)

`AcceleratedScrollablePanel` lets an application accept temporary trails in
exchange for shorter background output during movement on a slow display.
Ordinary scrollers and drawing operations retain their complete-paint behavior.
The feature needs measurement and visual review on the application's hardware
before adoption; a framebuffer emulator does not establish its benefit.

```cpp
#include "roo_windows/containers/accelerated_scrollable_panel.h"

app.window().setAdvisoryPaintBudget(roo_time::Millis(16));
AcceleratedScrollablePanel log_view(app.context(), WidgetRef(log_rows));
// Attach log_view with the application's normal layout and ownership API.
```

The window snapshots one advisory deadline before animation and layout. Zero
means unlimited. A draw that has started always completes, even if it crosses
the deadline. No traversal, animation sample, or composition state resumes in a
later frame.

During movement, default container backgrounds offer optional output in
16-device-pixel bands. Each moving paint guarantees one rotating band. Other
eligible bands can retain old pixels after the budget expires. Differently
colored rows and pending shadows, outlines, overlays, and captured rounded
foreground can share this lag. Pixels that are emitted use normal current
composition. Fills that feed an active mask's fractional edge colors and fills
under active content effects remain mandatory. Custom foreground drawing still
executes normally; a custom background becomes optional only when explicitly
offered through `PaintContext::clearDeferrableBackground()`.

Every actual omission requests normal full-viewport damage. At the next paint
with an unchanged scroll position, the panel completes the entire current image
regardless of the budget. This works during a held-still drag and after the last
fling sample without another input event. For an unchanged viewport, every band
receives current output within one complete rotation of eligible moving paints.
This is a bound in paint opportunities, not milliseconds.

Content changes, external damage, geometry changes, reveal, and presentation
changes require complete painting. A partial paint cannot certify full cleanup.
Nested accelerated panels and unclipped child groups suspend optional fills.
Caches cannot copy inside an active scope or publish approximate output as a
reusable source.

Call `requestCompleteRedraw()` to request a clean frame before capture or when
application policy demands it. This requests a later refresh and does not stop
motion. Cleanup can take as long as an ordinary full repaint, and mandatory
foreground work can exceed the advisory allowance too.

See the [design](design/in_progress/selective_accelerated_redraw_design.md) for
the rendering contract and release gates, and the
[scrolling log example](../examples/simple/scrolling_log/scrolling_log.ino) for a
complete comparison.

The [host/model measurement report](design/in_progress/measurements/selective_accelerated_redraw_host.md)
records the current evidence and outstanding release gates.
