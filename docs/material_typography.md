# Material Typography

`TextStyle` combines a borrowed font with line gap and integer tracking.
Use the compile-time `material2::text_style_*()` and
`material3::text_style_*()` accessors for the standard semantic roles. They
return stable references and retain only the selected font payload.

Font-only `material2::font_*()` convenience accessors live in
`roo_windows/material2/typography.h`. Include that header explicitly;
core no longer supplies these Material 2 names.

`TextLabel`, `StringViewLabel`, and `TextBlock` accept `TextStyle` and retain
only a non-owning pointer. `font()` remains available as a compatibility view;
use `setTextStyle()` to rebind a widget when its shared style changes. A style
must outlive every widget that borrows it. Custom styles should therefore
normally be function-local statics:

```cpp
const TextStyle& custom_body_style() {
  static const TextStyle style(material2::font_body1(), material2::font_body1().metrics().linegap(),
                               0);
  return style;
}
```

Use `style.fontOptions()` for every matching text measurement and draw so the
role's tracking is applied consistently.

The build-covered [Material 3 typography status-screen example](../examples/material3/typography/pool_status/pool_status.ino)
shows the roles adopted by badges, buttons, navigation destinations, tabs,
lists, and app bars. It compiles at all four supported zoom levels.
