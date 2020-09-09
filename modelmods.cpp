#include "Stylist.h"

void Stylist::ApplyModelChanges(appearance_t* appearance, uint16_t index, std::string pName)
{
    //Check if PC is already rendered.
    bool isRendered = IsEntityRendered(index);

    //Look up the character's name, so we can check our config.
    std::map<std::string, charmask_t>::iterator iter;
    if (pName.length() >= 3)
        iter  = mSettings.CharOverrides.find(pName);
    else if (isRendered)
        iter = mSettings.CharOverrides.find(std::string(m_AshitaCore->GetMemoryManager()->GetEntity()->GetName(index)));

    //If we found an entry for the char, process and mark overrides.
    if (iter != mSettings.CharOverrides.end())
    {
        if (iter->second.Override[0])
            appearance->Race = (uint8_t)iter->second.Params[0];

        if (iter->second.Override[1])
            appearance->Face = (uint8_t)iter->second.Params[1];

        for (int x = 2; x < 10; x++)
        {
            if (iter->second.Override[x])
                appearance->Equip[x - 2] = (uint16_t)iter->second.Params[x];
        }
    }

    //If we don't have to block blinks, we're done here.
    if (!CheckBlinkBlock(index))
        return;

    appearance->Face = m_AshitaCore->GetMemoryManager()->GetEntity()->GetLookHair(index);
    appearance->Race = m_AshitaCore->GetMemoryManager()->GetEntity()->GetRace(index);
    appearance->Equip[0] = m_AshitaCore->GetMemoryManager()->GetEntity()->GetLookHead(index);
    appearance->Equip[1] = m_AshitaCore->GetMemoryManager()->GetEntity()->GetLookBody(index);
    appearance->Equip[2] = m_AshitaCore->GetMemoryManager()->GetEntity()->GetLookHands(index);
    appearance->Equip[3] = m_AshitaCore->GetMemoryManager()->GetEntity()->GetLookLegs(index);
    appearance->Equip[4] = m_AshitaCore->GetMemoryManager()->GetEntity()->GetLookFeet(index);
    appearance->Equip[5] = m_AshitaCore->GetMemoryManager()->GetEntity()->GetLookMain(index);
    appearance->Equip[6] = m_AshitaCore->GetMemoryManager()->GetEntity()->GetLookSub(index);
    appearance->Equip[7] = m_AshitaCore->GetMemoryManager()->GetEntity()->GetLookRanged(index);
}
void Stylist::SaveInitialModels()
{
    for (int x = 0x400; x < 0x700; x++)
    {
        if (!IsEntityRendered(x))
            continue;

        //Pull real appearance from memory.
        Ashita::FFXI::entity_t* entity = m_AshitaCore->GetMemoryManager()->GetEntity()->GetRawEntity(x);
        uint16_t* initialAppearance     = (uint16_t*)((uint8_t*)entity + offsetof(Ashita::FFXI::entity_t, Look));

        //Add it to our table.
        appearance_t model;
        model.Race = m_AshitaCore->GetMemoryManager()->GetEntity()->GetRace(x);
        model.Face = initialAppearance[0];
        for (int x = 0; x < 8; x++)
        {
            model.Equip[x] = initialAppearance[x + 1];
        }

        mRealEquipValues[x] = model;
    }
}
void Stylist::UpdateAllModels(bool ForceRealModel)
{
    for (int x = 0x400; x < 0x700; x++)
    {
        if (!IsEntityRendered(x))
            continue;
        
        //Pull real appearance from memory.
        Ashita::FFXI::entity_t* entity = m_AshitaCore->GetMemoryManager()->GetEntity()->GetRawEntity(x);
        uint16_t* entityAppearance = (uint16_t*)((uint8_t*)entity + offsetof(Ashita::FFXI::entity_t, Look));
        uint8_t targetRace             = m_AshitaCore->GetMemoryManager()->GetEntity()->GetRace(x);
        uint16_t targetAppearance[9];
        for (int x = 0; x < 9; x++)
            targetAppearance[x] = entityAppearance[x];

        //If we have a stored packet for a more recent appearance, override with that.
        std::map<uint16_t, appearance_t>::iterator iter = mRealEquipValues.find(x);
        if (iter != mRealEquipValues.end())
        {
            targetRace          = iter->second.Race;
            targetAppearance[0] = iter->second.Face;
            for (int x = 0; x < 8; x++)
                targetAppearance[x + 1] = iter->second.Equip[x];
        }

        if (!ForceRealModel)
        {
            //If we have settings that determine a character should be wearing something else, override further with that.
            std::map<std::string, charmask_t>::iterator iter2 = mSettings.CharOverrides.find(std::string(m_AshitaCore->GetMemoryManager()->GetEntity()->GetName(x)));
            if (iter2 != mSettings.CharOverrides.end())
            {
                if (iter2->second.Override[0])
                    targetRace = (uint8_t)iter2->second.Params[0];

                if (iter2->second.Override[1])
                    targetAppearance[0] = (uint8_t)iter2->second.Params[1];

                for (int x = 2; x < 10; x++)
                {
                    if (iter2->second.Override[x])
                        targetAppearance[x - 1] = (uint16_t)iter2->second.Params[x];
                }
            }
        }

        //If the appearance we want doesn't match the entity's current appearance, write it and force a blink.
        if ((memcmp(entityAppearance, &targetAppearance, 18) != 0) || (m_AshitaCore->GetMemoryManager()->GetEntity()->GetRace(x) != targetRace))
        {
            memcpy(entityAppearance, &targetAppearance, 18);
            m_AshitaCore->GetMemoryManager()->GetEntity()->SetRace(x, targetRace);
            *((uint8_t*)entity + offsetof(Ashita::FFXI::entity_t, ModelUpdateFlags)) = 1;
        }
    }
}
void Stylist::UpdateOneModel(std::string name)
{
    for (int x = 0x400; x < 0x700; x++)
    {
        if (!IsEntityRendered(x))
            continue;
        if (std::string(m_AshitaCore->GetMemoryManager()->GetEntity()->GetName(x)) != name)
            continue;

        //Pull real appearance from memory.
        Ashita::FFXI::entity_t* entity = m_AshitaCore->GetMemoryManager()->GetEntity()->GetRawEntity(x);
        uint16_t* entityAppearance     = (uint16_t*)((uint8_t*)entity + offsetof(Ashita::FFXI::entity_t, Look));
        uint8_t targetRace             = m_AshitaCore->GetMemoryManager()->GetEntity()->GetRace(x);
        uint16_t targetAppearance[9];
        for (int x = 0; x < 9; x++)
            targetAppearance[x] = entityAppearance[x];

        //If we have a stored packet for a more recent appearance, override with that.
        std::map<uint16_t, appearance_t>::iterator iter = mRealEquipValues.find(x);
        if (iter != mRealEquipValues.end())
        {
            targetRace          = iter->second.Race;
            targetAppearance[0] = iter->second.Face;
            for (int x = 0; x < 8; x++)
                targetAppearance[x + 1] = iter->second.Equip[x];
        }

        //If we have settings that determine a character should be wearing something else, override further with that.
        std::map<std::string, charmask_t>::iterator iter2 = mSettings.CharOverrides.find(std::string(m_AshitaCore->GetMemoryManager()->GetEntity()->GetName(x)));
        if (iter2 != mSettings.CharOverrides.end())
        {
            if (iter2->second.Override[0])
                targetRace = (uint8_t)iter2->second.Params[0];

            if (iter2->second.Override[1])
                targetAppearance[0] = (uint8_t)iter2->second.Params[1];

            for (int x = 2; x < 10; x++)
            {
                if (iter2->second.Override[x])
                    targetAppearance[x - 1] = (uint16_t)iter2->second.Params[x];
            }
        }

        //If the appearance we want doesn't match the entity's current appearance, write it and force a blink.
        if ((memcmp(entityAppearance, &targetAppearance, 18) != 0) || (m_AshitaCore->GetMemoryManager()->GetEntity()->GetRace(x) != targetRace))
        {
            memcpy(entityAppearance, &targetAppearance, 18);
            m_AshitaCore->GetMemoryManager()->GetEntity()->SetRace(x, targetRace);
            *((uint8_t*)entity + offsetof(Ashita::FFXI::entity_t, ModelUpdateFlags)) = 1;
        }
        return;
    }
}
void Stylist::StoreRealValues(appearance_t * appearance, uint16_t index)
{
    mRealEquipValues[index] = *appearance;
}

bool Stylist::CheckBlinkBlock(uint16_t index)
{
    if (index == m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0))
        return mSettings.NoBlinkSelf;

    int targetStatus = m_AshitaCore->GetMemoryManager()->GetTarget()->GetIsSubTargetActive();
    if (index == m_AshitaCore->GetMemoryManager()->GetTarget()->GetTargetIndex(0))
        return mSettings.NoBlinkTarget;

    if ((targetStatus == 1) && (index == m_AshitaCore->GetMemoryManager()->GetTarget()->GetTargetIndex(1)))
        return mSettings.NoBlinkTarget;

    return mSettings.NoBlinkOthers;
}

bool Stylist::IsEntityRendered(uint16_t index)
{
    if (m_AshitaCore->GetMemoryManager()->GetEntity()->GetRawEntity(index) == 0)
        return false;
    
    if ((m_AshitaCore->GetMemoryManager()->GetEntity()->GetRenderFlags0(index) & 0x200) == 0)
        return false;
    
    return ((m_AshitaCore->GetMemoryManager()->GetEntity()->GetRenderFlags0(index) & 0x4000) == 0);
}