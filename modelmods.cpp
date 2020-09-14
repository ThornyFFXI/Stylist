#include "Stylist.h"

void Stylist::HandleModelPacket(modelPointers_t pointers, uint16_t index, std::string pName)
{
    //Save our values to the table.
    mState.RealEquip[index] = getValues(pointers);

    //Check if the character is already rendered.
    bool isRendered = IsEntityRendered(index);

    //If the character is already drawn, check if we need to block blinks for them.
    if (isRendered)
    {
        if (CheckBlinkBlock(index))
        {
            //If we're blocking blink, we can't apply any filters without forcing a blink, so we're done.
            ApplyBlinkBlock(index, pointers);
            return;
        }
    }

    //Look up the character's name, so we can see if they have a mask in config.
    std::map<std::string, charMask_t>::iterator iter;
    if (pName.length() >= 3)
        iter = mSettings.CharOverrides.find(pName);
    else if (isRendered)
        iter = mSettings.CharOverrides.find(std::string(m_AshitaCore->GetMemoryManager()->GetEntity()->GetName(index)));

    //Create a blank mask in case we didn't find it.
    charMask_t mask;
    //If we found an entry for the char, record that.
    if (iter != mSettings.CharOverrides.end())
        mask = iter->second;

    //Apply filters.
    ApplyModelChanges(pointers, mask, getValues(pointers));
}
bool Stylist::CheckBlinkBlock(uint16_t index)
{
    if (!IsEntityRendered(index))
        return false;

    if (index == m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0))
        return mSettings.NoBlinkSelf;

    int targetStatus = m_AshitaCore->GetMemoryManager()->GetTarget()->GetIsSubTargetActive();
    if (index == m_AshitaCore->GetMemoryManager()->GetTarget()->GetTargetIndex(0))
        return mSettings.NoBlinkTarget;

    if ((targetStatus == 1) && (index == m_AshitaCore->GetMemoryManager()->GetTarget()->GetTargetIndex(1)))
        return mSettings.NoBlinkTarget;

    return mSettings.NoBlinkOthers;
}
void Stylist::ApplyBlinkBlock(uint16_t index, modelPointers_t pointers)
{
    //Get a pointer to entity and use it to pull current appearance.
    Ashita::FFXI::entity_t* entity = m_AshitaCore->GetMemoryManager()->GetEntity()->GetRawEntity(index);
    modelValues_t currentValues = getValues(modelPointers_t(entity));

    //Write current appearance
    pointers.Write(currentValues);
}
void Stylist::ApplyModelChanges(modelPointers_t pointers, charMask_t overrides, modelValues_t realValues)
{
    //Start with our most current values for what the slot actually contains.
    modelValues_t workingValues = realValues;

    //Apply global slot overrides.
    for (int x = 0; x < 10; x++)
    {
        if (mSettings.DefaultOverride.SlotMasks[x].Override)
            workingValues.Values[x] = mSettings.DefaultOverride.SlotMasks[x].Value;
    }

    //Apply character-specific slot overrides.
    for (int x = 0; x < 10; x++)
    {
        if (overrides.SlotMasks[x].Override)
            workingValues.Values[x] = overrides.SlotMasks[x].Value;
    }
    
    /*
    * Apply model filters.  Note that we check against real values, because these take priority over generic character mappings.
    * We check if there is a character specific model mapping first, then we check for a global model mapping.
    */
    for (int x = 0; x < 10; x++)
    {
        std::map<uint16_t, singleFilter_t>::iterator iter = overrides.ModelFilters[x].find(realValues.Values[x]);
        if (iter != overrides.ModelFilters[x].end())
        {
            workingValues.Values[x] = iter->second.Model;
            continue;
        }
        iter = mSettings.DefaultOverride.ModelFilters[x].find(realValues.Values[x]);
        if (iter != mSettings.DefaultOverride.ModelFilters[x].end())
            workingValues.Values[x] = iter->second.Model;
    }

    //Write our finished values to the actual packet/entity.
    if (pointers != workingValues)
    {
        pointers.Write(workingValues);
    }
}

void Stylist::InitializeState()
{
    mState.myName    = "NO_NAME";
    uint16_t myIndex = m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0);
    if (myIndex > 0)
    {
        if (IsEntityRendered(myIndex))
        {
            mState.myIndex = myIndex;
            mState.myName  = m_AshitaCore->GetMemoryManager()->GetEntity()->GetName(myIndex);
        }
    }

    for (uint16_t x = 0x400; x < 0x700; x++)
    {
        if (!IsEntityRendered(x))
            continue;

        //Pull real appearance from memory.
        Ashita::FFXI::entity_t* entity = m_AshitaCore->GetMemoryManager()->GetEntity()->GetRawEntity(x);
        mState.RealEquip[x]            = getValues(modelPointers_t(entity));
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
        modelPointers_t pointers       = modelPointers_t(entity);
        modelValues_t values           = getValues(pointers);

        //Check our internal database, if we have a stored packet it's more reliable than current memory so apply it.
        std::map<uint16_t, modelValues_t>::iterator iter = mState.RealEquip.find(x);
        if (iter != mState.RealEquip.end())
            values = iter->second;

        //If we're forcing everything back to real models during an unload, this is all we need.
        if (ForceRealModel)
        {
            if (pointers != values)
            {
                pointers.Write(values);
            }
            continue;
        }

        //Look up the character's name, so we can see if they have a mask in config.
        std::map<std::string, charMask_t>::iterator iter2 = mSettings.CharOverrides.find(std::string(m_AshitaCore->GetMemoryManager()->GetEntity()->GetName(x)));

        //Create a blank mask in case they don't have one.
        charMask_t mask;

        //If we found an entry for the char, use it.
        if (iter2 != mSettings.CharOverrides.end())
            mask = iter2->second;
        
        //Apply our model changes.
        ApplyModelChanges(pointers, mask, values);
    }
}
void Stylist::UpdateOneModel(std::string name, charMask_t mask)
{
    for (int x = 0x400; x < 0x700; x++)
    {
        if (!IsEntityRendered(x))
            continue;
        if (std::string(m_AshitaCore->GetMemoryManager()->GetEntity()->GetName(x)) != name)
            continue;

        //Pull real appearance from memory.
        Ashita::FFXI::entity_t* entity = m_AshitaCore->GetMemoryManager()->GetEntity()->GetRawEntity(x);
        modelPointers_t pointers       = modelPointers_t(entity);
        modelValues_t values           = getValues(entity);

        //Check our internal database, if we have a stored packet it's more reliable than current memory so apply it.
        std::map<uint16_t, modelValues_t>::iterator iter = mState.RealEquip.find(x);
        if (iter != mState.RealEquip.end())
            values = iter->second;

        //Apply our model changes.
        ApplyModelChanges(pointers, mask, values);
        return;
    }
}

modelValues_t Stylist::getValues(modelPointers_t pointers)
{
    modelValues_t ret;
    ret.Values[0] = *(pointers.Race);
    ret.Values[1] = *(pointers.Face);
    for (int x = 0; x < 8; x++)
    {
        ret.Values[x + 2] = pointers.Equip[x];
    }
    return ret;
}

bool Stylist::IsEntityRendered(uint16_t index)
{
    if (m_AshitaCore->GetMemoryManager()->GetEntity()->GetRawEntity(index) == 0)
        return false;
    
    if ((m_AshitaCore->GetMemoryManager()->GetEntity()->GetRenderFlags0(index) & 0x200) == 0)
        return false;
    
    return ((m_AshitaCore->GetMemoryManager()->GetEntity()->GetRenderFlags0(index) & 0x4000) == 0);
}