#include <iostream>
#include <fstream>
#include <sstream>
#include <charconv>
#include <cstring>
#include <sys/types.h>
#include <xbyak.h>

#include "sig.h"
#include "proc.h"
#define BIN_NAME "32/ShovelKnight"

void ShovelKnightPatches() __attribute__((constructor));
void ShovelKnightPatches()
{
    auto mmaps = get_proc_self_maps(BIN_NAME);
    if (mmaps.empty()) {
        std::cout << "Empty mapping list, wrong BIN_NAME or failure to open /proc/self/maps.\n";
        exit(-1);
    }

    proc_maps_t *map = NULL; 
    auto nag_ptr = sig::scan<
    /* 08a908bd */ "89 3c 24 "         // MOV        dword ptr [ESP]=>local_4c, EDI
    /* 08a908c0 */ "c7 44 24 "         // MOV        dword ptr [ESP + 0x8]=>local_44, 0x8
                   "08 08 00 "   
                   "00 00 "  
    /* 08a908c8 */ "c7 44 24 "         // MOV        dword ptr [ESP + 0x4]=>local_48, 0x4
                   "04 04 00 "   
                   "00 00 "  
    /* 08a908d0 */ "e8 ?? ?? "         // CALL       TitleMenuEnv::CreateMessagePrompt                undefined CreateMessagePrompt(Ti
                   "?? ?? "  
    /* 08a908d5 */ "8b 87 f8 "         // MOV        EAX, dword ptr [EDI + 0xf8]
                   "00 00 00 "  
    /* 08a908db */ "c6 80 cd "         // MOV        byte ptr [EAX + 0xcd], 0x1
                   "00 00 00 01 "  
    /* 08a908e2 */ "ff 87 18 "         // INC        dword ptr [EDI + 0x118]
                   "01 00 00 "  
    /* 08a908e8 */ "e9 a3 00 "         // JMP        LAB_08a90990 (or nag_ptr + 0xD3)
                   "00 00 "  
    >(mmaps, &map);

    if (!nag_ptr) {
        std::cout << "Failed to acquire nag_ptr signature! Is this a GOG release?\n";
    } else {
        std::cout << "Found sig at 0x" << std::hex << (uintptr_t)nag_ptr << "\n";
        
        struct NAG_PAYLOAD : Xbyak::CodeGenerator {
            NAG_PAYLOAD(uint8_t *ptr) : Xbyak::CodeGenerator(4096, (void *)ptr)
            {
                inc(dword[edi + 0x118]);
                jmp(&ptr[0xD3], T_NEAR);
            }
        };

        NAG_PAYLOAD nag_payload(nag_ptr);
        nag_payload.ready();
    }
}