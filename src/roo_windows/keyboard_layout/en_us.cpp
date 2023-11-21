#include "roo_windows/keyboard_layout/en_us.h"

namespace roo_windows {

static const PROGMEM KeySpec kbEngUsRow1keysSmall[] = {
    textKey(2, 'q'), textKey(2, 'w'), textKey(2, 'e'), textKey(2, 'r'),
    textKey(2, 't'), textKey(2, 'y'), textKey(2, 'u'), textKey(2, 'i'),
    textKey(2, 'o'), textKey(2, 'p'),
};

static const PROGMEM KeySpec kbEngUsRow1keysCaps[] = {
    textKey(2, 'Q'), textKey(2, 'W'), textKey(2, 'E'), textKey(2, 'R'),
    textKey(2, 'T'), textKey(2, 'Y'), textKey(2, 'U'), textKey(2, 'I'),
    textKey(2, 'O'), textKey(2, 'P'),
};

static const PROGMEM KeySpec kbEngUsRow2keysSmall[] = {
    textKey(2, 'a'), textKey(2, 's'), textKey(2, 'd'),
    textKey(2, 'f'), textKey(2, 'g'), textKey(2, 'h'),
    textKey(2, 'j'), textKey(2, 'k'), textKey(2, 'l'),
};

static const PROGMEM KeySpec kbEngUsRow2keysCaps[] = {
    textKey(2, 'A'), textKey(2, 'S'), textKey(2, 'D'),
    textKey(2, 'F'), textKey(2, 'G'), textKey(2, 'H'),
    textKey(2, 'J'), textKey(2, 'K'), textKey(2, 'L'),
};

static const PROGMEM KeySpec kbEngUsRow3keysSmall[] = {
    fnKey(3, KeySpec::SHIFT), textKey(2, 'z'), textKey(2, 'x'),
    textKey(2, 'c'),          textKey(2, 'v'), textKey(2, 'b'),
    textKey(2, 'n'),          textKey(2, 'm'), fnKey(3, KeySpec::DEL),
};

static const PROGMEM KeySpec kbEngUsRow3keysCaps[] = {
    fnKey(3, KeySpec::SHIFT), textKey(2, 'Z'), textKey(2, 'X'),
    textKey(2, 'C'),          textKey(2, 'V'), textKey(2, 'B'),
    textKey(2, 'N'),          textKey(2, 'M'), fnKey(3, KeySpec::DEL),
};

static const PROGMEM KeySpec kbEngUsRow4keys[] = {
    pageSwitchKey(3, 0, 4, 1), textKey(2, '@'),          spaceKey(10),
    textKey(2, '.'),           fnKey(3, KeySpec::ENTER),
};

static const PROGMEM KeyboardRowSpec kbEngUsRowsLetters[] = {
    {.start_offset = 0,
     .key_count = 10,
     .keys = kbEngUsRow1keysSmall,
     .keys_caps = kbEngUsRow1keysCaps,
     .pageswitch_key_labels = nullptr},
    {.start_offset = 1,
     .key_count = 9,
     .keys = kbEngUsRow2keysSmall,
     .keys_caps = kbEngUsRow2keysCaps,
     .pageswitch_key_labels = nullptr},
    {.start_offset = 0,
     .key_count = 9,
     .keys = kbEngUsRow3keysSmall,
     .keys_caps = kbEngUsRow3keysCaps,
     .pageswitch_key_labels = nullptr},
    {.start_offset = 0,
     .key_count = 5,
     .keys = kbEngUsRow4keys,
     .keys_caps = kbEngUsRow4keys,
     .pageswitch_key_labels = "?123"}};

static const PROGMEM KeySpec kbEngUsDigitsRow1keys[] = {
    textKey(2, '1'), textKey(2, '2'), textKey(2, '3'), textKey(2, '4'),
    textKey(2, '5'), textKey(2, '6'), textKey(2, '7'), textKey(2, '8'),
    textKey(2, '9'), textKey(2, '0'),
};

static const PROGMEM KeySpec kbEngUsDigitsRow2keys[] = {
    textKey(2, '@'), textKey(2, '#'), textKey(2, '$'), textKey(2, '_'),
    textKey(2, '&'), textKey(2, '-'), textKey(2, '+'), textKey(2, '('),
    textKey(2, ')'), textKey(2, '/'),
};

static const PROGMEM KeySpec kbEngUsDigitsRow3keys[] = {
    pageSwitchKey(3, 0, 3, 2), textKey(2, '*'), textKey(2, '"'),
    textKey(2, '\''),          textKey(2, ':'), textKey(2, ';'),
    textKey(2, '!'),           textKey(2, '?'), fnKey(3, KeySpec::DEL),
};

static const PROGMEM KeySpec kbEngUsDigitsRow4keys[] = {
    pageSwitchKey(3, 0, 3, 0), textKey(2, ','),          spaceKey(10),
    textKey(2, '.'),           fnKey(3, KeySpec::ENTER),
};

static const PROGMEM KeyboardRowSpec kbEngUsRowsDigits[] = {
    {.start_offset = 0,
     .key_count = 10,
     .keys = kbEngUsDigitsRow1keys,
     .keys_caps = kbEngUsDigitsRow1keys,
     .pageswitch_key_labels = nullptr},
    {.start_offset = 0,
     .key_count = 10,
     .keys = kbEngUsDigitsRow2keys,
     .keys_caps = kbEngUsDigitsRow2keys,
     .pageswitch_key_labels = nullptr},
    {.start_offset = 0,
     .key_count = 9,
     .keys = kbEngUsDigitsRow3keys,
     .keys_caps = kbEngUsDigitsRow3keys,
     .pageswitch_key_labels = "=\\<"},
    {.start_offset = 0,
     .key_count = 5,
     .keys = kbEngUsDigitsRow4keys,
     .keys_caps = kbEngUsDigitsRow4keys,
     .pageswitch_key_labels = "ABC"}};

static const PROGMEM KeySpec kbEngUsSymbolsRow1keys[] = {
    textKey(2, '~'), textKey(2, '`'), textKey(2, '|'), textKey(2, u'·'),
    textKey(2, u'μ'), textKey(2, u'π'), textKey(2, u'÷'), textKey(2, u'×'),
    textKey(2, u'¶'), textKey(2, u'Ω'),
};

static const PROGMEM KeySpec kbEngUsSymbolsRow2keys[] = {
    textKey(2, u'£'), textKey(2, u'¢'), textKey(2, u'€'), textKey(2, u'¥'),
    textKey(2, '^'), textKey(2, u'°'), textKey(2, '='), textKey(2, '{'),
    textKey(2, '}'), textKey(2, '\\'),
};

static const PROGMEM KeySpec kbEngUsSymbolsRow3keys[] = {
    pageSwitchKey(3, 0, 4, 1), textKey(2, '%'), textKey(2, u'©'),
    textKey(2, u'®'),          textKey(2, u'™'), textKey(2, u'§'),
    textKey(2, '['),           textKey(2, ']'), fnKey(3, KeySpec::DEL),
};

static const PROGMEM KeySpec kbEngUsSymbolsRow4keys[] = {
    pageSwitchKey(3, 0, 3, 0), textKey(2, '<'),          spaceKey(10),
    textKey(2, '>'),           fnKey(3, KeySpec::ENTER),
};

static const PROGMEM KeyboardRowSpec kbEngUsRowsSymbols[] = {
    {.start_offset = 0,
     .key_count = 10,
     .keys = kbEngUsSymbolsRow1keys,
     .keys_caps = kbEngUsSymbolsRow1keys,
     .pageswitch_key_labels = nullptr},
    {.start_offset = 0,
     .key_count = 10,
     .keys = kbEngUsSymbolsRow2keys,
     .keys_caps = kbEngUsSymbolsRow2keys,
     .pageswitch_key_labels = nullptr},
    {.start_offset = 0,
     .key_count = 9,
     .keys = kbEngUsSymbolsRow3keys,
     .keys_caps = kbEngUsSymbolsRow3keys,
     .pageswitch_key_labels = "?123"},
    {.start_offset = 0,
     .key_count = 5,
     .keys = kbEngUsSymbolsRow4keys,
     .keys_caps = kbEngUsSymbolsRow4keys,
     .pageswitch_key_labels = "ABC"}};

static const PROGMEM KeyboardPageSpec kbEngUsPages[] = {
    {.row_width = 20, .row_count = 4, .rows = kbEngUsRowsLetters},
    {.row_width = 20, .row_count = 4, .rows = kbEngUsRowsDigits},
    {.row_width = 20, .row_count = 4, .rows = kbEngUsRowsSymbols}};

static const PROGMEM KeyboardSpec kbEngUsSpec = {.page_count = 3,
                                                 .pages = kbEngUsPages};

const KeyboardSpec* kbEngUS() { return &kbEngUsSpec; }

}  // namespace roo_windows
