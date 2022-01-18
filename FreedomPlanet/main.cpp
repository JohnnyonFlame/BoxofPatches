#include <iostream>
#include <fstream>
#include <sstream>
#include <charconv>
#include <cstring>
#include <sys/types.h>
#include <xbyak.h>

#include "sig.h"
#include "proc.h"
#define BIN_NAME "bin32/Chowdren"

static long *fps_target_ptr = NULL;
void hook_setup_hz_target()
{
    // Unlike Iconoclasts, this game runs at a fixed framerate.
    *fps_target_ptr = 60;
}

void FreedomPlanetPatches() __attribute__((constructor));
void FreedomPlanetPatches()
{
    auto mmaps = get_proc_self_maps(BIN_NAME);
    if (mmaps.empty()) {
        std::cout << "Empty mapping list, wrong BIN_NAME or failure to open /proc/self/maps.\n";
        exit(-1);
    }

    proc_maps_t *map = NULL;
    auto setup_hz_target = sig::scan<
    /* 0842d450 */ " 53 "             // PUSH       EBX
    /* 0842d451 */ " 83 ec 18 "       // SUB        ESP, 0x18
    /* 0842d454 */ " a1 90 fa "       // MOV        EAX, [DAT_0a3efa90]
                   " 3e 0a "
    /* 0842d459 */ " 89 04 24 "       // MOV        dword ptr [ESP]=>local_1c, EAX
    /* 0842d45c */ " e8 ff dc "       // CALL       SDL_GetWindowFlags                               undefined SDL_GetWindowFlags(und
                   " 76 01 "
    /* 0842d461 */ " a8 48 "          // TEST       AL, 0x48
    /* 0842d463 */ " 75 4d "          // JNZ        LAB_0842d4b2
    /* 0842d465 */ " db 05 40 "       // FILD       dword ptr [DAT_0a3ca040]
                   " a0 3c 0a "
    >(mmaps, &map);

    if (!setup_hz_target) {
        std::cout << "Failed to acquire setup_hz_target signature!\n";
        exit(-1);
    }

    std::cout << "Found sig at 0x" << std::hex << (uintptr_t)setup_hz_target << "\n";
    // Get pointer to the fps_target long
    /* 08109a82 db 05 68        FILD       dword ptr [DAT_08d4cc68]
                cc d4 08 */
    if (sig::decode<"DB 05 XX XX XX XX">(setup_hz_target + 21, (uintptr_t*)&fps_target_ptr)) {
        std::cout << "Found fps_target_ptr at 0x" << std::hex << (uintptr_t)fps_target_ptr << "\n";
    } else {
        std::cout << "Failed to find fps_target_ptr\n";
        exit(-1);
    }

    struct HZ_TARGET_PAYLOAD : Xbyak::CodeGenerator {
        HZ_TARGET_PAYLOAD(uint8_t *ptr) : Xbyak::CodeGenerator(4096, (void *)ptr)
        {
            push(ebx);
            mov(ebx, (uint32_t)hook_setup_hz_target);
            call(ebx);
            pop(ebx);
            ret();
            nop();
        }
    };

    // Compile and finish
    HZ_TARGET_PAYLOAD hz_target_payload(setup_hz_target);
    hz_target_payload.ready();
}