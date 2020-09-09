#include "Stylist.h"

std::string Stylist::GetSlotString(uint8_t slot)
{
    for (std::map<string, uint8_t>::iterator iter = mModelInfo.Slot.begin(); iter != mModelInfo.Slot.end(); iter++)
    {
        if (iter->second == slot)
            return iter->first;
    }

    return "Unknown";
}
uint8_t Stylist::GetModelTable(std::string text)
{
    for (std::map<string, uint8_t>::iterator iter = mModelInfo.Slot.begin(); iter != mModelInfo.Slot.end(); iter++)
    {
        if (_stricmp(text.c_str(), iter->first.c_str()) == 0)
            return iter->second;
    }
    return UINT8_MAX;
}
uint16_t Stylist::GetModelId(uint8_t table, std::string text)
{
    if (table == 0) //Race
    {
        for (std::map<string, uint16_t>::iterator iter = mModelInfo.Race.begin(); iter != mModelInfo.Race.end(); iter++)
        {
            if (_stricmp(text.c_str(), iter->first.c_str()) == 0)
                return iter->second;
        }
    }
    else if (table == 1) //Face
    {
        int32_t face = atoi(text.c_str());
        return min(max(face, 0), 15);
    }
    else if (table < 10)
    {
        if (_stricmp(text.c_str(), "none") == 0)
            return 0;

        uint16_t targetId = 0;
        if (IsPositiveInteger(text.c_str()))
        {
            int32_t itemId = atoi(text.c_str());
            if ((itemId > 0) && (itemId <= UINT16_MAX))
            {
                targetId = (uint16_t)itemId;               
            }
        }
        if (targetId == 0)
        {
            IItem* pResource = m_AshitaCore->GetResourceManager()->GetItemByName(text.c_str(), 0);
            if (pResource)
                targetId = (uint16_t)pResource->Id;
        }

        std::map<uint16_t, uint16_t>::iterator iter = mModelInfo.Equip[table - 2].find(targetId);
        if (iter == mModelInfo.Equip[table - 2].end())
            return UINT16_MAX;

        return iter->second;
    }

    return UINT16_MAX;
}
bool Stylist::IsPositiveInteger(const char* input)
{
    for (; input[0]; input++)
    {
        if (!isdigit(input[0]))
            return false;
    }
    return true;
}
std::string Stylist::FormatName(std::string name)
{
    std::string newName;
    for (int x = 0; x < name.length(); x++)
    {
        if (x == 0)
            newName += toupper(name[x]);
        else
            newName += tolower(name[x]);
    }
    return newName;
}