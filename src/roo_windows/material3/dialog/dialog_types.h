#pragma once

#include <stdint.h>

#include "roo_backport/string_view.h"

namespace roo_windows::material3 {

/// Identifies the semantic purpose of a basic-dialog action.
enum class DialogActionRole : uint8_t { kAcknowledge, kDismiss, kConfirm };

/// Identifies a non-action reason why a dialog was dismissed.
enum class DialogDismissReason : uint8_t {
  kBack,
  kEscape,
  kCloseButton,
  kProgrammatic,
  kInteractionOwnerDetached,
  kHostDestroyed,
};

/// Reports the synchronous result of attempting to show a dialog.
enum class DialogShowResult : uint8_t {
  kShown,
  kHostBusy,
  kAlreadyPresented,
  kInteractionOwnerUnavailable,
  kSurfaceUnavailable,
  kNavigationUnavailable,
};

/// Describes one fixed-capacity basic-dialog action.
///
/// The descriptor is copied by the dialog, but `label` remains a non-owning
/// view. Its backing storage must outlive the dialog configuration that uses
/// it.
struct DialogActionSpec {
  uint8_t id;
  roo::string_view label;
  DialogActionRole role;
  bool enabled = true;
};

}  // namespace roo_windows::material3
