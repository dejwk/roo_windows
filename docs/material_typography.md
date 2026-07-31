# Material Typography

`TextStyle` combines a borrowed font with line gap and integer tracking.
Use the compile-time `material2::text_style_*()` and
`material3::text_style_*()` accessors for the standard semantic roles. They
return stable references and retain only the selected font payload.

Text widgets still accept and expose fonts until their style-aware migration.
When a widget accepts a `TextStyle`, the style must outlive that widget. Custom
styles should therefore normally be function-local statics:

```cpp
const TextStyle& custom_body_style() {
  static const TextStyle style(font_body1(), font_body1().metrics().linegap(),
                               0);
  return style;
}
```

Use `style.fontOptions()` for every matching text measurement and draw so the
role's tracking is applied consistently.
