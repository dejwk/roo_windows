#pragma once

#include <stddef.h>
#include <stdint.h>

namespace roo_windows {

/// Checked, copyable view borrowing immutable RWKB version 2 program-memory
/// data.
class KeyboardLayout {
 public:
  enum class Error : uint8_t { kOk, kInvalidData, kUnsupportedVersion };
  enum class Function : uint8_t {
    kText,
    kDelete,
    kEnter,
    kShift,
    kSpace,
    kSwitchPage
  };
  enum class Shape : uint8_t { kRoundedRect, kCircle };

  struct Page {
    uint8_t width = 0;
    uint8_t row_count = 0;
  };

  struct Row {
    uint8_t key_count = 0;
  };

  struct Character {
    uint32_t lower = 0;
    uint32_t upper = 0;
  };

  struct Key {
    uint8_t start = 0;
    uint8_t width = 0;
    Function function = Function::kText;
    Shape shape = Shape::kRoundedRect;
    Character character;
    uint8_t target_page = 0;
    uint8_t label_bytes = 0;
    uint8_t alternative_count = 0;
    uint8_t alternative_rows = 0;
    uint8_t default_alternative = 0;
  };

  struct KeyRange {
    uint16_t first = 0;
    uint16_t past_last = 0;
  };

  /// Constructs an empty view.
  KeyboardLayout() = default;

  /// Validates readable bytes without allocation; clears out on failure.
  static Error Open(const uint8_t* data, size_t size, KeyboardLayout& out);

  /// Returns whether this view is empty.
  bool empty() const { return data_ == nullptr; }

  /// Returns zero for an empty view.
  uint8_t pageCount() const;

  /// Reads a page, resetting out on an invalid index.
  bool readPage(int page, Page& out) const;

  /// Reads a row, resetting out on invalid indices.
  bool readRow(int page, int row, Row& out) const;

  /// Reads a key, resetting out on invalid indices.
  bool readKey(int page, int row, int key, Key& out) const;

  /// Reads an alternative excluding the base, resetting out on failure.
  bool readAlternative(int page, int row, int key, int alternative,
                       Character& out) const;

  /// Copies a complete switch label without a terminator. On failure sets
  /// length to zero and leaves buffer untouched. Capacity must cover all label
  /// bytes.
  bool copyLabel(int page, int row, int key, char* buffer, size_t capacity,
                 size_t& length) const;

  /// Returns a row-local key index, or -1 for invalid input or empty space.
  int findKey(int page, int row, int column) const;

  /// Returns the key range intersecting a half-open grid-column interval.
  KeyRange findKeyRange(int page, int row, int first_column,
                        int past_column) const;

 private:
  uint32_t read(size_t offset, int bytes = 1) const;
  bool span(size_t offset, size_t length) const;
  size_t pageOffset(int page) const;
  size_t rowOffset(int page, int row) const;
  size_t keyOffset(int page, int row, int key) const;
  bool validate() const;
  bool validLabel(size_t offset) const;

  const uint8_t* data_ = nullptr;
  uint16_t size_ = 0;
};

}  // namespace roo_windows
