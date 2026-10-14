#pragma once

namespace roo_windows {

struct Theme;

/// Returns the framework's default Material 3 theme for compatibility.
/// New applications can select material3::DefaultTheme() explicitly.
const Theme& DefaultTheme();

}  // namespace roo_windows
