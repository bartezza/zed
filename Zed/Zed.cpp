
#include <cstdio>
#include <cassert>
#include <cstdarg>
#include <cstring>
#include "Zed.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h> // for DebugBreak

using namespace Zed;


#define BIT0        0b00000001
#define BIT1        0b00000010
#define BIT2        0b00000100
#define BIT3        0b00001000
#define BIT4        0b00010000
#define BIT5        0b00100000
#define BIT6        0b01000000
#define BIT7        0b10000000
#define BIT8        0b0000000100000000
#define BIT9        0b0000001000000000
#define BIT10       0b0000010000000000
#define BIT11       0b0000100000000000
#define BIT12       0b0001000000000000
#define BIT13       0b0010000000000000
#define BIT14       0b0100000000000000
#define BIT15       0b1000000000000000

#define BE16(w) \
    ((((w) & 0xFF) << 8) | (((w) >> 8) & 0xFF))

#define READ16(_addr) \
    (((uint16_t) m_state.mem[(_addr)] << 8) | ((uint16_t) m_state.mem[(_addr) + 1]))

#define WRITE16(_addr, _val) \
    { m_state.mem[(_addr)] = (uint8_t)(((_val) >> 8) & 0xFF); m_state.mem[(_addr) + 1] = (uint8_t)((_val) & 0xFF); }


#define OPC_FORM_MASK        0b11000000
#define OPC_FORM_VARIABLE    0b11000000
#define OPC_FORM_SHORT       0b10000000

// 2OP
#define OP_CONST_CONST              0b00000000
#define OP_CONST_VAR                0b00100000
#define OP_VAR_CONST                0b01000000
#define OP_VAR_VAR                  0b01100000

#define OPC_JE                      0x01
#define OPC_JL                      0x02
#define OPC_JG                      0x03
#define OPC_DEC_CHK                 0x04
#define OPC_INC_CHK                 0x05
#define OPC_JIN                     0x06
#define OPC_TEST                    0x07
#define OPC_OR                      0x08
#define OPC_AND                     0x09
#define OPC_TEST_ATTR               0x0A
#define OPC_SET_ATTR                0x0B
#define OPC_CLEAR_ATTR              0x0C
#define OPC_STORE                   0x0D
#define OPC_INSERT_OBJ              0x0E
#define OPC_LOADW                   0x0F
#define OPC_LOADB                   0x10
#define OPC_GET_PROP                0x11
#define OPC_GET_PROP_ADDR           0x12
#define OPC_GET_NEXT_PROP           0x13
#define OPC_ADD                     0x14 // h14 (20), h54 (84), h34 (52), h74 (116)
#define OPC_SUB                     0x15
#define OPC_MUL                     0x16
#define OPC_DIV                     0x17
#define OPC_MOD                     0x18
#define OPC_CALL_2S                 0x19
#define OPC_CALL_2N                 0x1A
#define OPC_SET_COLOUR              0x1B
#define OPC_THROW                   0x1C

// 1OP
#define OPC_JZ                      0x00
#define OPC_GET_SIBLING             0x01
#define OPC_GET_CHILD               0x02
#define OPC_GET_PARENT              0x03
#define OPC_GET_PROP_LEN            0x04
#define OPC_INC                     0x05
#define OPC_DEC                     0x06
#define OPC_PRINT_ADDR              0x07
#define OPC_CALL_1S                 0x08
#define OPC_REMOVE_OBJ              0x09
#define OPC_PRINT_OBJ               0x0A
#define OPC_RET                     0x0B
#define OPC_JUMP                    0x0C
#define OPC_PRINT_PADDR             0x0D
#define OPC_LOAD                    0x0E
#define OPC_NOT_1OP                 0x0F

// 0OP
#define OPC_RTRUE                   0x00
#define OPC_RFALSE                  0x01
#define OPC_PRINT                   0x02
#define OPC_PRINT_RET               0x03
#define OPC_NOP                     0x04
#define OPC_SAVE                    0x05
#define OPC_RESTORE                 0x06
#define OPC_RESTART                 0x07
#define OPC_RET_POPPED              0x08
#define OPC_POP                     0x09
#define OPC_QUIT                    0x0A
#define OPC_NEWLINE                 0x0B
#define OPC_SHOW_STATUS             0x0C
#define OPC_VERIFY                  0x0D
// #define OPC_EXTENDED                0x0E
#define OPC_PIRACY                  0x0F

// VAR
#define OPC_CALL                    0x00
#define OPC_STOREW                  0x01
#define OPC_STOREB                  0x02
#define OPC_PUT_PROP                0x03
#define OPC_SREAD                   0x04
#define OPC_PRINT_CHAR              0x05
#define OPC_PRINT_NUM               0x06
#define OPC_RANDOM                  0x07
#define OPC_PUSH                    0x08
#define OPC_PULL                    0x09 // 2OP
#define OPC_SPLIT_WINDOW            0x0A
#define OPC_SET_WINDOW              0x0B
#define OPC_CALL_VS2                0x0C
#define OPC_ERASE_WINDOW            0x0D
#define OPC_ERASE_LINE              0x0E
#define OPC_SET_CURSOR              0x0F
#define OPC_GET_CURSOR              0x10
#define OPC_SET_TEXT_STYLE          0x11
#define OPC_BUFFER_MODE             0x12
#define OPC_OUTPUT_STREAM           0x13
#define OPC_INPUT_STREAM            0x14
#define OPC_SOUND_EFFECT            0x15
#define OPC_READ_CHAR               0x16
#define OPC_SCAN_TABLE              0x17
#define OPC_NOT_VAR                 0x18
#define OPC_CALL_VN                 0x19
#define OPC_CALL_VN2                0x1A
#define OPC_TOKENISE                0x1B
#define OPC_ENCODE_TEXT             0x1C
#define OPC_COPY_TABLE              0x1D
#define OPC_PRINT_TABLE             0x1E
#define OPC_CHECK_ARG_COUNT         0x1F

// other (full) opcodes
// VAR
#define OPC_EXTENDED                0xBE

#define OPTYPE_LARGE_CONST          0x00
#define OPTYPE_SMALL_CONST          0x01
#define OPTYPE_VAR                  0x02
#define OPTYPE_OMITTED              0x03

// v1-3
#define packedAddressToByte(_addr) \
        (2 * _addr)


#define OPF_STORE               BIT0
#define OPF_BRANCH              BIT1
#define OPF_OBJ1                BIT2  // operand1 is object
#define OPF_OBJ2                BIT3  // operand2 is object

typedef struct ZOpcodeInfo {
    const char* name;
    uint8_t flags;
} ZOpcodeInfo;

