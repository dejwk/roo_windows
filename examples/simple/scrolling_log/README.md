# Scrolling log comparison

Compare complete scrolling with temporary background trails in an offline log.
Choose **Complete** or **Accelerated**, drag or fling the rows, then hold still
or release. Both views keep independent scroll positions. The accelerated view
uses a 16 ms advisory allowance and completes its image when movement pauses.
The rounded rows intentionally have contrasting surfaces, outlines and shadows.

Run from the roo_windows repository:

```sh
bazel run //examples/simple/scrolling_log:scrolling_log
```

roo_testing 2.3.1 can lose retained pixels on native-size window redraws. The
local frontend fix is in the canonical roo_testing repository. Until that fix
is released, validate with:

```sh
bazel run //examples/simple/scrolling_log:scrolling_log \
  --override_module=roo_testing=$HOME/Documents/Arduino/roo/roo_testing
```

The emulator needs no network or credentials. Its fast framebuffer does not
represent SPI transfer time; use the [host/model measurements](../../../docs/design/in_progress/measurements/selective_accelerated_redraw_host.md)
for the deliberately modeled comparison. Evaluate trails and readability on
your own hardware before adopting this experimental feature.

For the physical sketch, use an ESP32 with a 240×320 ILI9341 SPI display and an
XPT2046 touch controller. The sketch rotates the display to 320×240 landscape.
The ILI9341 driver uses its default 40 MHz SPI configuration. Wire the shared
SPI bus and individual control pins as follows, and provide the power and
ground connections required by your module:

| Signal | ESP32 GPIO |
| --- | ---: |
| SPI SCK | 4 |
| SPI MISO | 5 |
| SPI MOSI | 6 |
| Display CS | 7 |
| Display DC | 2 |
| Display reset | 3 |
| Touch CS | 1 |

Customize these constants and the physical `TouchCalibration` for your board
and panel. The supplied calibration bounds are (269, 249) through (3829, 3684)
with `Orientation::LeftDown()`. The emulator setup matches those bounds and the
rotated viewport. Keep hardware-specific changes in the display setup section;
the comparison uses the public scroller and window APIs.

See [accelerated scrolling](../../../docs/accelerated_scrolling.md) for cleanup,
mandatory drawing, and explicit opt-in semantics. To compare on a slower bus,
configure the display driver for your hardware and repeat similar drags in both
modes. Record motion through the final pause: a clean stopped image alone does
not establish acceptable ghosting during movement.
