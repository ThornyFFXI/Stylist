#ifndef __ASHITA_Stylist_Structs_H_INCLUDED__
#define __ASHITA_Stylist_Structs_H_INCLUDED__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "C:\Program Files (x86)\Ashita 4\plugins\sdk\Ashita.h"
#include <chrono>
#include <list>
#include <map>
#include <stdint.h>
#include <string>

struct singleFilter_t
{
    uint16_t Model;
    std::string Initial;
    std::string Target;

    singleFilter_t(){}

    singleFilter_t(uint16_t Model, std::string Initial, std::string Target)
        : Model(Model)
        , Initial(Initial)
        , Target(Target)
    {}
};
struct singleMask_t
{
    bool Override;
    uint16_t Value;
    std::string Text;

    singleMask_t()
        : Override(false)
        , Value(0)
        , Text("Undefined")
    {}

    singleMask_t(bool Override, uint16_t Value, std::string Text)
        : Override(Override)
        , Value(Value)
        , Text(Text)
    {}
};

struct charMask_t
{
    singleMask_t SlotMasks[10];
    std::map<uint16_t, singleFilter_t> ModelFilters[10];
};

struct modelValues_t
{
    uint16_t Values[10];
};
struct modelPointers_t
{
    uint8_t* Face;
    uint8_t* Race;
    uint16_t* Equip;
    uint8_t* Blink;

    modelPointers_t(uint8_t* packetOffset)
    {
        Face = packetOffset;
        Race = packetOffset + 1;
        Equip = (uint16_t*)(packetOffset + 2);
        Blink = NULL;
    }

    modelPointers_t(Ashita::FFXI::entity_t* entity)
    {
        Race  = (uint8_t*)((uint8_t*)entity + offsetof(Ashita::FFXI::entity_t, Race));
        Face = (uint8_t*)((uint8_t*)entity + offsetof(Ashita::FFXI::entity_t, Look));
        Equip = (uint16_t*)((uint8_t*)entity + offsetof(Ashita::FFXI::entity_t, Look) + 2);
        Blink = ((uint8_t*)entity + offsetof(Ashita::FFXI::entity_t, ModelUpdateFlags));
    }

    void Write(modelValues_t values)
    {
        *Race = values.Values[0];
        *Face = values.Values[1];
        for (int x = 0; x < 8; x++)
        {
            Equip[x] = values.Values[x + 2];
        }
        if (Blink != NULL)
            *Blink = 1;
    }

    bool operator!=(const modelValues_t& other)
    {
        if (*Race != other.Values[0])
            return true;

        if (*Face != other.Values[1])
            return true;

        for (int x = 0; x < 8; x++)
        {
            if (Equip[x] != other.Values[x + 2])
                return true;
        }

        return false;
    }
};


struct settings_t
{
    bool NoBlinkSelf;
    bool NoBlinkTarget;
    bool NoBlinkOthers;
    charMask_t DefaultOverride;
    std::map<std::string, charMask_t> CharOverrides;

    settings_t()
        : NoBlinkSelf(true)
        , NoBlinkTarget(true)
        , NoBlinkOthers(false)
        , DefaultOverride(charMask_t())
        , CharOverrides(std::map<std::string, charMask_t>())
    {}
};

struct state_t
{
    std::map<uint16_t, modelValues_t> RealEquip;
    std::string myName;
    uint16_t myIndex;
};

struct modelinfo_t
{
    std::map<std::string, uint8_t> Slot;
    std::map<std::string, uint16_t> Race;
    std::map<uint16_t, uint16_t> Equip[8];
};
#endif