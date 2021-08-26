
#include <cstdio>
#include <cassert>
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "Zed.h"
#include <vector>


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

#if 0 // TEST: parse z-text
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
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    std::vector<uint8_t> mem(size);
    if (fread(&mem[0], 1, size, fp) != size) {
        printf("ERROR: Could not read %i bytes of story file\n", size);
        fclose(fp);
        return 1;
    }
    fclose(fp);


    const ZHeader* header = (const ZHeader*)&mem[0];

    assert(header->version <= 3);

    printf("highMemory = %04X, staticMemory = %04X\n", BE16(header->highMemory), BE16(header->staticMemory));
    printf("dict = %04X, objects = %04X, globals = %04X\n", BE16(header->dictionaryAddress), BE16(header->objectsAddress), BE16(header->globalsAddress));
    printf("initPC = %04X\n", BE16(header->initPC));

    uint32_t pc = BE16(header->initPC);

    // long form: bit7 = 0, bit6 = type 1st operand (0 = small constant, 1 = variable),
    // bit5 = type 2nd operand, bit4-0 = opcode

// 2OP
#define OP_CONST_CONST              0b00000000
#define OP_CONST_VAR                0b00100000
#define OP_VAR_CONST                0b01000000
#define OP_VAR_VAR                  0b01100000

#define OPC_ADD                     0b00010100 // h14 (20), h54 (84), h34 (52), h74 (116)

// VAR
#define OPC_CALL                    224

#define OPTYPE_LARGE_CONST          0x00
#define OPTYPE_SMALL_CONST          0x01
#define OPTYPE_VAR                  0x02
#define OPTYPE_OMITTED              0x03

#define CALLFRAME_OFF_PC                0
#define CALLFRAME_OFF_RET               1
#define CALLFRAME_STORE_VAR             2
#define CALLFRAME_OFF_NUM_LOCALS        3
#define CALLFRAME_OFF_LOCALS            4

// v1-3
#define packedAddressToByte(_addr) \
        (2 * _addr)

    const char* opTypes[] = {
        "large", "small", "var", "(no)"
    };

    //std::vector<uint8_t> callsMem(16 * 1024);
    std::vector<uint16_t> callsPtr(1024);
    uint32_t curCall = 0;

    std::vector<uint16_t> stackMem(64 * 1024);
    uint32_t sp = 0;

    auto printVarName = [](uint8_t var) {
        if (var == 0)
            printf(" (SP)");
        else if (var <= 0x0F)
            printf(" L%02X", var - 1);
        else
            printf(" G%02X", var - 0x10);
    };

    auto getVar = [&](uint8_t b) -> uint16_t {
        // check which var
        if (b == 0) {
            // pop from stack
            assert(sp > 0);
            return stackMem[--sp];
        }
        else if (b <= 0x0F) {
            // local variable
            assert(curCall > 0);
            const uint32_t callPtr = callsPtr[curCall - 1];
            assert((b - 1) < stackMem[callPtr + CALLFRAME_OFF_NUM_LOCALS]); // check number of locals
            return stackMem[callPtr + CALLFRAME_OFF_LOCALS + b - 1];
        }
        else {
            // globals
            uint32_t addr = header->globalsAddress + (b - 0x10) * 2;
            return ((uint16_t)mem[addr] << 8) | ((uint16_t)mem[addr + 1]);
        }
    };

    auto setVar = [&](uint8_t var, uint16_t value) {
        // check which var
        if (var == 0) {
            // push to stack
            assert(sp < (stackMem.size() - 1));
            stackMem[sp++] = value;
        }
        else if (var <= 0x0F) {
            // local variable
            assert(curCall > 0);
            const uint32_t callPtr = callsPtr[curCall - 1];
            assert((var - 1) < stackMem[callPtr + CALLFRAME_OFF_NUM_LOCALS]); // check number of locals
            stackMem[callPtr + CALLFRAME_OFF_LOCALS + var - 1] = value;
        }
        else {
            // globals
            uint32_t addr = header->globalsAddress + (var - 0x10) * 2;
            mem[addr] = (value >> 8) & 0xFF;
            mem[addr + 1] = value & 0xFF;
        }
    };

    auto getOperand_CONST = [&]() {
        uint8_t _b = mem[pc++];
        printf(" #%02X", _b);
        return _b;
    };

    auto getOperand_VAR = [&]() {
        uint8_t _b = mem[pc++];
        uint16_t _val = getVar(_b);
        printVarName(_b);
        return _val;
    };

    auto dumpMem = [&](uint32_t _pc, uint32_t _size) {
        for (uint32_t i = 0; i < _size; ++i) {
            printf("%02X ", mem[_pc + i]);
            if (((i + 1) % 8) == 0)
                printf("\n");
        }
        printf("\n");
    };


#define DEFINE_2OP_EX(name, func, opType1, opType2) \
    case OPC_ ## name | OP_ ## opType1 ## _ ## opType2: { \
        printf("%s", # name); \
        uint16_t val1 = getOperand_ ## opType1 (); \
        uint16_t val2 = getOperand_ ## opType2 (); \
        uint8_t st = mem[pc++]; \
        setVar(st, (uint16_t)(func)); \
        printf(" ->"); printVarName(st); printf("\n"); \
        break; \
    }

