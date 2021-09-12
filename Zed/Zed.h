
#ifndef _H_ZED_
#define _H_ZED_

#include <cstdint>
#include <vector>
#include <functional>


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

typedef struct ZMachineState {
    std::vector<uint8_t> mem;
    const ZHeader* header = nullptr;
    uint32_t pc = 0;
    std::vector<uint16_t> callsPtr;
    uint32_t curCall = 0;
    std::vector<uint16_t> stackMem;
    uint32_t sp = 0;

    //! Reset Z-Machine state
    void reset();
} ZMachineState;

typedef struct ZMachineTemp {
    uint32_t curPc;
    uint8_t opcode;
    uint8_t numOps;
    uint16_t opVals[8];
    uint8_t opTypes[8];
    uint8_t opVars[8];
} ZMachineTemp;

class Zed {
public:
    ZMachineState m_state;
    ZMachineTemp m_temp;

    std::function<void(const char*)> debugPrintCallback = nullptr;
    std::function<void(const char*)> errorPrintCallback = nullptr;
    std::function<void(const char*)> gamePrintCallback = nullptr;

    void copyStory(const uint8_t* mem, size_t memSize);

    void reset();

    bool run();

    bool step();

    void disasmCurInstruction();

    int parseZText(const uint8_t* text, char* out, uint32_t outSize, uint32_t* outTextBytesRead, bool enableAbbrev = true);

    int parseZCharacters(const uint8_t* buf, uint32_t numBuf, char* out, uint32_t outSize, bool enableAbbrev = true);

protected:
    void debugZText(const uint8_t* text);
    void debugPrintf(const char* fmt, ...);
    void debugPrint(const char* text);

    void debugPrintVarName(uint8_t var);
    void debugPrintObjName(const ZObject_v1* obj);

    void errorPrintf(const char* fmt, ...);
    void errorPrint(const char* text);

    void gamePrintf(const char* fmt, ...);
    void gamePrint(const char* text);

    //! Read variable indexed by idx
    uint16_t getVar(uint8_t idx);

    //! Write in variable indexed by idx
    void setVar(uint8_t idx, uint16_t value);

    // v1-3
    ZObject_v1* getObject(uint16_t objIndex);

    // v1-3
    uint16_t getPropertyDefault(uint16_t propIndex);


    //! Just read branch info and return it
    void readBranchInfo(uint32_t& curPc, bool& jumpCond, uint32_t& dest) const;

    //! Parse opcode and subsequent operands, storing everything in temp
    bool parseOpcodeAndOperands(ZMachineTemp& temp) const;

    //! Execute a CALL
    void execCall();

    //! Execute a RET, with the given return value
    void execRet(uint16_t val);

    //! Read branch info and jump if condition is met
    void execBranch(bool condition);

    bool exec0OPInstruction(uint8_t opcode);
    bool exec1OPInstruction(uint8_t opcode);
    bool exec2OPInstruction(uint8_t opcode);
    bool execVarInstruction(uint8_t opcode);

    //! Disasm the branch info
    void disasmBranch(uint32_t& curPc);
};

#endif // _H_ZED_
