#include "roo_windows/keyboard_layout/keyboard_layout.h"

#include <algorithm>

#include "roo_display/hal/progmem.h"

namespace roo_windows {
namespace {

bool IsScalar(uint32_t value) {
  return value <= 0x10FFFF && !(value >= 0xD800 && value <= 0xDFFF);
}

}  // namespace

uint32_t KeyboardLayout::read(size_t offset, int bytes) const {
  uint32_t result = 0;
  while (bytes-- > 0) result = (result << 8) | pgm_read_byte(data_ + offset++);
  return result;
}

bool KeyboardLayout::span(size_t offset, size_t length) const {
  return offset >= 8 && offset <= size_ && length <= size_ - offset;
}

KeyboardLayout::Error KeyboardLayout::Open(const uint8_t* data, size_t size,
                                           KeyboardLayout& out) {
  out = KeyboardLayout();
  if (data == nullptr || size < 8 || size > 65535) return Error::kInvalidData;
  KeyboardLayout candidate;
  candidate.data_ = data;
  candidate.size_ = size;
  if (candidate.read(0, 4) != 0x52574B42) return Error::kInvalidData;
  if (candidate.read(4) != 1) return Error::kUnsupportedVersion;
  if (candidate.read(6, 2) != size || !candidate.validate())
    return Error::kInvalidData;
  out = candidate;
  return Error::kOk;
}

// Validate UTF-8 without a RAM copy, including overlong and surrogate
// rejection.
bool KeyboardLayout::validLabel(size_t offset) const {
  if (!span(offset, 1)) return false;
  size_t length = read(offset++);
  if (length == 0 || !span(offset, length)) return false;
  const size_t end = offset + length;
  while (offset < end) {
    uint32_t value = read(offset++);
    if (value < 0x80) continue;
    int count;
    uint32_t minimum;
    if (value >= 0xC2 && value <= 0xDF) {
      count = 1;
      minimum = 0x80;
      value &= 0x1F;
    } else if (value >= 0xE0 && value <= 0xEF) {
      count = 2;
      minimum = 0x800;
      value &= 0x0F;
    } else if (value >= 0xF0 && value <= 0xF4) {
      count = 3;
      minimum = 0x10000;
      value &= 7;
    } else
      return false;
    if (end - offset < static_cast<size_t>(count)) return false;
    while (count-- > 0) {
      uint32_t next = read(offset++);
      if ((next & 0xC0) != 0x80) return false;
      value = (value << 6) | (next & 0x3F);
    }
    if (value < minimum || !IsScalar(value)) return false;
  }
  return true;
}

// Validate all referenced records before allowing unchecked fixed-field reads.
bool KeyboardLayout::validate() const {
  const int pages = read(5);
  if (pages == 0 || !span(8, pages * 4)) return false;
  for (int p = 0; p < pages; ++p) {
    const size_t page = 8 + 4 * p;
    const int width = read(page), rows = read(page + 1);
    const size_t table = read(page + 2, 2);
    if (width == 0 || rows == 0 || !span(table, rows * 4)) return false;
    for (int r = 0; r < rows; ++r) {
      const size_t row = table + 4 * r;
      const int count = read(row);
      const size_t keys = read(row + 2, 2);
      if (count == 0 || read(row + 1) != 0 || !span(keys, count * 11))
        return false;
      int previous_end = 0;
      for (int k = 0; k < count; ++k) {
        const size_t key = keys + 11 * k;
        const int start = read(key), w = read(key + 1), flags = read(key + 2);
        const int function = flags & 7;
        const uint32_t low = read(key + 3, 3), high = read(key + 6, 3);
        const size_t menu = read(key + 9, 2);
        if (w == 0 || start < previous_end || start + w > width ||
            (flags & 0xF0) || function > 5)
          return false;
        previous_end = start + w;
        if (function == 0) {
          if ((flags & 8) || !IsScalar(low) || !IsScalar(high)) return false;
          if (menu != 0) {
            if (!span(menu, 1)) return false;
            const int n = read(menu);
            if (n == 0 || n > 9 || !span(menu + 1, 6 * n)) return false;
            for (int a = 0; a < n; ++a) {
              if (!IsScalar(read(menu + 1 + 6 * a, 3)) ||
                  !IsScalar(read(menu + 4 + 6 * a, 3)))
                return false;
            }
          }
        } else {
          if (menu != 0) return false;
          if (function == 5) {
            if (low >= static_cast<uint32_t>(pages) || !validLabel(high))
              return false;
          } else if (low != 0 || high != 0)
            return false;
        }
      }
    }
  }
  return true;
}

uint8_t KeyboardLayout::pageCount() const { return empty() ? 0 : read(5); }

size_t KeyboardLayout::pageOffset(int page) const {
  return page < 0 || page >= pageCount() ? 0 : 8 + 4 * page;
}

size_t KeyboardLayout::rowOffset(int page, int row) const {
  size_t offset = pageOffset(page);
  return offset == 0 || row < 0 || row >= static_cast<int>(read(offset + 1))
             ? 0
             : read(offset + 2, 2) + 4 * row;
}

size_t KeyboardLayout::keyOffset(int page, int row, int key) const {
  size_t offset = rowOffset(page, row);
  return offset == 0 || key < 0 || key >= static_cast<int>(read(offset))
             ? 0
             : read(offset + 2, 2) + 11 * key;
}

bool KeyboardLayout::readPage(int page, Page& out) const {
  out = Page();
  size_t offset = pageOffset(page);
  if (!offset) return false;
  out.width = read(offset);
  out.row_count = read(offset + 1);
  return true;
}

bool KeyboardLayout::readRow(int page, int row, Row& out) const {
  out = Row();
  size_t offset = rowOffset(page, row);
  if (!offset) return false;
  out.key_count = read(offset);
  return true;
}

bool KeyboardLayout::readKey(int page, int row, int key, Key& out) const {
  out = Key();
  size_t offset = keyOffset(page, row, key);
  if (!offset) return false;
  out.start = read(offset);
  out.width = read(offset + 1);
  out.function = static_cast<Function>(read(offset + 2) & 7);
  out.shape = (read(offset + 2) & 8) ? Shape::kCircle : Shape::kRoundedRect;
  if (out.function == Function::kText) {
    out.character = {read(offset + 3, 3), read(offset + 6, 3)};
    size_t menu = read(offset + 9, 2);
    out.alternative_count = menu == 0 ? 0 : read(menu);
  } else if (out.function == Function::kSwitchPage) {
    out.target_page = read(offset + 3, 3);
    out.label_bytes = read(read(offset + 6, 3));
  }
  return true;
}

bool KeyboardLayout::readAlternative(int page, int row, int key,
                                     int alternative, Character& out) const {
  out = Character();
  size_t offset = keyOffset(page, row, key);
  if (!offset || (read(offset + 2) & 7) != 0) return false;
  size_t menu = read(offset + 9, 2);
  if (!menu || alternative < 0 || alternative >= static_cast<int>(read(menu)))
    return false;
  out = {read(menu + 1 + 6 * alternative, 3),
         read(menu + 4 + 6 * alternative, 3)};
  return true;
}

bool KeyboardLayout::copyLabel(int page, int row, int key, char* buffer,
                               size_t capacity, size_t& length) const {
  length = 0;
  size_t offset = keyOffset(page, row, key);
  if (!offset || (read(offset + 2) & 7) != 5 || buffer == nullptr) return false;
  size_t label = read(offset + 6, 3);
  size_t bytes = read(label);
  if (capacity < bytes) return false;
  for (size_t i = 0; i < bytes; ++i) buffer[i] = read(label + 1 + i);
  length = bytes;
  return true;
}

int KeyboardLayout::findKey(int page, int row, int column) const {
  size_t offset = rowOffset(page, row);
  if (!offset || column < 0 ||
      column >= static_cast<int>(read(pageOffset(page))))
    return -1;
  int first = 0, last = read(offset);
  const size_t keys = read(offset + 2, 2);
  while (first < last) {
    const int mid = first + (last - first) / 2;
    if (read(keys + 11 * mid) <= static_cast<uint32_t>(column))
      first = mid + 1;
    else
      last = mid;
  }
  if (first == 0) return -1;
  offset = keys + 11 * (first - 1);
  return column < static_cast<int>(read(offset) + read(offset + 1)) ? first - 1
                                                                    : -1;
}

KeyboardLayout::KeyRange KeyboardLayout::findKeyRange(int page, int row,
                                                      int first_column,
                                                      int past_column) const {
  size_t offset = rowOffset(page, row);
  if (!offset) return {};
  const int width = read(pageOffset(page));
  first_column = std::max(0, first_column);
  past_column = std::min(width, past_column);
  if (first_column >= past_column) return {};
  const int count = read(offset);
  if (first_column == 0 && past_column == width)
    return {0, static_cast<uint16_t>(count)};
  const size_t keys = read(offset + 2, 2);
  int lo = 0, hi = count;
  while (lo < hi) {
    int mid = lo + (hi - lo) / 2;
    size_t key = keys + 11 * mid;
    if (read(key) + read(key + 1) <= static_cast<uint32_t>(first_column))
      lo = mid + 1;
    else
      hi = mid;
  }
  const int first = lo;
  hi = count;
  while (lo < hi) {
    int mid = lo + (hi - lo) / 2;
    if (read(keys + 11 * mid) < static_cast<uint32_t>(past_column))
      lo = mid + 1;
    else
      hi = mid;
  }
  return {static_cast<uint16_t>(first), static_cast<uint16_t>(lo)};
}

}  // namespace roo_windows
