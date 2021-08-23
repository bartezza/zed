
#include <cstdio>
#include <cassert>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Zed.h"


#define BE16(w) \
    ((((w) & 0xFF) << 8) | (((w) >> 8) & 0xFF))


// NOTE: +1 to include the last \0
// NOTE: version 2-4
const char g_zalphabets[3 * 26 + 1] = {
    "abcdefghijklmnopqrstuvwxyz" // A0
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ" // A1
    " \n0123456789.,!?_#'\"/\\-:()" // A2
};

#define ZCHAR_SHIFT_A1      4
#define ZCHAR_SHIFT_A2      5
#define ZCHAR_A2_10BITS     6
#define ZCHAR_DELETE        8
//#define ZCHAR_TAB           9 // v6
//#define ZCHAR_SENT_SPACE    11 // v6
#define ZCHAR_NEWLINE       13
#define ZCHAR_ESCAPE        27
#define ZCHAR_CURSOR_UP     129
#define ZCHAR_CURSOR_DOWN   130
#define ZCHAR_CURSOR_LEFT   131
#define ZCHAR_CURSOR_RIGHT  132
#define ZCHAR_F1            133
#define ZCHAR_F2            134
#define ZCHAR_F3            135
#define ZCHAR_F4            136
#define ZCHAR_F5            137
#define ZCHAR_F6            138
#define ZCHAR_F7            139
#define ZCHAR_F8            140
#define ZCHAR_F9            141
#define ZCHAR_F10           142
#define ZCHAR_F11           143
#define ZCHAR_F12           144
#define ZCHAR_KEYPAD_0      145
#define ZCHAR_KEYPAD_1      146
#define ZCHAR_KEYPAD_2      147
#define ZCHAR_KEYPAD_3      148
#define ZCHAR_KEYPAD_4      149
#define ZCHAR_KEYPAD_5      150
#define ZCHAR_KEYPAD_6      151
#define ZCHAR_KEYPAD_7      152
#define ZCHAR_KEYPAD_8      153
#define ZCHAR_KEYPAD_9      154
//#define ZCHAR_MENU_CLICK    252 // v6
//#define ZCHAR_DOUBLE_CLICK  253 // v6
#define ZCHAR_SINGLE_CLICK  254


int parseZText(const uint8_t* text, char* out, int outSize) {

    /*
    --first byte-------   --second byte---
    7    6 5 4 3 2  1 0   7 6 5  4 3 2 1 0
    bit  --first--  --second---  --third--
    */

    //  4 13 10  17 17 20  5 18 5  7 5 5.

    // read all z-text and extract the z-chars into a temporary buffer
    int i = 0;
    char buf[256];
    int curBuf = 0;
    while (1) {
        // for (int i = 0; i < 10; i += 2) {
        uint8_t a = (text[i] >> 2) & 0x1F;
        uint8_t b = ((text[i] & 0x03) << 3) | ((text[i + 1] >> 5) & 0x07);
        uint8_t c = text[i + 1] & 0x1F;

        // TODO: deal better with this
        assert(curBuf < sizeof(buf) - 3);

        buf[curBuf++] = a;
        buf[curBuf++] = b;
        buf[curBuf++] = c;

        // printf("a = %u, b = %u, c = %u\n", a, b, c);

        // check end of text
        if (text[i] & 0x80) {
            // printf("EOT\n");
            break;
        }

        // advance
        i += 2;
    }

    // NOTE: the code below is for version >= 3 only
    // parse the z-chars
    int curOut = 0;
    for (i = 0; i < curBuf; ++i) {
        uint8_t curCh = buf[i];
        uint8_t alphabet = 0;
        if (curCh == ZCHAR_SHIFT_A1) {
            if (i >= (curBuf - 1))
                break;
            alphabet = 1;
            // advance
            curCh = buf[++i];
        }
        else if (buf[i] == ZCHAR_SHIFT_A2) {
            if (i >= (curBuf - 1))
                break;
            // check for special char for 10 bits ZSCII
            if (buf[i + 1] == ZCHAR_A2_10BITS) {
                // TODO
                assert(false);
            }
            alphabet = 2;
            // advance
            curCh = buf[++i];
        }
        // parse z-char
        if (curCh == 0x00) {
            out[curOut++] = ' ';
        }
        else if (curCh >= 1 && curCh <= 3) {
            if (i >= (curBuf - 1))
                break;
            // abbreviation
            uint8_t abbrevIndex = 32 * (curCh - 1) + buf[i + 1];
            // TODO: print abbreviation
            printf("TODO: print abbreviation %u\n", abbrevIndex);
            assert(false);
            ++i;
        }
        /*else if (curCh == ZCHAR_DELETE) {
            // for input
        }
        else if (curCh == ZCHAR_NEWLINE) {
            out[curOut++] = '\n';
        }*/
        else if (curCh >= 6) {
            out[curOut++] = g_zalphabets[26 * alphabet + curCh - 6];
        }
    }
    out[curOut] = '\0';
    return curOut;
}



int main(int argc, char** argv) {
    char cwd[MAX_PATH];
    GetCurrentDirectoryA(sizeof(cwd), cwd);

#if 1 // TEST: parse z-text
    const uint8_t temp[] = { 0x11, 0xaa, 0x46, 0x34, 0x16, 0x45, 0x9c, 0xa5 };
    char out[256];
    int ret = parseZText(temp, out, sizeof(out));
    printf("=> '%s'\n", out); // should give "Hello.\n"
    return 1;
#endif

    // const char* filename = "..\\..\\..\\..\\Data\\zork1-r119-s880429.z3";
    const char* filename = "..\\..\\..\\..\\Data\\zork1-r88-s840726.z3";
    FILE* fp = fopen(filename, "rb");
    if (fp == nullptr) {
        printf("ERROR: Could not open '%s'\n", filename);
        return 1;
    }

    ZHeader header;
    fread(&header, sizeof(header), 1, fp);

    assert(header.version <= 3);

    printf("highMemory = %04X, staticMemory = %04X\n", BE16(header.highMemory), BE16(header.staticMemory));
    printf("dict = %04X, objects = %04X, globals = %04X\n", BE16(header.dictionaryAddress), BE16(header.objectsAddress), BE16(header.globalsAddress));
    printf("initPC = %04X\n", BE16(header.initPC));

    unsigned char buf[0x38];
    if (fseek(fp, BE16(header.initPC), SEEK_SET) != 0)
        printf("ERROR: Could not seek to target address\n");

    fread(buf, 1, sizeof(buf), fp);

    for (int i = 0; i < sizeof(buf); ++i) {
        printf("%02X ", buf[i]);
        if (((i + 1) % 8) == 0)
            printf("\n");
    }
    printf("\n");

    fclose(fp);

    printf("%s\n", cwd);
    return 0;
}
