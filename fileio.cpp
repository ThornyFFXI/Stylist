#include "Stylist.h"
#include "thirdparty\rapidxml.hpp"
#include <fstream>

using namespace rapidxml;

void Stylist::InitModelInfo()
{
    //Create path to resource XML.
    char buffer[1024];
    sprintf_s(buffer, 1024, "%s\\resources\\stylist\\modelinfo.xml", m_AshitaCore->GetInstallPath());

    //If resource XML doesn't exist, throw an error.
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(buffer))
    {
        pOutput->error("Resource xml does not exist!");
        return;
    }

    std::ifstream Reader(buffer, ios::in | ios::binary | ios::ate);
    if (!Reader.is_open())
    {
        pOutput->error_f("Failed to read file.  [$H%s$R]", buffer);
        return;
    }

    long Size  = Reader.tellg();
    char* File = new char[Size + 1];
    Reader.seekg(0, ios::beg);
    Reader.read(File, Size);
    Reader.close();
    File[Size] = '\0';

    xml_document<>* XMLReader = new xml_document<>();
    try
    {
        XMLReader->parse<0>(File);
    }
    catch (const rapidxml::parse_error& e)
    {
        int line = static_cast<long>(std::count(File, e.where<char>(), '\n') + 1);
        stringstream error;
        error << "Parse error in resource XML[$H" << e.what() << "$R] at line $H" << line << "$R.";
        pOutput->error(error.str().c_str());
        delete XMLReader;
        delete[] File;
        return;
    }
    catch (...)
    {
        pOutput->error("Failed to parse resource XML.");
        delete XMLReader;
        delete[] File;
        return;
    }

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
void Stylist::LoadSettings(const char* fileName)
{
    //Reset settings.
    mSettings = settings_t();

    //Create path to settings XML.
    char buffer[1024];
    sprintf_s(buffer, 1024, "%sconfig\\stylist\\settings.xml", m_AshitaCore->GetInstallPath());

    //Ensure directories exist, making them if not.
    string makeDirectory(buffer);
    size_t nextDirectory = makeDirectory.find("\\");
    nextDirectory        = makeDirectory.find("\\", nextDirectory + 1);
    while (nextDirectory != string::npos)
    {
        string currentDirectory = makeDirectory.substr(0, nextDirectory + 1);
        if ((!CreateDirectory(currentDirectory.c_str(), NULL)) && (ERROR_ALREADY_EXISTS != GetLastError()))
        {
            pOutput->error_f("Could not find or create folder. [$H%s$R]", currentDirectory.c_str());
            return;
        }
        nextDirectory = makeDirectory.find("\\", nextDirectory + 1);
    }

    std::ifstream inputStream;
    sprintf_s(buffer, 1024, "%s", fileName);
    inputStream = ifstream(buffer, ios::in | ios::binary | ios::ate);
    if (!inputStream.is_open())
    {
        sprintf_s(buffer, 1024, "%s.xml", fileName);
        inputStream = ifstream(buffer, ios::in | ios::binary | ios::ate);
    }
    if (!inputStream.is_open())
    {
        sprintf_s(buffer, 1024, "%sconfig\\stylist\\%s", m_AshitaCore->GetInstallPath(), fileName);
        inputStream = ifstream(buffer, ios::in | ios::binary | ios::ate);
    }
    if (!inputStream.is_open())
    {
        sprintf_s(buffer, 1024, "%sconfig\\stylist\\%s.xml", m_AshitaCore->GetInstallPath(), fileName);
        inputStream = ifstream(buffer, ios::in | ios::binary | ios::ate);
    }
    if (!inputStream.is_open())
    {
        if (strcmp(fileName, "settings.xml") == 0)
        {
            SaveSettings("settings.xml");
            sprintf_s(buffer, 1024, "%sconfig\\stylist\\settings.xml", m_AshitaCore->GetInstallPath());
            mState.currentSettings = buffer;
        }
        else
        {
            pOutput->error_f("Failed to read file.  Loading defaults.  [$H%s$R]", buffer);
            LoadSettings("settings.xml");
        }
        return;
    }

    long Size  = inputStream.tellg();
    char* File = new char[Size + 1];
    inputStream.seekg(0, ios::beg);
    inputStream.read(File, Size);
    inputStream.close();
    File[Size]             = '\0';

    xml_document<>* XMLReader = new xml_document<>();
    try
    {
        XMLReader->parse<0>(File);
    }
    catch (const rapidxml::parse_error& e)
    {
        int line = static_cast<long>(std::count(File, e.where<char>(), '\n') + 1);
        stringstream error;
        error << "Parse error in settings XML [$H" << e.what() << "$R] at line $H" << line << "$R.";
        pOutput->error(error.str().c_str());
        delete XMLReader;
        delete[] File;
        return;
    }
    catch (...)
    {
        pOutput->error("Failed to parse settings XML.");
        delete XMLReader;
        delete[] File;
        return;
    }
    mState.currentSettings = buffer;

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

                (mSettings.ModelFilters[slot])[model] = singleFilter_t(targetmodel, attr->value(), Node->value());
            }
        }
    }

    delete[] File;
    delete XMLReader;
}

std::string Stylist::SaveSettings(const char* fileName)
{
    char buffer[1024];
    if (strstr(fileName, ".") == fileName + (strlen(fileName) - 4))
        sprintf_s(buffer, 1024, "%s\\config\\stylist\\%s", m_AshitaCore->GetInstallPath(), fileName);
    else
        sprintf_s(buffer, 1024, "%s\\config\\stylist\\%s.xml", m_AshitaCore->GetInstallPath(), fileName);

    ofstream outstream(buffer);
    if (!outstream.is_open())
    {
        pOutput->error_f("Failed to write file.  Loading defaults.  [%s]", buffer);
        LoadSettings("settings.xml");
        return mState.currentSettings;
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

    for (int x = 0; x < 10; x++)
    {
        for (std::map<uint16_t, singleFilter_t>::iterator iter = mSettings.ModelFilters[x].begin(); iter != mSettings.ModelFilters[x].end(); iter++)
        {
            outstream << "\t\t<filter item=\"" << iter->second.Initial << "\" slot=\"" << GetSlotString(x) << "\">" << iter->second.Target << "</filter>\n";
        }
    }

    outstream << "</stylist>";
    outstream.close();
    pOutput->message_f("Wrote settings XML. [$H%s$R]", buffer);
    return buffer;
}