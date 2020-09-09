#ifndef __ASHITA_Stylist_H_INCLUDED__
#define __ASHITA_Stylist_H_INCLUDED__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "C:\Program Files (x86)\Ashita 4\plugins\sdk\Ashita.h"
#include "Structs.h"
#include "Output.h"
#include <map>

class Stylist : IPlugin
{
public:
    std::map<uint16_t, appearance_t> mRealEquipValues;
    modelinfo_t mModelInfo;
    settings_t mSettings;
    OutputHelpers* pOutput;

public:
    const char* GetName(void) const override
    {
        return u8"Stylist";
    }
    const char* GetAuthor(void) const override
    {
        return u8"Thorny";
    }
    const char* GetDescription(void) const override
    {
        return u8"Insert description here.";
    }
    const char* GetLink(void) const override
    {
        return u8"Insert link here.";
    }
    double GetVersion(void) const override
    {
        return 1.00f;
    }
    int32_t GetPriority(void) const override
    {
        return 0;
    }
    uint32_t GetFlags(void) const override
    {
        return (uint32_t)Ashita::PluginFlags::Legacy;
    }
	
    bool Initialize(IAshitaCore* core, ILogManager* logger, const uint32_t id) override;
    void Release(void) override;
	
    // Event Callbacks: ChatManager
    bool HandleCommand(int32_t mode, const char* command, bool injected) override;	
	
    // Event Callbacks: PacketManager
    bool HandleIncomingPacket(uint16_t id, uint32_t size, const uint8_t* data, uint8_t* modified, uint32_t sizeChunk, const uint8_t* dataChunk, bool injected, bool blocked) override;

    //fileio.cpp
    void InitModelInfo();
    void LoadSettings();
    void SaveSettings();

    //models.cpp
    std::string GetSlotString(uint8_t slot);
    uint8_t GetModelTable(std::string text);
    uint16_t GetModelId(uint8_t table, std::string text);
    bool IsPositiveInteger(const char* input);
    std::string FormatName(std::string name);

    //modelmods.cpp
    void ApplyModelChanges(appearance_t* appearance, uint16_t index, std::string pName);
    void SaveInitialModels();
    void UpdateAllModels(bool ForceRealModel);
    void UpdateOneModel(std::string name);
    void StoreRealValues(appearance_t* appearance, uint16_t index);
    bool CheckBlinkBlock(uint16_t index);
    bool IsEntityRendered(uint16_t index);
};
#endif