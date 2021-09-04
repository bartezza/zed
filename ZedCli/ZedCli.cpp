
#include <cstdio>
#include <cassert>
#include <vector>
#include "../Zed/Zed.h"


int main(int argc, char** argv) {
    // load story
    // const char* filename = "..\\..\\..\\..\\Data\\zork1-r119-s880429.z3";
    const char* filename = "..\\..\\..\\..\\Data\\zork1-r88-s840726.z3";
    if (argc >= 2) {
        filename = argv[1];
    }
    // open story file
    FILE* fp = fopen(filename, "rb");
    if (fp == nullptr) {
        printf("[ERROR] Could not open '%s'\n", filename);
        return 1;
    }
    // get total size
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    // alloc memory
    std::vector<uint8_t> mem(size);
    // read the file
    if (fread(&mem[0], 1, size, fp) != size) {
        printf("[ERROR] Could not read %i bytes of story file\n", size);
        fclose(fp);
        return 1;
    }
    fclose(fp);

    // instantiate zed
    Zed zed;
    // set callbacks
    zed.debugPrintCallback = [](const char* text) {
        fputs(text, stdout);
    };
    // copy story
    zed.copyStory(mem.data(), mem.size());

    // run
    //zed.run();
    bool ret = true;
    while (ret) {
        zed.disasmCurInstruction();
        ret = zed.step();
    }
    return 0;
}
