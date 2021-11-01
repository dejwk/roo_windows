#include "roo_windows/keyboard_layout/en_us.h"

namespace roo_windows {

static const PROGMEM KeySpec kbEngUsRow1keys[] = {
    textKey(2, 'q'), textKey(2, 'w'), textKey(2, 'e'), textKey(2, 'r'),
    textKey(2, 't'), textKey(2, 'y'), textKey(2, 'u'), textKey(2, 'i'),
    textKey(2, 'o'), textKey(2, 'p'),
};

static const PROGMEM KeySpec kbEngUsRow2keys[] = {
    textKey(2, 'a'), textKey(2, 's'), textKey(2, 'd'),
    textKey(2, 'f'), textKey(2, 'g'), textKey(2, 'h'),
    textKey(2, 'j'), textKey(2, 'k'), textKey(2, 'l'),
};

static const PROGMEM KeySpec kbEngUsRow3keys[] = {
    fnKey(3, KeySpec::SHIFT), textKey(2, 'z'), textKey(2, 'x'),
    textKey(2, 'c'),          textKey(2, 'v'), textKey(2, 'b'),
    textKey(2, 'n'),          textKey(2, 'm'), fnKey(3, KeySpec::DEL),
};

static const PROGMEM KeySpec kbEngUsRow4keys[] = {
    textKey(2, '@'),
    spaceKey(10),
    textKey(2, '.'),
    fnKey(3, KeySpec::ENTER),
};

static const PROGMEM KeyboardRowSpec kbEngUsRows[] = {
    {.start_offset = 0, .key_count = 10, .keys = kbEngUsRow1keys},
    {.start_offset = 1, .key_count = 9, .keys = kbEngUsRow2keys},
    {.start_offset = 0, .key_count = 9, .keys = kbEngUsRow3keys},
    {.start_offset = 3, .key_count = 4, .keys = kbEngUsRow4keys}};

const PROGMEM KeyboardPageSpec kbEngUSspec = {
    .row_width = 20, .row_count = 4, .rows = kbEngUsRows};

const KeyboardPageSpec* kbEngUS() { return &kbEngUSspec; }

}  // namespace roo_windows
