#include "Stylist.h"
#include "..\common\thirdparty\rapidxml.hpp"

using namespace rapidxml;

void Stylist::InitModelInfo()
{
    //Create path to resource XML.
    char buffer[1024];
    sprintf_s(buffer, 1024, "%s\\resources\\stylist\\modelinfo.xml", m_AshitaCore->GetInstallPath());

    //Attempt to load and parse XML.
    char* File = NULL;
    xml_document<>* XMLReader = pSettings->LoadXml(buffer, File);
    if (XMLReader == NULL)
        return;

    //Read in model info from XML.
    xml_node<>* Node = XMLReader->first_node("modelinfo");
    if (Node)
        Node = Node->first_node();
    for (; Node; Node = Node->next_sibling())
    {
        if (_stricmp(Node->name(), "slot") == 0)
        {
            for (xml_node<>* SubNode = Node->first_node(); SubNode; SubNode = SubNode->next_sibling())
            {
                if (_stricmp(SubNode->name(), "entry") == 0)
                {
                    xml_attribute<>* attr1 = SubNode->first_attribute("index");
                    xml_attribute<>* attr2 = SubNode->first_attribute("text");
                    if ((attr1) && (attr2))
                    {
                        mModelInfo.Slot.insert(std::make_pair(std::string(attr2->value()), (uint8_t)atoi(attr1->value())));
                    }
                }
            }
        }

        else if (_stricmp(Node->name(), "race") == 0)
        {
            for (xml_node<>* SubNode = Node->first_node(); SubNode; SubNode = SubNode->next_sibling())
            {
                if (_stricmp(SubNode->name(), "entry") == 0)
                {
                    xml_attribute<>* attr1 = SubNode->first_attribute("model");
                    xml_attribute<>* attr2 = SubNode->first_attribute("text");
                    if ((attr1) && (attr2))
                    {
                        mModelInfo.Race.insert(std::make_pair(std::string(attr2->value()), (uint16_t)atoi(attr1->value())));
                    }
                }
            }
        }
        else
        {
            int32_t table = -1;
            if (_stricmp(Node->name(), "head") == 0)
                table = 0;
            else if (_stricmp(Node->name(), "body") == 0)
                table = 1;
            else if (_stricmp(Node->name(), "hands") == 0)
                table = 2;
            else if (_stricmp(Node->name(), "legs") == 0)
                table = 3;
            else if (_stricmp(Node->name(), "feet") == 0)
                table = 4;
            else if (_stricmp(Node->name(), "main") == 0)
                table = 5;
            else if (_stricmp(Node->name(), "sub") == 0)
                table = 6;
            else if (_stricmp(Node->name(), "range") == 0)
                table = 7;

            if (table == -1)
                continue;

            for (xml_node<>* SubNode = Node->first_node(); SubNode; SubNode = SubNode->next_sibling())
            {
                if (_stricmp(SubNode->name(), "entry") == 0)
                {
                    xml_attribute<>* attr1 = SubNode->first_attribute("model");
                    xml_attribute<>* attr2 = SubNode->first_attribute("itemid");
                    if ((attr1) && (attr2))
                    {
                        uint16_t modelMod = (uint16_t)atoi(attr1->value());
                        modelMod += (4096 * (table + 1));
                        mModelInfo.Equip[table].insert(std::make_pair((uint16_t)atoi(attr2->value()), modelMod));
                    }
                }
            }
        }
    }

    delete[] File;
    delete XMLReader;
}
void Stylist::LoadDefaultXml(bool forceReload)
{
    //Reset settings.
    mSettings = settings_t();

    //Get path to settings XML.
    std::string Path = pSettings->GetCharacterSettingsPath(mState.myName.c_str());
    if ((Path == pSettings->GetLoadedXmlPath()) && (!forceReload))
        return;

    if (Path == "FILE_NOT_FOUND")
    {
        Path = pSettings->GetDefaultSettingsPath();
        pSettings->CreateDirectories(Path.c_str());
        SaveSettings("default.xml");
    }
    else
    {
        //Yea, I know this wastes a filesystem::exists call, it doesn't really matter.
        LoadSettings(Path.c_str());
    }
}
bool Stylist::LoadSettings(const char* fileName)
{
    std::string SettingsFile = pSettings->GetInputSettingsPath(fileName);
    if (SettingsFile == "FILE_NOT_FOUND")
    {
        pOutput->error_f("Could not find settings file.  Loading defaults.  [$H%s$R]", fileName);
        LoadDefaultXml(true);
        return false;
    }

    //Reset settings.
    mSettings = settings_t();

    xml_document<>* XMLReader = pSettings->LoadSettingsXml(SettingsFile);
    mSettings.DefaultOverride = charMask_t();

    xml_node<>* Node = XMLReader->first_node("stylist");
    if (Node)
        Node = Node->first_node();
    for (; Node; Node = Node->next_sibling())
    {
        if (_stricmp(Node->name(), "settings") == 0)
        {
            for (xml_node<>* SubNode = Node->first_node(); SubNode; SubNode = SubNode->next_sibling())
            {
                if (_stricmp(SubNode->name(), "noblinkself") == 0)
                {
                    if (_stricmp(SubNode->value(), "false") == 0)
                        mSettings.NoBlinkSelf = false;
                }
                if (_stricmp(SubNode->name(), "noblinktarget") == 0)
                {
                    if (_stricmp(SubNode->value(), "false") == 0)
                        mSettings.NoBlinkTarget = false;
                }
                if (_stricmp(SubNode->name(), "noblinkothers") == 0)
                {
                    if (_stricmp(SubNode->value(), "false") == 0)
                        mSettings.NoBlinkOthers = false;
                }
            }
        }

        else if (_stricmp(Node->name(), "player") == 0)
        {
            xml_attribute<>* attr = Node->first_attribute("name");
            if (!attr)
                continue;

            std::string name(FormatName(attr->value()));
            charMask_t mask;

            for (xml_node<>* SubNode = Node->first_node(); SubNode; SubNode = SubNode->next_sibling())
            {
                if (_stricmp(SubNode->name(), "filter") == 0)
                {
                    attr = SubNode->first_attribute("item");
                    xml_attribute<>* attr2 = SubNode->first_attribute("slot");
                    if (attr && attr2)
                    {
                        uint8_t slot = GetModelTable(attr2->value());
                        if (slot == UINT8_MAX)
                            continue;

                        uint16_t model = GetModelId(slot, attr->value());
                        if (model == UINT16_MAX)
                            continue;

                        uint16_t targetmodel = GetModelId(slot, SubNode->value());
                        if (targetmodel == UINT16_MAX)
                            continue;

                        (mask.ModelFilters[slot])[model] = singleFilter_t(targetmodel, attr->value(), SubNode->value());
                    }
                }
                else
                {
                    uint8_t slot = GetModelTable(SubNode->name());
                    if (slot == UINT8_MAX)
                        continue;

                    uint16_t model = GetModelId(slot, SubNode->value());
                    if (model == UINT16_MAX)
                        continue;

                    mask.SlotMasks[slot] = singleMask_t(true, model, SubNode->value());
                }
            }
            mSettings.CharOverrides.insert(std::make_pair(name, mask));
        }

        else if (_stricmp(Node->name(), "filter") == 0)
        {
            xml_attribute<>* attr                   = Node->first_attribute("item");
            xml_attribute<>* attr2 = Node->first_attribute("slot");
            if (attr && attr2)
            {
                uint8_t slot = GetModelTable(attr2->value());
                if (slot == UINT8_MAX)
                    continue;

                uint16_t model = GetModelId(slot, attr->value());
                if (model == UINT16_MAX)
                    continue;

                uint16_t targetmodel = GetModelId(slot, Node->value());
                if (targetmodel == UINT16_MAX)
                    continue;

                (mSettings.DefaultOverride.ModelFilters[slot])[model] = singleFilter_t(targetmodel, attr->value(), Node->value());
            }
        }

        else
        {
            uint8_t slot = GetModelTable(Node->name());
            if (slot == UINT8_MAX)
                continue;

            uint16_t model = GetModelId(slot, Node->value());
            if (model == UINT16_MAX)
                continue;

            mSettings.DefaultOverride.SlotMasks[slot] = singleMask_t(true, model, Node->value());
        }
    }

    pOutput->message_f("Settings loaded.  [$H%s$R]", pSettings->GetLoadedXmlPath().c_str());
    return true;
}
void Stylist::SaveSettings(const char* fileName)
{
    std::string Path = pSettings->GetInputWritePath(fileName);

    ofstream outstream(Path.c_str());
    if (!outstream.is_open())
    {
        pOutput->error_f("Failed to write file.  [%s]", Path.c_str());
        return;
    }

    outstream << "<stylist>\n";
    outstream << "\n\t<settings>\n";
    outstream << "\t\t<noblinkself>";
    if (mSettings.NoBlinkSelf)
        outstream << "true";
    else
        outstream << "false";
    outstream << "</noblinkself>\n";
    outstream << "\t\t<noblinktarget>";
    if (mSettings.NoBlinkTarget)
        outstream << "true";
    else
        outstream << "false";
    outstream << "</noblinktarget>\n";
    outstream << "\t\t<noblinkothers>";
    if (mSettings.NoBlinkOthers)
        outstream << "true";
    else
        outstream << "false";
    outstream << "</noblinkothers>\n";
    outstream << "\t</settings>\n\n";

    bool WroteFilter = false;
    for (int x = 0; x < 10; x++)
    {
        if (!mSettings.DefaultOverride.SlotMasks[x].Override)
            continue;

        outstream << "\t<" << GetSlotString(x) << ">" << mSettings.DefaultOverride.SlotMasks[x].Text << "</" << GetSlotString(x) << ">\n";
        WroteFilter = true;
    }
    if (WroteFilter)
    {
        outstream << "\n";
        WroteFilter = false;
    }

    for (int x = 0; x < 10; x++)
    {
        for (std::map<uint16_t, singleFilter_t>::iterator iter = mSettings.DefaultOverride.ModelFilters[x].begin(); iter != mSettings.DefaultOverride.ModelFilters[x].end(); iter++)
        {
            outstream << "\t<filter item=\"" << iter->second.Initial << "\" slot=\"" << GetSlotString(x) << "\">" << iter->second.Target << "</filter>\n";
            WroteFilter = true;
        }
    }
    if (WroteFilter)
    {
        outstream << "\n";
        WroteFilter = false;
    }

    for (std::map<std::string, charMask_t>::iterator iter = mSettings.CharOverrides.begin(); iter != mSettings.CharOverrides.end(); iter++)
    {
        outstream << "\t<player name=\"" << iter->first << "\">\n";

        for (int x = 0; x < 10; x++)
        {
            if (!iter->second.SlotMasks[x].Override)
                continue;

            outstream << "\t\t<" << GetSlotString(x) << ">" << iter->second.SlotMasks[x].Text << "</" << GetSlotString(x) << ">\n";
        }

        for (int x = 0; x < 10; x++)
        {
            for (std::map<uint16_t, singleFilter_t>::iterator iter2 = iter->second.ModelFilters[x].begin(); iter2 != iter->second.ModelFilters[x].end(); iter2++)
            {
                outstream << "\t\t<filter item=\"" << iter2->second.Initial << "\" slot=\"" << GetSlotString(x) << "\">" << iter2->second.Target << "</filter>\n";
            }
        }

        outstream << "\t</player>\n\n";
    }

    outstream << "</stylist>";
    outstream.close();
    pOutput->message_f("Wrote settings XML. [$H%s$R]", Path.c_str());
}