#define DEFINE_2OP(name, func) \
    DEFINE_2OP_EX(name, (func), CONST, CONST) \
    DEFINE_2OP_EX(name, (func), CONST, VAR) \
    DEFINE_2OP_EX(name, (func), VAR, CONST) \
    DEFINE_2OP_EX(name, (func), VAR, VAR)

    while (1) {
        // dumpMem(pc, 32);

        uint8_t opcode = mem[pc++];
        switch (opcode) {
        case OPC_CALL: {
            printf("CALL");
            uint8_t b = mem[pc++];
            uint8_t opType[4] = {
                (b >> 6) & 0x03,
                (b >> 4) & 0x03,
                (b >> 2) & 0x03,
                b & 0x03
            };

            uint16_t vals[4] = {};
            uint8_t numOps;
            for (numOps = 0; numOps < 4; ++numOps) {
                uint8_t opt = opType[numOps];
                uint16_t val;
                if (opt == OPTYPE_LARGE_CONST) {
                    b = mem[pc++];
                    uint8_t c = mem[pc++];
                    vals[numOps] = ((uint16_t)b << 8) | (uint16_t)c;
                    printf(" %04X", vals[numOps]);
                }
                else if (opt == OPTYPE_SMALL_CONST) {
                    vals[numOps] = mem[pc++];
                    printf(" %02X", vals[numOps]);
                }
                else if (opt == OPTYPE_VAR) {
                    b = mem[pc++];
                    printVarName(b);
                    vals[numOps] = getVar(b);
                }
                else {
                    // no more valid operands
                    break;
                }
            }

            // where to store the result
            // TODO: handle this
            b = mem[pc++];
            printf(" ->"); printVarName(b); printf("\n");

            // execute routine
            // save call frame ptr
            callsPtr[curCall] = sp;
            // store new location (not necessary, but handy for debugging)
            uint32_t newPc = packedAddressToByte(vals[0]); // packed
            assert(pc < mem.size());
            stackMem[sp++] = newPc; // TOFIX: saving to 16bit
            // store old location
            stackMem[sp++] = pc;
            // store return variable
            stackMem[sp++] = b;
            // go to new location
            pc = newPc;
            // read number of local variables
            uint8_t numLocals = mem[pc++];
            assert(numLocals < 16);
            stackMem[sp++] = numLocals;
            // read the initial values of the local variables, v1-4
            // NOTE: locals starts from callsPtr + 2
            for (uint8_t i = 0; i < numLocals; ++i) {
                b = mem[pc++];
                stackMem[sp + i] = ((uint16_t)b << 8) | ((uint16_t)mem[pc++]);
            }
            // overwrite them with function arguments
            for (uint8_t i = 0; i < (numOps - 1) && i < numLocals; ++i) {
                stackMem[sp + i] = vals[i + 1];
            }
            // advance sp
            sp += numLocals;
            // advance call index
            ++curCall;

            // DEBUG: print call frame
            uint32_t pp = callsPtr[curCall - 1];
            printf("Call frame %u: start = %04X, ret = %04X, numLocals = %u\n", curCall - 1, stackMem[pp + CALLFRAME_OFF_PC], stackMem[pp + CALLFRAME_OFF_RET], stackMem[pp + CALLFRAME_OFF_NUM_LOCALS]);
            for (uint16_t i = 0; i < stackMem[pp + CALLFRAME_OFF_NUM_LOCALS]; ++i)
                printf("%u) %04X\n", i, stackMem[pp + CALLFRAME_OFF_LOCALS + i]);

            break;
        }
        /*case OPC_ADD | OP_VAR_CONST: {
            printf("ADD");
            uint8_t b = mem[pc++];
            uint16_t val = getVar(b);
            printVarName(b);
            uint8_t c = mem[pc++];
            printf(" #%02X", c);
            // where to store the result
            uint8_t st = mem[pc++];
            setVar(st, (uint16_t)((int16_t)val + (int16_t)c));
            printf(" ->"); printVarName(st); printf("\n");
            break;
        }
        case OPC_ADD | OP_VAR_VAR: {
            printf("ADD");
            uint8_t b = mem[pc++];
            uint16_t val = getVar(b);
            printVarName(b);
            uint8_t c = mem[pc++];
            uint16_t val2 = getVar(c);
            printVarName(c);
            // where to store the result
            uint8_t st = mem[pc++];
            setVar(st, (uint16_t)((int16_t)val + (int16_t)val2));
            printf(" ->"); printVarName(st); printf("\n");
            break;
        }*/
        DEFINE_2OP(ADD, (int16_t)val1 + (int16_t)val2);
        default:
            dumpMem(pc - 1, 32);
            printf("ERROR: Opcode %u not implemented yet\n", opcode);
            return 1;
        };
        
        printf("\n");
    }

    return 0;
}