ZOpcodeInfo g_opcodes2OP[] = {
    {"INVALID", 0},
    {"JE", OPF_BRANCH}, // 0x01
    {"JL", OPF_BRANCH},
    {"JG", OPF_BRANCH},
    {"DEC_CHK", OPF_BRANCH},
    {"INC_CHK", OPF_BRANCH}, // 0x05
    {"JIN", OPF_BRANCH | OPF_OBJ1 | OPF_OBJ2},
    {"TEST", OPF_BRANCH},
    {"OR", OPF_STORE},
    {"AND", OPF_STORE},
    {"TEST_ATTR", OPF_BRANCH | OPF_OBJ1}, // 0x0A
    {"SET_ATTR", OPF_OBJ1},
    {"CLEAR_ATTR", OPF_OBJ1},
    {"STORE", 0},
    {"INSERT_OBJ", OPF_OBJ1},
    {"LOADW", OPF_STORE},
    {"LOADB", OPF_STORE}, // 0x10
    {"GET_PROP", OPF_STORE | OPF_OBJ1},
    {"GET_PROP_ADDR", OPF_STORE | OPF_OBJ1},
    {"GET_NEXT_PROP", OPF_STORE | OPF_OBJ1},
    {"ADD", OPF_STORE},
    {"SUB", OPF_STORE}, // 0x15
    {"MUL", OPF_STORE},
    {"DIV", OPF_STORE},
    {"MOD", OPF_STORE},
    {"CALL_2S", OPF_STORE},
    {"CALL_2N", 0}, // 0x1A
    {"SET_COLOUR", 0},
    {"THROW", 0}
};

ZOpcodeInfo g_opcodes1OP[] = {
    {"JS", OPF_BRANCH}, // 0x00
    {"GET_SIBLING", OPF_BRANCH | OPF_STORE | OPF_OBJ1},
    {"GET_CHILD", OPF_BRANCH | OPF_STORE | OPF_OBJ1},
    {"GET_PARENT", OPF_STORE | OPF_OBJ1},
    {"GET_PROP_LEN", OPF_STORE | OPF_OBJ1},
    {"INC", 0}, // 0x05
    {"DEC", 0},
    {"PRINT_ADDR", 0},
    {"CALL_1S", OPF_STORE},
    {"REMOVE_OBJ", OPF_OBJ1},
    {"PRINT_OBJ", OPF_OBJ1}, // 0x0A
    {"RET", 0},
    {"JUMP", 0},
    {"PRINT_PADDR", 0},
    {"LOAD", OPF_STORE},
    {"NOT", OPF_STORE} // 0x0F v1-4, CALL_1N v5
};

ZOpcodeInfo g_opcodes0OP[] = {
    {"RTRUE", 0}, // 0x00
    {"RFALSE", 0},
    {"PRINT", 0},
    {"PRINT_RET", 0},
    {"NOP", 0}, // only v1
    {"SAVE", OPF_BRANCH}, // 0x05, v1-4, then illegal
    {"RESTORE", OPF_BRANCH}, // v1-4, then illegal
    {"RESTART", 0},
    {"RET_POPPED", 0},
    {"POP", 0}, // v1-4, CATCH v5-6
    {"QUIT", 0}, // 0x0A
    {"NEW_LINE", 0},
    {"SHOW_STATUS", 0}, // v3, then illegal
    {"VERIFY", OPF_BRANCH}, // v3+
    {"EXTENDED", 0}, // v5+
    {"PIRACY", OPF_BRANCH} // 0x0F, v5 only
};

ZOpcodeInfo g_opcodesVAR[] = {
    {"CALL", OPF_STORE}, // 0x00, CALL_VS v4+
    {"STOREW", 0},
    {"STOREB", 0},
    {"PUT_PROP", 0},
    {"SREAD", 0}, // v1-4, AREAD v5+
    {"PRINT_CHAR", 0}, // 0x05
    {"PRINT_NUM", 0},
    {"RANDOM", OPF_STORE},
    {"PUSH", 0},
    {"PULL", 0},
    {"SPLIT_WINDOW", 0}, // 0x0A, v3+
    {"SET_WINDOW", 0}, // v3+
    {"CALL_VS2", OPF_STORE}, // v4+
    {"ERASE_WINDOW", 0}, // v4+
    {"ERASE_LINE", 0}, // v4/6
    {"SET_CURSOR", 0}, // v4
    {"GET_CURSOR", 0}, // 0x10, v4/6
    {"SET_TEXT_STYLE", 0}, // v4
    {"BUFFER_MODE", 0}, // v4
    {"OUTPUT_STREAM", 0}, // v3
    {"INPUT_STREAM", 0}, // v3
    {"SOUND_EFFECT", 0}, // 0x15, v3
    {"READ_CHAR", OPF_STORE}, // v4
    {"SCAN_TABLE", OPF_STORE | OPF_BRANCH}, // v4
    {"NOT", OPF_STORE}, // v5
    {"CALL_VN", 0}, // v5
    {"CALL_VN2", 0}, // 0x1A, v5
    {"TOKENISE", 0}, // v5
    {"ENCODE_TEXT", 0}, // v5
    {"COPY_TABLE", 0}, // v5
    {"PRINT_TABLE", 0}, // v5
    {"CHECK_ARG_COUNT", OPF_BRANCH} // 0x1F, v5
};

// TODO: ext table of opcodes


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


void TextBuffer::reset() {
    ptr = 0;
    buf[0] = '\0';
}

void TextBuffer::printf(const char* str, ...) {
    va_list args;
    va_start(args, str);
    vsnprintf(&buf[ptr], sizeof(buf) - ptr, str, args);
    va_end(args);
    ptr = strlen(buf);
}

void TextBuffer::copy(const char* str) {
    size_t len = strlen(str);
    if ((ptr + len) < (sizeof(buf) - 1)) {
        memcpy(&buf[ptr], str, len + 1);
        ptr += len;
    }
}

void TextBuffer::copy(char ch) {
    if (ptr < (sizeof(buf) - 1)) {
        buf[ptr++] = ch;
        buf[ptr] = '\0';
    }
}


void ZMachine::copyStory(const uint8_t* mem, size_t memSize) {
    // alloc and copy memory
    m_state.mem.resize(memSize);
    memcpy(&m_state.mem[0], mem, memSize);

    // setup interpreter-based flags
    // status bar not supported
    ((ZHeader *)&m_state.mem[0])->flags1 |= BIT4;
    // Tandy bit
    ((ZHeader *)&m_state.mem[0])->flags1 &= ~BIT3;

    // reset state
    reset();
}

void ZMachine::reset() {
    m_state.reset();

    debugPrintf("highMemory = %04X, staticMemory = %04X\n", BE16(m_state.header->highMemory), BE16(m_state.header->staticMemory));
    debugPrintf("dict = %04X, objects = %04X, globals = %04X\n", BE16(m_state.header->dictionaryAddress), BE16(m_state.header->objectsAddress), BE16(m_state.header->globalsAddress));
    debugPrintf("initPC = %04X\n", BE16(m_state.header->initPC));
}

