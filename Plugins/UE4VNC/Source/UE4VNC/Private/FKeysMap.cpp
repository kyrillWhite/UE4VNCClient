#include "FKeysMap.h"

std::map<FString, uint32> FKeysMap::FKeyToASCII = {
    {"BackSpace",        0xff08},
    {"Tab",              0xff09},
    {"Enter",            0xff0d},
    {"Pause",            0xff13},
    {"CapsLock",         0xffe5},
    {"Escape",           0xff1b},
    {"SpaceBar",         0x0020},
    {"PageUp",           0xff55},
    {"PageDown",         0xff56},
    {"End",              0xff57},
    {"Home",             0xff50},
    {"Left",             0xff51},
    {"Up",               0xff52},
    {"Right",            0xff53},
    {"Down",             0xff54},
    {"Insert",           0xff63},
    {"Delete",           0xffff},
    {"Zero",             0x0030},
    {"One",              0x0031},
    {"Two",              0x0032},
    {"Three",            0x0033},
    {"Four",             0x0034},
    {"Five",             0x0035},
    {"Six",              0x0036},
    {"Seven",            0x0037},
    {"Eight",            0x0038},
    {"Nine",             0x0039},
    {"A",                0x0061},
    {"B",                0x0062},
    {"C",                0x0063},
    {"D",                0x0064},
    {"E",                0x0065},
    {"F",                0x0066},
    {"G",                0x0067},
    {"H",                0x0068},
    {"I",                0x0069},
    {"J",                0x006a},
    {"K",                0x006b},
    {"L",                0x006c},
    {"M",                0x006d},
    {"N",                0x006e},
    {"O",                0x006f},
    {"P",                0x0070},
    {"Q",                0x0071},
    {"R",                0x0072},
    {"S",                0x0073},
    {"T",                0x0074},
    {"U",                0x0075},
    {"V",                0x0076},
    {"W",                0x0077},
    {"X",                0x0078},
    {"Y",                0x0079},
    {"Z",                0x007a},
    {"NumPadZero",       0xffb0},
    {"NumPadOne",        0xffb1},
    {"NumPadTwo",        0xffb2},
    {"NumPadThree",      0xffb3},
    {"NumPadFour",       0xffb4},
    {"NumPadFive",       0xffb5},
    {"NumPadSix",        0xffb6},
    {"NumPadSeven",      0xffb7},
    {"NumPadEight",      0xffb8},
    {"NumPadNine",       0xffb9},
    {"Multiply",         0xffaa},
    {"Add",              0xffab},
    {"Subtract",         0xffad},
    {"Decimal",          0xffae},
    {"Divide",           0xffaf},
    {"F1",               0xffbe},
    {"F2",               0xffbf},
    {"F3",               0xffc0},
    {"F4",               0xffc1},
    {"F5",               0xffc2},
    {"F6",               0xffc3},
    {"F7",               0xffc4},
    {"F8",               0xffc5},
    {"F9",               0xffc6},
    {"F10",              0xffc7},
    {"F11",              0xffc8},
    {"F12",              0xffc9},
    {"NumLock",          0xff7f},
    {"ScrollLock",       0xff14},
    {"LeftShift",        0xffe1},
    {"RightShift",       0xffe2},
    {"LeftControl",      0xffe3},
    {"RightControl",     0xffe4},
    {"LeftAlt",          0xffe9},
    {"RightAlt",         0xffea},
    {"Semicolon",        0x003b},
    {"Equals",           0x003d},
    {"Comma",            0x002c},
    {"Underscore",       0x005f},
    {"Hyphen",           0x002d},
    {"Period",           0x002e},
    {"Slash",            0x002f},
    {"Tilde",            0x007e},
    {"LeftBracket",      0x005b},
    {"Backslash",        0x005c},
    {"RightBracket",     0x005d},
    {"Apostrophe",       0x0027},
    {"Ampersand",        0x0026},
    {"Asterix",          0x002a},
    {"Caret",            0x0afc},
    {"Colon",            0x003a},
    {"Dollar",           0x0024},
    {"Exclamation",      0x00a1},
    {"LeftParantheses",  0x0028},
    {"RightParantheses", 0x0029},
    {"Quote",            0x0022},
    {"Section",          0x00a7}
};

std::map<FString, uint32> FKeysMap::FKeyToShiftedASCII = {
    {"One",              0x0021},
    {"Two",              0x0040},
    {"Three",            0x0023},
    {"Four",             0x0024},
    {"Five",             0x0025},
    {"Six",              0x005E},
    {"Seven",            0x0026},
    {"Eight",            0x002A},
    {"Nine",             0x0028},
    {"Zero",             0x0029},
    {"A",                0x0041},
    {"B",                0x0042},
    {"C",                0x0043},
    {"D",                0x0044},
    {"E",                0x0045},
    {"F",                0x0046},
    {"G",                0x0047},
    {"H",                0x0048},
    {"I",                0x0049},
    {"J",                0x004a},
    {"K",                0x004b},
    {"L",                0x004c},
    {"M",                0x004d},
    {"N",                0x004e},
    {"O",                0x004f},
    {"P",                0x0050},
    {"Q",                0x0051},
    {"R",                0x0052},
    {"S",                0x0053},
    {"T",                0x0054},
    {"U",                0x0055},
    {"V",                0x0056},
    {"W",                0x0057},
    {"X",                0x0058},
    {"Y",                0x0059},
    {"Z",                0x005a},
    {"Semicolon",        0x003a},
    {"Equals",           0x002b},
    {"Comma",            0x003c},
    {"Hyphen",           0x005f},
    {"Period",           0x003e},
    {"Slash",            0x003f},
    {"LeftBracket",      0x007b},
    {"Backslash",        0x007c},
    {"RightBracket",     0x007d},
    {"Apostrophe",       0x0022}
};
