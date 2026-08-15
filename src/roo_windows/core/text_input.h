#pragma once

#include <stdint.h>

namespace roo_windows {

class Application;
class ApplicationTextInput;

/// Semantic action requested by a software text-input producer.
enum class TextInputAction : uint8_t {
  kDone,
};

/// Producer endpoint for synchronous software text input.
///
/// An emitter has at most one destination application. Connections and
/// delivery after an application has started must occur on its UI thread.
/// The emitter owns its connection and can safely outlive its destination.
class TextInputEmitter {
 public:
  TextInputEmitter() = default;
  ~TextInputEmitter();

  /// Connects this unused emitter to `destination`'s active text editor.
  void connect(Application& destination);

  /// Removes this emitter's destination. Safe when already disconnected.
  void disconnect();

  /// Returns whether this emitter has a destination application.
  bool isConnected() const { return destination_application_ != nullptr; }

  /// Commits one Unicode scalar to the destination's active editor.
  /// Returns false without side effects when disconnected or inactive.
  bool commitRune(uint32_t rune);

  /// Requests backward deletion in the destination's active editor.
  /// Returns false without side effects when disconnected or inactive.
  bool deleteBackward();

  /// Performs a semantic action in the destination's active editor.
  /// Returns false without side effects when disconnected or inactive.
  bool performAction(TextInputAction action);

  TextInputEmitter(const TextInputEmitter&) = delete;
  TextInputEmitter& operator=(const TextInputEmitter&) = delete;
  TextInputEmitter(TextInputEmitter&&) = delete;
  TextInputEmitter& operator=(TextInputEmitter&&) = delete;

 private:
  friend class Application;
  friend class ApplicationTextInput;

  Application* destination_application_ = nullptr;
  TextInputEmitter* next_emitter_ = nullptr;
};

}  // namespace roo_windows