void ZMachineState::reset() {
    // set header
    header = (const ZHeader*)&mem[0];
    // initialize pc
    pc = BE16(header->initPC);
    // initialize stack
    stackMem.resize(64 * 1024);
    // create a fake initial call frame
    callBase = 0;
    stackMem[CALL_OLD_BASE_HI] = 0xFFFF; // sentinel value to detect invalid RETs from main routine
    stackMem[CALL_OLD_BASE_LO] = 0xFFFF;
    stackMem[CALL_RET_HI] = 0xFFFF;
    stackMem[CALL_RET_LO] = 0xFFFF;
    stackMem[CALL_STORE_VAR] = 0;
    stackMem[CALL_NUM_LOCALS] = 0;
    bp = sp = CALL_LOCALS;
    // CHECK
    assert(header->version <= 3);
}

size_t ZMachine::parseZCharacters(const uint8_t *buf, size_t numBuf, char* out, size_t outSize, bool enableAbbrev) {
    // TODO: change this to output ZSCII chars (not assuming ASCII as output)

    // NOTE: the code below is for version >= 3 only
    // parse the z-chars
    size_t curOut = 0;
    for (size_t i = 0; i < numBuf; ++i) {
        uint8_t curCh = buf[i];
        uint8_t alphabet = 0;
        if (curCh == ZCHAR_SHIFT_A1) {
            if (i >= (numBuf - 1))
                break;
            alphabet = 1;
            // advance
            curCh = buf[++i];
        }
        else if (buf[i] == ZCHAR_SHIFT_A2) {
            if (i >= (numBuf - 1))
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
        else if (curCh >= 1 && curCh <= 3) { // v3
            if (i >= (numBuf - 1))
                break;
            // abbreviation index
            uint8_t abbrevIndex = 32 * (curCh - 1) + buf[i + 1];
            if (enableAbbrev) {
                // read corresponding abbreviation location
                uint32_t addr = BE16(m_state.header->abbrevAddress) + abbrevIndex * 2;
                // NOTE: this is a word address (so we need to multiply it by 2)
                uint32_t addr2 = READ16(addr) * 2;
                // printf("[DEBUG] abbrevIndex = %u, addr = %04X, addr2 = %04X\n", abbrevIndex, addr, addr2);
                // print abbreviation at location
                size_t ret = parseZText(&m_state.mem[addr2], &out[curOut], outSize - curOut, nullptr, false);
                // advance curOut according to the abbreviation
                curOut += ret;
            }
            else {
                errorPrintf("Requested abbreviation %u with abbreviations disabled", abbrevIndex);
            }
            ++i;
        }
        else if (curCh <= 5) {
            debugPrintf("TODO: handle shift locking (curCh = %02X)\n", curCh);
            // assert(false);
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

size_t ZMachine::parseZText(const uint8_t* text, char* out, size_t outSize, size_t* outTextBytesRead, bool enableAbbrev) {

    /*
    --first byte-------   --second byte---
    7    6 5 4 3 2  1 0   7 6 5  4 3 2 1 0
    bit  --first--  --second---  --third--
    */

    //  4 13 10  17 17 20  5 18 5  7 5 5.

    // read all z-text and extract the z-chars into a temporary buffer
    size_t i = 0;
    uint8_t buf[4096];
    size_t curBuf = 0;
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

    // return number of bytes read
    if (outTextBytesRead)
        *outTextBytesRead = i + 2; // skip last 2 bytes

    return parseZCharacters(buf, curBuf, out, outSize, enableAbbrev);
}

// NOTE: the ZSCII should go to 1023, but in practice 256-1023 are not used
char zsciiToAscii(uint8_t ch) {
    // TODO: v6
    if ((ch == 0) || (ch >= 32 && ch <= 126)) {
        return (char)ch;
    }
    else if (ch == 13) {
        return '\n';
    }
    else if (ch >= 155 && ch <= 251) {
        // TODO: extra characters
        return '?';
    }
    else {
        assert(false);
        return '?';
    }
}

void ZMachine::debugPrintf(const char* fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    debugPrint(buffer);
}

void ZMachine::debugPrint(const char* text) {
    if (debugPrintCallback)
        debugPrintCallback(text);
    else
        fputs(text, stdout);
}

void ZMachine::errorPrintf(const char* fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    errorPrint(buffer);
}

void ZMachine::errorPrint(const char* text) {
    if (errorPrintCallback)
        errorPrintCallback(text);
    else {
        fputs("[ERROR] ", stdout);
        // fputs(text, stdout);
        puts(text);
    }
}

void ZMachine::gamePrintf(const char* fmt, ...) {
    char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    gamePrint(buffer);
}

void ZMachine::gamePrint(const char* text) {
    if (gamePrintCallback)
        gamePrintCallback(text);
    else {
        fputs(text, stdout);
    }
}

void ZMachine::debugZText(const uint8_t* text) {
    char out[1024];
    parseZText(text, out, sizeof(out), nullptr);
    debugPrint(out);
}

// TODO: remove this?
void ZMachine::debugPrintVarName(TextBuffer& tb, uint8_t var) {
    if (var == 0)
        tb.copy(" (SP)");
    else if (var <= 0x0F)
        tb.printf(" L%02X", var - 1);
    else
        tb.printf(" G%02X", var - 0x10);
}

uint16_t ZMachine::getVar(uint8_t idx) {
    // check which var
    if (idx == 0) {
        // pop from stack
        assert(m_state.sp > 0);
        return m_state.stackMem[--m_state.sp];
    }
    else if (idx <= 0x0F) {
        // local variable
        const uint16_t* locals = &m_state.stackMem[m_state.callBase + CALL_LOCALS];
        uint16_t numLocals = m_state.stackMem[m_state.callBase + CALL_NUM_LOCALS];
        assert((idx - 1) < numLocals); // check number of locals
        return locals[idx - 1];
    }
    else {
        // globals
        uint32_t addr = BE16(m_state.header->globalsAddress) + (idx - 0x10) * 2;
        return READ16(addr);
    }
}

void ZMachine::setVar(uint8_t idx, uint16_t value) {
    // check which var
    if (idx == 0) {
        // push to stack
        assert(m_state.sp < (m_state.stackMem.size() - 1));
        m_state.stackMem[m_state.sp++] = value;
    }
    else if (idx <= 0x0F) {
        // local variable
        uint16_t* locals = &m_state.stackMem[m_state.callBase + CALL_LOCALS];
        uint16_t numLocals = m_state.stackMem[m_state.callBase + CALL_NUM_LOCALS];
        assert((idx - 1) < numLocals); // check number of locals
        locals[idx - 1] = value;
    }
    else {
        // globals
        uint32_t addr = BE16(m_state.header->globalsAddress) + (idx - 0x10) * 2;
        WRITE16(addr, value);
    }
}

bool ZMachine::execCall() {
    // read where to store the result
    uint8_t storeVar = m_state.mem[m_temp.curPc++];
    // execute routine
    // save call frame ptr
    uint32_t callBase = m_state.sp;
    // store old call base
    m_state.stackMem[m_state.sp++] = (uint16_t)((m_state.callBase >> 16) & 0xFFFF);
    m_state.stackMem[m_state.sp++] = (uint16_t)(m_state.callBase & 0xFFFF);
    // get new location
    uint32_t newPc = packedAddressToByte(m_temp.opVals[0]); // packed
    assert(newPc < m_state.mem.size());
    // store ret location, pointer to byte after the CALL
    m_state.stackMem[m_state.sp++] = (uint16_t)((m_temp.curPc >> 16) & 0xFFFF);
    m_state.stackMem[m_state.sp++] = (uint16_t)(m_temp.curPc & 0xFFFF);
    // store return variable
    m_state.stackMem[m_state.sp++] = storeVar;
    // go to new location
    m_temp.curPc = newPc;
    // read number of local variables
    uint8_t numLocals = m_state.mem[m_temp.curPc++];
    if (numLocals >= 16) {
        // should not happen by specs
        errorPrint("Too many local variables");
        return false;
    }
    m_state.stackMem[m_state.sp++] = numLocals;
    // read the initial values of the local variables, v1-4
    for (uint8_t i = 0; i < numLocals; ++i) {
        m_state.stackMem[m_state.sp + i] = READ16(m_temp.curPc);
        m_temp.curPc += 2;
    }
    // overwrite them with function arguments
    for (uint8_t i = 0; i < (m_temp.numOps - 1) && i < numLocals; ++i) {
        m_state.stackMem[m_state.sp + i] = m_temp.opVals[i + 1];
    }
    // advance sp
    m_state.sp += numLocals;
    // store callbase and bp
    m_state.callBase = callBase;
    m_state.bp = m_state.sp;

    // DEBUG: print call frame
    const uint16_t* cb = &m_state.stackMem[m_state.callBase];
    debugPrintf("Call frame: ret = %04X, numLocals = %u\n",
        (uint32_t)(cb[CALL_RET_HI] << 16) | (uint32_t)(cb[CALL_RET_LO]),
        cb[CALL_NUM_LOCALS]);
    for (uint16_t i = 0; i < cb[CALL_NUM_LOCALS]; ++i)
        debugPrintf("%u) %04X\n", i, cb[CALL_LOCALS + i]);
    return true;
}

bool ZMachine::execRet(uint16_t val) {
    // check if we are 
    // get store variable
    const uint16_t* cb = &m_state.stackMem[m_state.callBase];
    uint8_t st = (uint8_t)cb[CALL_STORE_VAR];
    // get where to return to
    m_temp.curPc = (uint32_t)(cb[CALL_RET_HI] << 16) | (uint32_t)(cb[CALL_RET_LO]);
    // get old call base
    uint32_t oldCallBase = (uint32_t)(cb[CALL_OLD_BASE_HI] << 16) | (uint32_t)(cb[CALL_OLD_BASE_LO]);
    // check against sentinel value
    if (oldCallBase == 0xFFFFFFFF) {
        errorPrint("Trying to RET from the main routine");
        return false;
    }
    // set stack to old location, before the CALL
    m_state.sp = m_state.callBase;
    // reset callbase and also old bp
    m_state.callBase = oldCallBase;
    const uint16_t* oldCb = &m_state.stackMem[oldCallBase];
    m_state.bp = oldCallBase + CALL_LOCALS + oldCb[CALL_NUM_LOCALS];
    // store value
    // NOTE: this is done after having reset the stack since st could be 0,
    // meaning to push the return value onto the stack
    setVar(st, val);
    return true;
};

void ZMachine::readBranchInfo(uint32_t &curPc, bool &jumpCond, uint32_t &dest) const {
    // read branch info
    // bit7 == 0 => jump if false, else jump if true
    // if bit6 == 1 => offset in bit5-0
    // if bit6 == 0 => offset is 14bit signed bit5-0 first byte + all bits of next byte
    uint16_t _b = (uint16_t)m_state.mem[curPc++];
    jumpCond = (_b & BIT7) != 0;
    // read destination of jump
    uint16_t offset;
    if (_b & BIT6) {
        // short 6bit jump
        offset = _b & 0b00111111;
    }
    else {
        // final offset is 14bit signed integer
        offset = ((_b & 0b00111111) << 8) | (uint16_t)m_state.mem[curPc++];
        // make sure sign is preserved
        if (offset & BIT13)
            offset |= BIT14 | BIT15;
    }
    // check offset for special cases
    if (offset <= 1) {
        // this corresponds to returning true or false
        dest = offset;
    }
    else {
        // pass the actual eventual destination
        dest = (int32_t)curPc + (int16_t)offset - 2;
    }
}

void ZMachine::disasmBranch(TextBuffer& tb, uint32_t& curPc) {
    // read info
    bool jumpCond;
    uint32_t dest;
    readBranchInfo(curPc, jumpCond, dest);
    // print jump condition
    tb.copy(jumpCond ? " [TRUE]" : " [FALSE]");
    // print dest
    if (dest == 0) tb.copy(" RFALSE");
    else if (dest == 1) tb.copy(" RTRUE");
    else tb.printf(" %04X", dest);
}

bool ZMachine::execBranch(bool condition) {
    // read branch info
    bool jumpCond;
    uint32_t dest;
    readBranchInfo(m_temp.curPc, jumpCond, dest);
    // check condition
    if (!((condition) ^ jumpCond)) {
        // if 0 means RFALSE, 1 means RTRUE
        if (dest == 0) {
            return execRet(0);
        }
        else if (dest == 1) {
            return execRet(1);
        }
        else {
            // jump
            m_temp.curPc = dest;
        }
    }
    return true;
};

/*
auto dumpMem = [&](uint32_t _pc, uint32_t _size) {
    for (uint32_t i = 0; i < _size; ++i) {
        printf("%02X ", mem[_pc + i]);
        if (((i + 1) % 8) == 0)
            printf("\n");
    }
    printf("\n");
};
*/

// v1-3
ZObject_v1* ZMachine::getObject(uint16_t objIndex) {
    // v1-3
    assert(m_state.header->version <= 3);
    // TODO: objIndex != 0
    uint16_t baseObjs = BE16(m_state.header->objectsAddress);
    ZObject_v1* obj = (ZObject_v1*)&m_state.mem[baseObjs + 31 * 2 + (objIndex - 1) * sizeof(ZObject_v1)];
    return obj;
}

// v1-3
uint16_t ZMachine::getPropertyDefault(uint16_t propIndex) {
    // v1-3
    assert(propIndex < 32);
    uint16_t baseObjs = BE16(m_state.header->objectsAddress);
    // NOTE: properties starts from 1!!!
    return READ16(baseObjs + (propIndex - 1) * 2);
}

void ZMachine::debugPrintObjName(TextBuffer& tb, const ZObject_v1* obj) {
    // debugPrint(" '"); debugZText(&m_state.mem[BE16(obj->props) + 1]); debugPrint("'");
    tb.copy(" '");
    const uint8_t* text = &m_state.mem[BE16(obj->props) + 1];
    parseZText(text, &tb.buf[tb.ptr], sizeof(tb.buf) - tb.ptr, nullptr);
    tb.ptr = strlen(tb.buf);
    tb.copy("'");
}

bool ZMachine::exec0OPInstruction(uint8_t opcode) {
    switch (opcode & 0b00001111) {
    case OPC_RTRUE:
        return execRet(1);
    case OPC_RFALSE:
        return execRet(0);
    case OPC_PRINT: {
        char out[1024];
        size_t bytesRead = 0;
        size_t curOut = parseZText(&m_state.mem[m_temp.curPc], out, sizeof(out), &bytesRead);
        m_temp.curPc += (uint32_t) bytesRead;
        gamePrint(out);
        break;
    }
    case OPC_NEWLINE: {
        gamePrint("\n");
        break;
    }
    default:
        errorPrintf("Short 0OP opcode %02X (%s) not implemented yet (op = %u)",
            opcode & 0b00001111, g_opcodes0OP[opcode & 0b00001111].name, opcode);
        return false;
    }
    return true;
}

bool ZMachine::exec1OPInstruction(uint8_t opcode) {
    const uint16_t val = m_temp.opVals[0];
    switch (opcode & 0b00001111) {
    case OPC_JZ:
        execBranch(val == 0);
        break;
    case OPC_INC: {
        // inc (variable)
        // NOTE: val1 is set as a small constant, so we need to get the variable value here.
        // need to get variable
        int16_t curVal = (int16_t)getVar((uint8_t)val);
        // increment
        ++curVal;
        // set
        setVar((uint8_t)val, (uint16_t)curVal);
        break;
    }
    case OPC_DEC: {
        // dec (variable)
        // NOTE: val1 is set as a small constant, so we need to get the variable value here.
        // need to get variable
        int16_t curVal = (int16_t)getVar((uint8_t)val);
        // decrement
        --curVal;
        // set
        setVar((uint8_t)val, (uint16_t)curVal);
        break;
    }
    case OPC_GET_SIBLING: {
        // get_sibling object -> (result) ?(label)
        // Get next object in tree, branching if this exists, i.e.is not 0.
        uint8_t objId = (uint8_t)val;
        ZObject_v1* obj = getObject(objId);
        //debugPrintObjName(obj);
        setVar(m_state.mem[m_temp.curPc++], obj->sibling);
        execBranch(obj->sibling != 0);
        break;
    }
    case OPC_GET_CHILD: {
        // get_child object -> (result) ?(label)
        // Get first object contained in given object, branching if this exists,
        // i.e. is not nothing (i.e., is not 0).
        uint8_t objId = (uint8_t)val;
        ZObject_v1* obj = getObject(objId);
        //debugPrintObjName(obj);
        setVar(m_state.mem[m_temp.curPc++], obj->child);
        execBranch(obj->child != 0);
        break;
    }
    case OPC_GET_PARENT: {
        // get_parent object -> (result)
        uint8_t objId = (uint8_t)val;
        ZObject_v1* obj = getObject(objId);
        //debugPrintObjName(obj);
        setVar(m_state.mem[m_temp.curPc++], obj->parent);
        break;
    }
    case OPC_PRINT_OBJ: {
        // print_obj object
        uint8_t objId = (uint8_t)val;
        ZObject_v1* obj = getObject(objId);
        char out[1024];
        parseZText(&m_state.mem[BE16(obj->props) + 1], out, sizeof(out), nullptr);
        gamePrint(out);
        break;
    }
    case OPC_RET: {
        execRet(val);
        break;
    }
    case OPC_JUMP: {
        // branch of 16bits signed offset
        m_temp.curPc = (uint32_t)((int32_t)m_temp.curPc + (int16_t)val - 2);
        break;
    }
    case OPC_PRINT_PADDR: {
        uint32_t addr = packedAddressToByte(val); // packed
        char out[1024];
        parseZText(&m_state.mem[addr], out, sizeof(out), nullptr);
        gamePrint(out);
        // TEMP
        return false;
        break;
    }
    default:
        errorPrintf("Short 1OP opcode %02X (%s) not implemented yet (op = %u)",
            opcode & 0b00001111, g_opcodes1OP[opcode & 0b00001111].name, opcode);
        return false;
    }
    return true;
}

bool ZMachine::exec2OPInstruction(uint8_t opcode) {
    // parse and execute opcode
    const uint16_t val1 = m_temp.opVals[0];
    const uint16_t val2 = m_temp.opVals[1];
    switch (opcode & 0b00011111) {
    case OPC_JE:
        // je a b c d ?(label)
        // Jump if a is equal to any of the subsequent operands.
        // (Thus @je a never jumps and @je a b jumps if a = b.)
        // je with just 1 operand is not permitted.
        assert(m_temp.numOps > 1);
        if (m_temp.numOps == 2)
            execBranch(m_temp.opVals[0] == m_temp.opVals[1]);
        else if (m_temp.numOps == 3)
            execBranch((m_temp.opVals[0] == m_temp.opVals[1]) || (m_temp.opVals[0] == m_temp.opVals[2]));
        else
            execBranch((m_temp.opVals[0] == m_temp.opVals[1]) || (m_temp.opVals[0] == m_temp.opVals[2]) || (m_temp.opVals[0] == m_temp.opVals[3]));
        break;
    case OPC_JL: {
        // jl a b ? (label)
        // Jump if a < b (using a signed 16 - bit comparison).
        execBranch((int16_t)val1 < (int16_t)val2);
        break;
    }
    case OPC_JG: {
        // jg a b ?(label)
        // Jump if a > b (using a signed 16-bit comparison).
        execBranch((int16_t)val1 > (int16_t)val2);
        break;
    }
    case OPC_DEC_CHK: {
        // dec_chk (variable) value ?(label)
        // Decrement variable, and branch if it is now less than the given value.
        // NOTE: this opcode could be called both as 2OP and as VAR. when called as 2OP, the val1 is
        // set as a small constant, so we need to get the variable value here.
        // NOTE: not re-getting var here if already gotten since the getVar could have the side effect
        // of changing the stack (as getVar(0) means pop value from stack)
        int16_t curVal;
        if (m_temp.opTypes[0] == OPTYPE_VAR) {
            // value already gotten
            curVal = (int16_t)m_temp.opVals[0];
        }
        else {
            // need to get variable
            curVal = (int16_t)getVar((uint8_t)m_temp.opVals[0]);
        }
        --curVal;
        setVar(m_temp.opVars[0], (uint16_t)curVal);
        execBranch(curVal < (int16_t)val2);
        break;
    }
    case OPC_INC_CHK: {
        // inc_chk(variable) value ? (label)
        // Increment variable, and branch if now greater than value.
        // NOTE: this opcode could be called both as 2OP and as VAR. when called as 2OP, the val1 is
        // set as a small constant, so we need to get the variable value here.
        // NOTE: not re-getting var here if already gotten since the getVar could have the side effect
        // of changing the stack (as getVar(0) means pop value from stack)
        int16_t curVal;
        if (m_temp.opTypes[0] == OPTYPE_VAR) {
            // value already gotten
            curVal = (int16_t)m_temp.opVals[0];
        }
        else {
            // need to get variable
            curVal = (int16_t)getVar((uint8_t)m_temp.opVals[0]);
        }
        ++curVal;
        setVar(m_temp.opVars[0], (uint16_t)curVal);
        execBranch(curVal > (int16_t)val2);
        break;
    }
    case OPC_JIN: {
        // jin obj1 obj2 ?(label)
        const ZObject_v1* obj1 = getObject(val1);
        const ZObject_v1* obj2 = getObject(val2);
        // DEBUG: print object name
        //debugPrintObjName(obj1);
        //debugPrintObjName(obj2);
        // jump if obj1 is direct child of obj2
        execBranch(obj1->parent == val2);
        break;
    }
    case OPC_OR:
        setVar(m_state.mem[m_temp.curPc++], val1 | val2);
        break;
    case OPC_AND:
        setVar(m_state.mem[m_temp.curPc++], val1 & val2);
        break;
    case OPC_TEST_ATTR: {
        assert(val2 < 32);
        const ZObject_v1* obj = getObject(val1);
        // DEBUG: print object name
        //debugPrintObjName(obj);
        // check attribute and jump
        uint8_t cond = obj->attr[val2 >> 3] & (1 << (7 - val2 & 0x07));
        execBranch(cond);
        break;
    }
    case OPC_SET_ATTR: {
        assert(val2 < 32);
        // set_attr object attribute
        ZObject_v1* obj = getObject(val1);
        // DEBUG: print object name
        //debugPrintObjName(obj);
        // set attribute
        obj->attr[val2 >> 3] |= (1 << (7 - val2 & 0x07));
        break;
    }
    case OPC_STORE:
        setVar((uint8_t)val1, val2);
        break;
    case OPC_INSERT_OBJ: {
        // insert_obj object destination
        assert(val1 < 256);
        assert(val2 < 256);
        uint8_t objId = (uint8_t)val1;
        uint8_t destId = (uint8_t)val2;
        ZObject_v1* obj = getObject(objId);
        ZObject_v1* dest = getObject(destId);

        // DEBUG: print object names
        //debugPrintObjName(obj);
        //debugPrintObjName(dest);
            
        // if obj already belongs to a hierarchy of objects we need to fix it
        if (obj->parent != 0) {
            // check if obj is first child of its parent
            ZObject_v1* objParent = getObject(obj->parent);
            if (objParent->child == objId) {
                // easy, just set obj sibling as new first child
                objParent->child = obj->sibling;
            }
            else {
                // this means that obj is not first child, but it's somewhere
                // in the sibling chain
                // check each of them until we find an object pointing to ours
                uint8_t curObjId = objParent->child;
                ZObject_v1* curObj = getObject(curObjId);
                while (curObj->sibling != 0) {
                    if (curObj->sibling == objId) {
                        // substitute obj with the next sibling in the chain
                        curObj->sibling = obj->sibling;
                        break;
                    }
                }
            }
        }
        // new sibling is the current first child of dest, which will become
        // second child
        obj->sibling = dest->child;
        // obj is the new first child of dest
        dest->child = objId;
        // fix also obj parent
        obj->parent = destId;
        break;
    }
    case OPC_LOADW:
        // load word at word address
        // TODO: check that the address lies in static or dynamic memory
        setVar(m_state.mem[m_temp.curPc++], READ16((uint32_t)val1 + (uint32_t)val2 * 2));
        break;
    case OPC_LOADB:
        // load byte at byte address
        // TODO: check that the address lies in static or dynamic memory
        setVar(m_state.mem[m_temp.curPc++], m_state.mem[(uint32_t)val1 + (uint32_t)val2]);
        break;
    case OPC_GET_PROP: {
        // get_prop object property -> (result)
        const ZObject_v1* obj = getObject(val1);
        // DEBUG: print object name
        //debugPrintObjName(obj);
        // get prop header
        uint16_t curPtr = BE16(obj->props);
        // skip name of object
        curPtr += 1 + m_state.mem[curPtr] * 2;
        // read properties
        while (1) {
            // read header
            uint8_t propHead = m_state.mem[curPtr];
            // end of property list?
            if (propHead == 0) {
                // not found, return default
                setVar(m_state.mem[m_temp.curPc++], getPropertyDefault(val2));
                break;
            }
            // propHead = 32 times the number of data bytes minus one, plus the property number
            uint8_t propNum = propHead & 0b00011111;
            uint8_t propSize = (propHead >> 5) + 1;
            // check prop num
            if (propNum == val2) {
                // we can get the value only if size is 1 or 2 bytes
                assert((propSize == 1) || (propSize == 2));
                // get prop value
                if (propSize == 1) {
                    setVar(m_state.mem[m_temp.curPc++], m_state.mem[curPtr + 1]);
                } else {
                    setVar(m_state.mem[m_temp.curPc++], READ16(curPtr + 1));
                }
                break;
            }
            // advance
            curPtr += 1 + propSize;
        }
        break;
    }
    case OPC_ADD:
        setVar(m_state.mem[m_temp.curPc++], (uint16_t)((int16_t)val1 + (int16_t)val2));
        break;
    case OPC_SUB:
        setVar(m_state.mem[m_temp.curPc++], (uint16_t)((int16_t)val1 - (int16_t)val2));
        break;
    case OPC_MUL:
        setVar(m_state.mem[m_temp.curPc++], (uint16_t)((int16_t)val1 * (int16_t)val2));
        break;
    case OPC_DIV:
        if (val2 == 0) {
            errorPrint("Division by zero");
            return false;
        }
        setVar(m_state.mem[m_temp.curPc++], (uint16_t)((int16_t)val1 / (int16_t)val2));
        break;
    case OPC_MOD:
        if (val2 == 0) {
            errorPrint("Division by zero");
            return 1;
        }
        setVar(m_state.mem[m_temp.curPc++], (uint16_t)((int16_t)val1 % (int16_t)val2));
        break;
    default:
        errorPrintf("2OP opcode %02X (%s) not implemented yet (op = %u)",
            opcode & 0b00011111, g_opcodes2OP[opcode & 0b00011111].name, opcode);
        return false;
    }
    return true;
}

bool ZMachine::execVarInstruction(uint8_t opcode) {
    switch (opcode & 0b00011111) {
    case OPC_CALL: {
        return execCall();
    }
    case OPC_STOREW: {
        assert(m_temp.numOps == 3);
        // TODO: check actual address (should be in dynamic memory)
        uint32_t addr = (uint32_t)m_temp.opVals[0] + (uint32_t)m_temp.opVals[1] * 2;
        WRITE16(addr, m_temp.opVals[2]);
        break;
    }
    case OPC_PUT_PROP: {
        // put_prop object property value
        uint16_t inObj = m_temp.opVals[0];
        uint16_t inProp = m_temp.opVals[1];
        uint16_t inValue = m_temp.opVals[2];
        const ZObject_v1* obj = getObject(inObj);
        // DEBUG: print object name
        //debugPrintObjName(obj);
        // get prop header
        uint16_t curPtr = BE16(obj->props);
        // skip name of object
        curPtr += 1 + m_state.mem[curPtr] * 2;
        // read properties
        while (1) {
            // read header
            uint8_t propHead = m_state.mem[curPtr];
            // end of property list?
            if (propHead == 0) {
                // not found
                errorPrintf("Property %02X not found", inProp);
                assert(false);
                return false;
            }
            // propHead = 32 times the number of data bytes minus one, plus the property number
            uint8_t propNum = propHead & 0b00011111;
            uint8_t propSize = (propHead >> 5) + 1;
            // check prop num
            if (propNum == inProp) {
                // we can set the value only if size is 1 or 2 bytes
                assert((propSize == 1) || (propSize == 2));
                //printf("propNum = %02X, propSize = %02X, wanted = %04X\n", propNum, propSize, vals[2]);
                // set prop value
                if (propSize == 1) {
                    // set least significant byte
                    m_state.mem[curPtr + 1] = (uint8_t)(inValue & 0xFF);
                }
                else {
                    WRITE16(curPtr + 1, inValue);
                }
                break;
            }
            // advance
            curPtr += 1 + propSize;
        }
        break;
    }
    case OPC_PRINT_CHAR: {
        assert(m_temp.numOps == 1);
        char ch[2] = {
            zsciiToAscii((uint8_t)m_temp.opVals[0]),
            '\0'
        };
        gamePrint(ch);
        break;
    }
    case OPC_PRINT_NUM: {
        assert(m_temp.numOps == 1);
        gamePrintf("%i", (int16_t)m_temp.opVals[0]);
        break;
    }
    case OPC_PUSH: {
        assert(m_temp.numOps == 1);
        // push value
        assert(m_state.sp < m_state.stackMem.size());
        m_state.stackMem[m_state.sp++] = m_temp.opVals[0];
        break;
    }
    case OPC_PULL: {
        assert(m_temp.numOps == 1);
        // pull (variable)
        if (m_state.sp <= m_state.bp) {
            errorPrint("Stack underflow");
            return false;
        }
        setVar((uint8_t)m_temp.opVals[0], m_state.stackMem[--m_state.sp]);
        break;
    }
    default:
        errorPrintf("Variable VAR opcode %02X (%s) not implemented yet (op = %u)",
            opcode & 0b00011111, g_opcodesVAR[opcode & 0b00011111].name, opcode);
        return false;
    }
    return true;
}

bool ZMachine::parseOpcodeAndOperands(ZMachineTemp& temp) const {
    // read full opcode
    uint32_t &curPc = temp.curPc;
    uint8_t opcode = m_state.mem[curPc++];
    temp.opcode = opcode;

    if (opcode == OPC_EXTENDED) { // only v5+
        // operand count is VAR (see VAR form below)
        // opcode in second byte
        assert(false);
        return false;
    }
    else if ((opcode & OPC_FORM_MASK) == OPC_FORM_VARIABLE) {
        // if bit5 == 0 => 2OP, else VAR
        // opcode in bit4-0
        // next byte with 4 operand types
        // 00 = large constant, 01 = small constant, 10 = variable, 11 = omitted
        // next operands
        uint8_t b = m_state.mem[curPc++];
        temp.opTypes[0] = (b >> 6) & 0x03;
        temp.opTypes[1] = (b >> 4) & 0x03;
        temp.opTypes[2] = (b >> 2) & 0x03;
        temp.opTypes[3] = b & 0x03;

        for (temp.numOps = 0; temp.numOps < 4; ++temp.numOps) {
            uint8_t opt = temp.opTypes[temp.numOps];
            if (opt == OPTYPE_OMITTED) {
                // no more valid operands
                break;
            }
            switch (opt) {
                case OPTYPE_LARGE_CONST: {
                    temp.opVals[temp.numOps] = READ16(curPc);
                    temp.opVars[temp.numOps] = temp.opVals[temp.numOps] & 0xFF; // see below
                    curPc += 2;
                    break;
                }
                case OPTYPE_SMALL_CONST: {
                    uint8_t val = (uint16_t)m_state.mem[curPc++];
                    temp.opVals[temp.numOps] = val;
                    temp.opVars[temp.numOps] = val; // hack for opcodes like INC_CHK to get the variable index correctly
                    // this should not be necessary for VAR instructions though
                    break;
                }
                case OPTYPE_VAR: {
                    uint8_t var = m_state.mem[curPc++];
                    temp.opVals[temp.numOps] = var; // this value needs to be set separately (as getVar can have side effects)
                    temp.opVars[temp.numOps] = var;
                    break;
                }
            }
        }
    }
    else if ((opcode & OPC_FORM_MASK) == OPC_FORM_SHORT) {
        // bit5-4 = operand type
        // if == 11 => 0OP, otherwise 1OP
        uint8_t opt = (opcode >> 4) & 0x03;
        if (opt == OPTYPE_OMITTED) {
            // this means it's a 0OP instruction
            // opcode in bit3-0
            temp.numOps = 0;
        }
        else {
            // this means it's a 1OP instruction
            // read operand
            temp.numOps = 1;
            temp.opTypes[0] = opt;
            switch (opt) {
                case OPTYPE_LARGE_CONST: {
                    temp.opVals[0] = READ16(curPc);
                    temp.opVars[0] = temp.opVals[0] & 0xFF; // hack for opcodes like INC_CHK to get the variable index correctly
                    curPc += 2;
                    break;
                }
                case OPTYPE_SMALL_CONST: {
                    uint8_t val = m_state.mem[curPc++];
                    temp.opVals[0] = val;
                    temp.opVars[0] = val; // hack for opcodes like INC_CHK to get the variable index correctly
                    break;
                }
                case OPTYPE_VAR: {
                    uint8_t var = m_state.mem[curPc++];
                    temp.opVals[0] = var; // this value needs to be set separately (as getVar can have side effects)
                    temp.opVars[0] = var;
                    break;
                }
                default:
                    assert(false);
                    return false;
            }
        }
    }
    else {
        // long form, operand count is always 2OP
        // bit6 = type 1st operand, bit5 = type 2nd operand
        // 0 = small constant, 1 = variable
        // opcode in bit4-0
        temp.numOps = 2;

        // read 1st operand
        uint8_t isVar1 = (opcode & BIT6) != 0;
        temp.opVars[0] = m_state.mem[curPc++];
        temp.opVals[0] = (uint16_t)temp.opVars[0]; // this value needs to be set separately (as getVar can have side effects)
        temp.opTypes[0] = isVar1 ? OPTYPE_VAR : OPTYPE_SMALL_CONST;

        // read 2nd operand
        uint8_t isVar2 = (opcode & BIT5) != 0;
        temp.opVars[1] = m_state.mem[curPc++];
        temp.opVals[1] = (uint16_t)temp.opVars[1]; // this value needs to be set separately (as getVar can have side effects)
        temp.opTypes[1] = isVar2 ? OPTYPE_VAR : OPTYPE_SMALL_CONST;
    }
    return true;
}

bool ZMachine::disasmCurInstruction(TextBuffer &tb) {
    // basic check
    if ((size_t)m_state.pc >= m_state.mem.size()) {
        errorPrint("PC out of memory");
        return false;
    }

    // get current pc
    ZMachineTemp temp;
    uint32_t initPc = m_state.pc;
    temp.curPc = initPc;
    tb.reset();
    tb.printf("[%04X] ", temp.curPc);

    // if (temp.curPc == 0x54C1) DebugBreak();

    // parse opcode and operands
    parseOpcodeAndOperands(temp);
    const ZOpcodeInfo* opInfo = nullptr;
    // parse opcode
    if (temp.opcode == OPC_EXTENDED) { // only v5+
        // operand count is VAR (see VAR form below)
        // opcode in second byte
        tb.copy("extended opcode not supported yet");
        return false;
    }
    else if ((temp.opcode & OPC_FORM_MASK) == OPC_FORM_VARIABLE) {
        // if bit5 == 0 => 2OP, else VAR
        // opcode in bit4-0
        // check if VAR or 2OP
        if (temp.opcode & BIT5) {
            // VAR opcode
            opInfo = &g_opcodesVAR[temp.opcode & 0b00011111];
        }
        else {
            // 2OP opcode
            opInfo = &g_opcodes2OP[temp.opcode & 0b00011111];
        }
    }
    else if ((temp.opcode & OPC_FORM_MASK) == OPC_FORM_SHORT) {
        // bit5-4 = operand type
        // if == 11 => 0OP, otherwise 1OP
        uint8_t opt = (temp.opcode >> 4) & 0x03;
        if (opt == OPTYPE_OMITTED) {
            // this means it's a 0OP instruction
            // opcode in bit3-0
            opInfo = &g_opcodes0OP[temp.opcode & 0b00001111];
        }
        else {
            // this means it's a 1OP instruction
            // opcode in bit3-0
            opInfo = &g_opcodes1OP[temp.opcode & 0b00001111];
        }
    }
    else {
        // long form, operand count is always 2OP
        // opcode in bit4-0
        opInfo = &g_opcodes2OP[temp.opcode & 0b00011111];
    }
    // print opcode
    tb.copy(opInfo->name);
    // print operands
    for (uint8_t i = 0; i < temp.numOps; ++i) {
        switch (temp.opTypes[i]) {
            case OPTYPE_SMALL_CONST:
                tb.printf(" #%02X", temp.opVals[i]);
                break;
            case OPTYPE_LARGE_CONST:
                tb.printf(" #%04X", temp.opVals[i]);
                break;
            case OPTYPE_VAR:
                debugPrintVarName(tb, temp.opVars[i]);
                break;
        }
        // print object names eventually
        if ((opInfo->flags & OPF_OBJ1) && (i == 0) && (temp.opTypes[i] != OPTYPE_VAR))
            debugPrintObjName(tb, getObject(temp.opVals[i]));
        else if ((opInfo->flags & OPF_OBJ2) && (i == 1) && (temp.opTypes[i] != OPTYPE_VAR))
            debugPrintObjName(tb, getObject(temp.opVals[i]));
    }
    // parse branch info
    if (opInfo->flags & OPF_BRANCH)
        disasmBranch(tb, temp.curPc);
    // parse storing of result
    if (opInfo->flags & OPF_STORE) {
        tb.copy(" ->");
        debugPrintVarName(tb, m_state.mem[temp.curPc++]);
    }
    return true;
}

bool ZMachine::step() {
    bool ret;

    // basic check
    if ((size_t)m_state.pc >= m_state.mem.size()) {
        errorPrint("PC out of memory");
        return false;
    }

    // set current pc
    m_temp.curPc = m_state.pc;

    // parse opcode and operands
    if (!parseOpcodeAndOperands(m_temp))
        return false;

    // read operands which are variables
    for (uint8_t i = 0; i < m_temp.numOps; ++i) {
        if (m_temp.opTypes[i] == OPTYPE_VAR)
            m_temp.opVals[i] = getVar(m_temp.opVars[i]);
    }

    /*if (m_state.pc == 0x8EF8)
        DebugBreak();*/

    // execute opcode
    if (m_temp.opcode == OPC_EXTENDED) { // only v5+
        // operand count is VAR (see VAR form below)
        // opcode in second byte
        errorPrint("Extended opcode not implemented yet");
        return false;
    }
    else if ((m_temp.opcode & OPC_FORM_MASK) == OPC_FORM_VARIABLE) {
        // if bit5 == 0 => 2OP, else VAR
        // opcode in bit4-0
        // check if VAR or 2OP
        if (m_temp.opcode & BIT5) {
            // VAR opcode
            ret = execVarInstruction(m_temp.opcode);
        }
        else {
            // handle 2OP
            ret = exec2OPInstruction(m_temp.opcode);
        }
    }
    else if ((m_temp.opcode & OPC_FORM_MASK) == OPC_FORM_SHORT) {
        // bit5-4 = operand type
        // if == 11 => 0OP, otherwise 1OP
        uint8_t opt = (m_temp.opcode >> 4) & 0x03;
        if (opt == OPTYPE_OMITTED) {
            // this means it's a 0OP instruction
            // opcode in bit3-0
            ret = exec0OPInstruction(m_temp.opcode);
        }
        else {
            // this means it's a 1OP instruction
            // opcode in bit3-0
            ret = exec1OPInstruction(m_temp.opcode);
        }
    }
    else {
        // long form, operand count is always 2OP
        // opcode in bit4-0
        ret = exec2OPInstruction(m_temp.opcode);
    }

    // update state pc
    if (ret)
        m_state.pc = m_temp.curPc;
    return ret;
}

bool ZMachine::run() {
    bool ret = true;
    while (ret) {
        ret = step();
    }
    return ret;
}
