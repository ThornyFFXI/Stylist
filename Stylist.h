#ifndef __ASHITA_Stylist_H_INCLUDED__
#define __ASHITA_Stylist_H_INCLUDED__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "C:\Ashita 4\plugins\sdk\Ashita.h"
#include "..\common\Utilities.h"
#include "..\common\Settings.h"
#include "Structs.h"
#include "..\common\Output.h"
#include <map>

class Stylist : IPlugin
{
private:
    IAshitaCore* m_AshitaCore;
    ILogManager* m_LogManager;
    uint32_t m_PluginId;

public:
    modelinfo_t mModelInfo;
    settings_t mSettings;
    state_t mState;
    OutputHelpers* pOutput;
    SettingsHelper* pSettings;

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
        return 1.10f;
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
    void LoadDefaultXml(bool forceReload);
    void LoadSettings(const char* Name);
    void SaveSettings(const char* Name);

    //models.cpp
    std::string GetSlotString(uint8_t slot);
    uint8_t GetModelTable(std::string text);
    uint16_t GetModelId(uint8_t table, std::string text);
    bool IsPositiveInteger(const char* input);

    //modelmods.cpp
    void HandleModelPacket(modelPointers_t pointers, uint16_t index, std::string pName);
    void ApplyBlinkBlock(uint16_t index, modelPointers_t pointers);
    void ApplyModelChanges(modelPointers_t pointers, charMask_t overrides, modelValues_t realValues);
    void InitializeState();
    void UpdateAllModels(bool ForceRealModel);
    void UpdateOneModel(std::string name, charMask_t mask);
    bool CheckBlinkBlock(uint16_t index);
    modelValues_t getValues(modelPointers_t pointers);
    bool IsEntityRendered(uint16_t index);
};
#endif