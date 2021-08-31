
#ifndef _H_ZED_
#define _H_ZED_

#include <cstdint>

#pragma pack(push, 1)

// http://inform-fiction.org/zmachine/standards/z1point1/sect11.html
typedef struct ZHeader {
    uint8_t version; // 0
    uint8_t flags1; // 1
    uint8_t _u0[2]; // 2-3
    uint16_t highMemory; // 4-5
    uint16_t initPC; // 6-7
    uint16_t dictionaryAddress; // 8-9
    uint16_t objectsAddress; // A-B
    uint16_t globalsAddress; // C-D
    uint16_t staticMemory; // E-F
    uint16_t flags2; // 10-11
    uint8_t _u1[6]; // 12-17
    uint16_t abbrevAddress; // 18-19
    uint16_t fileLength; // 1A-1B
    uint16_t checksum; // 1C-1D
    uint8_t interpreterNumber; // 1E
    uint8_t interpreterVersion; // 1F
    uint8_t screenHeight; // 20, in lines
    uint8_t screenWidth; // 21, in characters
    uint16_t screenWidthUnits; // 22-23
    uint16_t screenHeightUnits; // 24-25
    uint8_t fontWidth; // 26
    uint8_t fontHeight; // 27
    uint16_t routinesOffset; // 28
    uint16_t stringsOffset; // 2A
    uint8_t defaultBackgroundColor; // 2C
    uint8_t defaultForegroundColor; // 2D
    uint16_t termCharsAddress; // 2E-2F
    uint16_t totalWidthStream3; // 30-31
    uint16_t revNumber; // 32-33
    uint16_t alphabetAddress; // 34-35
    uint16_t extensionAddress; // 36-37
} ZHeader;

// v1-3
typedef struct ZObject_v1 {
    uint8_t attr[4];
    uint8_t parent;
    uint8_t sibling;
    uint8_t child;
    uint16_t props; // byte address
} ZObject_v1;

#pragma pack(pop)

#endif // _H_ZED_
