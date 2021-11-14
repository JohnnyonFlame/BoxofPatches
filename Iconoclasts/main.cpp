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

// Target 30fps target by default
static long fps_target_setting = 30;
static long *fps_target_ptr = NULL;
static uint8_t *maxfps_disabled = NULL;

void hook_setup_hz_target()
{
    *maxfps_disabled = '\1';
    char *fps_setting = getenv("CHOWDREN_FPS");
    if (fps_setting) 
        std::from_chars(fps_setting, fps_setting+strlen(fps_setting), fps_target_setting);

    *fps_target_ptr = fps_target_setting;
}

void IconoclastsPatches() __attribute__((constructor));
void IconoclastsPatches()
{
    auto mmaps = get_proc_self_maps(BIN_NAME);
    if (mmaps.empty()) {
        std::cout << "Empty mapping list, wrong BIN_NAME or failure to open /proc/self/maps.\n";
        exit(-1);
    }

    proc_maps_t *map = NULL;
    auto setup_hz_target = sig::scan<
    /* 08109a60 */ " 53"              // PUSH       EBX
    /* 08109a61 */ " 83 ec 38"        // SUB        ESP, 0x38
    /* 08109a64 */ " 80 3d ??"        // CMP        byte ptr [DAT_08db2f20], 0x0
                   " ?? ?? ?? 00"
    /* 08109a6b */ " 0f 84 97"        // JZ         LAB_08109b08
                   " 00 00 00"
    /* 08109a71 */ " a1 ?? ??"        // MOV        EAX, [DAT_08db2db0]
                   " ?? ??"
    >(mmaps, &map);

    if (!setup_hz_target) {
        std::cout << "Failed to acquire setup_hz_target signature!\n";
        exit(-1);
    }

    std::cout << "Found sig at 0x" << std::hex << (uintptr_t)setup_hz_target << "\n";

    // Get pointer to the max_fps toggle
    /* 08109a64 80 3d 20        CMP        byte ptr [DAT_08db2f20], 0x0
                2f db 08 00 */
    if (sig::decode<"80 3D XX XX XX XX 00">(setup_hz_target + 4, (uintptr_t*)&maxfps_disabled)) {
        std::cout << "Found maxfps_disabled at 0x" << std::hex << (uintptr_t)maxfps_disabled << "\n";
    } else {
        std::cout << "Failed to find maxfps_disabled\n";
        exit(-1);
    }

    // Get pointer to the fps_target long
    /* 08109a82 db 05 68        FILD       dword ptr [DAT_08d4cc68]
                cc d4 08 */
    if (sig::decode<"DB 05 XX XX XX XX">(setup_hz_target + 34, (uintptr_t*)&fps_target_ptr)) {
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