#ifndef __ASHITA_Stylist_Structs_H_INCLUDED__
#define __ASHITA_Stylist_Structs_H_INCLUDED__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <chrono>
#include <list>
#include <map>
#include <stdint.h>
#include <string>

#define RBUFP(p, pos) (((uint8_t*)(p)) + (pos))
#define Read8(p, pos) (*(uint8_t*)RBUFP((p), (pos)))
#define Read16(p, pos) (*(uint16_t*)RBUFP((p), (pos)))
#define Read32(p, pos) (*(uint32_t*)RBUFP((p), (pos)))
#define Read64(p, pos) (*(uint64_t*)RBUFP((p), (pos)))
#define ReadFloat(p, pos) (*(float_t*)RBUFP((p), (pos)))

#define WBUFP(p, pos) (((uint8_t*)(p)) + (pos))
#define Write8(p, pos) (*(uint8_t*)WBUFP((p), (pos)))
#define Write16(p, pos) (*(uint16_t*)WBUFP((p), (pos)))
#define Write32(p, pos) (*(uint32_t*)WBUFP((p), (pos)))
#define Write64(p, pos) (*(uint64_t*)WBUFP((p), (pos)))
#define WriteFloat(p, pos) (*(float_t*)WBUFP((p), (pos)))

#define CheckArg(a, b) (argcount > a) && (_stricmp(args[a].c_str(), b) == 0)

struct appearance_t
{
    uint8_t Face;
    uint8_t Race;
    uint16_t Equip[8];
};

//When in overrides, flag 0x01 means write the slot.
//When in real values, flag 0x01 means the slot was already written by an override.
struct charmask_t
{
    bool Override[10];
    uint16_t Params[10];
    std::string String[10];
};

struct settings_t
{
    bool NoBlinkSelf;
    bool NoBlinkTarget;
    bool NoBlinkOthers;
    std::map<std::string, charmask_t> CharOverrides;

    settings_t()
        : NoBlinkSelf(true)
        , NoBlinkTarget(true)
        , NoBlinkOthers(false)
        , CharOverrides(std::map<std::string, charmask_t>()) { }
};

struct modelinfo_t
{
    std::map<std::string, uint8_t> Slot;
    std::map<std::string, uint16_t> Race;
    std::map<uint16_t, uint16_t> Equip[8];
};
#endif