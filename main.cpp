#include "Stylist.h"

__declspec(dllexport) IPlugin* __stdcall expCreatePlugin(const char* args)
{
    UNREFERENCED_PARAMETER(args);

    return (IPlugin*)(new Stylist());
}

__declspec(dllexport) double __stdcall expGetInterfaceVersion(void)
{
    return ASHITA_INTERFACE_VERSION;
}

bool Stylist::Initialize(IAshitaCore* core, ILogManager* logger, const uint32_t id)
{
    m_AshitaCore = core;
    m_LogManager = logger;
    m_PluginId = id;
    pOutput      = new OutputHelpers(core, logger, this->GetName());
    pSettings    = new SettingsHelper(core, pOutput, this->GetName());
    InitModelInfo();
    InitializeState();
    LoadDefaultXml(false);
    UpdateAllModels(false);

    return true;
}

void Stylist::Release(void)
{
    UpdateAllModels(true);
    delete pSettings;
    delete pOutput;
}

bool Stylist::HandleCommand(int32_t mode, const char* command, bool injected)
{
	UNREFERENCED_PARAMETER(mode);
	UNREFERENCED_PARAMETER(injected);
    std::vector<string> args;
    int argcount = Ashita::Commands::GetCommandArgs(command, &args);

    if ((CheckArg(0, "/sl")) || (CheckArg(0, "/stylist")))
    {
        if (CheckArg(1, "blink"))
        {
            UpdateAllModels(false);
        }

        else if (CheckArg(1, "self"))
        {
            if (CheckArg(2, "on"))
                mSettings.NoBlinkSelf = true;
            else if (CheckArg(2, "off"))
                mSettings.NoBlinkSelf = false;
            else
                mSettings.NoBlinkSelf = !mSettings.NoBlinkSelf;

            pOutput->message_f("Self blink blocking $H%s$R.", mSettings.NoBlinkSelf ? "enabled" : "disabled"); 
        }

        else if (CheckArg(1, "target"))
        {
            if (CheckArg(2, "on"))
                mSettings.NoBlinkTarget = true;
            else if (CheckArg(2, "off"))
                mSettings.NoBlinkTarget = false;
            else
                mSettings.NoBlinkTarget = !mSettings.NoBlinkTarget;

            pOutput->message_f("Target blink blocking $H%s$R.", mSettings.NoBlinkTarget ? "enabled" : "disabled");
        }

        else if (CheckArg(1, "others"))
        {
            if (CheckArg(2, "on"))
                mSettings.NoBlinkOthers = true;
            else if (CheckArg(2, "off"))
                mSettings.NoBlinkOthers = false;
            else
                mSettings.NoBlinkOthers = !mSettings.NoBlinkOthers;

            pOutput->message_f("Others blink blocking $H%s$R.", mSettings.NoBlinkOthers ? "enabled" : "disabled");
        }

        else if (CheckArg(1, "add"))
        {
            if (argcount < 4)
            {
                pOutput->error("Invalid format.  Correct usage: /sl add [Slot] [Model].");
                return true;
            }
            uint8_t slot = GetModelTable(args[2]);
            if (slot == UINT8_MAX)
            {
                pOutput->error_f("Invalid slot specified. [$H%s$R]", args[2].c_str());
                return true;            
            }
            uint16_t model = GetModelId(slot, args[3]);
            if (model == UINT16_MAX)
            {
                pOutput->error_f("Invalid model specified. [$H%s$R]", args[3].c_str());
                return true;
            }

            //Create our mask.
            singleMask_t newMask;
            newMask.Override                                  = true;
            newMask.Value                                     = model;
            newMask.Text                                      = args[3];

            mSettings.DefaultOverride.SlotMasks[slot] = newMask;
            UpdateAllModels(false);
            pOutput->message("Filter added.");
        }

        else if (CheckArg(1, "remove"))
        {
            if (argcount < 3)
            {
                pOutput->error("Invalid format.  Correct usage: /sl remove [Slot].");
                return true;
            }
            uint8_t slot = GetModelTable(args[2]);
            if (slot == UINT8_MAX)
            {
                pOutput->error_f("Invalid slot specified. [$H%s$R]", args[2].c_str());
                return true;
            }

            mSettings.DefaultOverride.SlotMasks[slot] = singleMask_t();
            pOutput->message("Filter removed.");
            UpdateAllModels(false);
        }

        else if (CheckArg(1, "addc"))
        {
            if (argcount < 5)
            {
                pOutput->error("Invalid format.  Correct usage: /sl addc [Name] [Slot] [Model].");
                return true;
            }
            std::string name(FormatName(args[2]));
            uint8_t slot = GetModelTable(args[3]);
            if (slot == UINT8_MAX)
            {
                pOutput->error_f("Invalid slot specified. [$H%s$R]", args[3].c_str());
                return true;
            }
            uint16_t model = GetModelId(slot, args[4]);
            if (model == UINT16_MAX)
            {
                pOutput->error_f("Invalid model specified. [$H%s$R]", args[4].c_str());
                return true;
            }

            //Create our mask.
            singleMask_t newMask;
            newMask.Override = true;
            newMask.Value    = model;
            newMask.Text     = args[4];

            //Save our mask and update our model.
            std::map<std::string, charMask_t>::iterator iter2 = mSettings.CharOverrides.find(name);
            if (iter2 != mSettings.CharOverrides.end())
            {
                iter2->second.SlotMasks[slot] = newMask;
                UpdateOneModel(name, iter2->second);
            }
            else
            {
                charMask_t newChar;
                newChar.SlotMasks[slot] = newMask;
                mSettings.CharOverrides.insert(std::make_pair(name, newChar));
                UpdateOneModel(name, newChar);
            }

            pOutput->message("Filter added.");
        }

        else if (CheckArg(1, "removec"))
        {
            if (argcount < 4)
            {
                pOutput->error("Invalid format.  Correct usage: /sl removec [Name] [Slot].");
                return true;
            }
            std::string name(FormatName(args[2]));
            uint8_t slot = GetModelTable(args[3]);
            if (slot == UINT8_MAX)
            {
                pOutput->error_f("Invalid slot specified. [$H%s$R]", args[3].c_str());
                return true;
            }

            std::map<std::string, charMask_t>::iterator iter2 = mSettings.CharOverrides.find(name);
            if (iter2 != mSettings.CharOverrides.end())
            {
                iter2->second.SlotMasks[slot] = singleMask_t();
                pOutput->message("Filter removed.");
                UpdateOneModel(name, iter2->second);
            }
            else
            {
                pOutput->message("Filter did not exist.");
            }
        }

        else if (CheckArg(1, "addmodel"))
        {
            if (argcount < 5)
            {
                pOutput->error("Invalid format.  Correct usage: /sl addmodel [Slot] [Initial Model] [Target Model].");
                return true;
            }
            uint8_t slot = GetModelTable(args[2]);
            if (slot == UINT8_MAX)
            {
                pOutput->error_f("Invalid slot specified. [$H%s$R]", args[2].c_str());
                return true;
            }
            uint16_t model = GetModelId(slot, args[3]);
            if (model == UINT16_MAX)
            {
                pOutput->error_f("Invalid model specified. [$H%s$R]", args[3].c_str());
                return true;
            }
            uint16_t newModel = GetModelId(slot, args[4]);
            if (newModel == UINT16_MAX)
            {
                pOutput->error_f("Invalid model specified. [$H%s$R]", args[4].c_str());
                return true;
            }

            //Add to settings.
            (mSettings.DefaultOverride.ModelFilters[slot])[model] = singleFilter_t(newModel, args[3], args[4]);
            UpdateAllModels(false);
            pOutput->message("Filter added.");
        }

        else if (CheckArg(1, "removemodel"))
        {
            if (argcount < 4)
            {
                pOutput->error("Invalid format.  Correct usage: /sl removemodel [Slot] [Model].");
                return true;
            }
            uint8_t slot = GetModelTable(args[2]);
            if (slot == UINT8_MAX)
            {
                pOutput->error_f("Invalid slot specified. [$H%s$R]", args[2].c_str());
                return true;
            }
            uint16_t model = GetModelId(slot, args[3]);
            if (model == UINT16_MAX)
            {
                pOutput->error_f("Invalid model specified. [$H%s$R]", args[3].c_str());
                return true;
            }
            std::map<uint16_t, singleFilter_t>::iterator iter = mSettings.DefaultOverride.ModelFilters[slot].find(model);
            if (iter != mSettings.DefaultOverride.ModelFilters[slot].end())
            {
                mSettings.DefaultOverride.ModelFilters->erase(iter);
                UpdateAllModels(false);
                pOutput->message("Filter removed.");
            }
            else
            {
                pOutput->message("Filter did not exist.");
            }
        }

        else if (CheckArg(1, "addmodelc"))
        {
            if (argcount < 5)
            {
                pOutput->error("Invalid format.  Correct usage: /sl addmodelc [Name] [Slot] [Initial Model] [Target Model].");
                return true;
            }
            uint8_t slot = GetModelTable(args[3]);
            if (slot == UINT8_MAX)
            {
                pOutput->error_f("Invalid slot specified. [$H%s$R]", args[3].c_str());
                return true;
            }
            uint16_t model = GetModelId(slot, args[4]);
            if (model == UINT16_MAX)
            {
                pOutput->error_f("Invalid model specified. [$H%s$R]", args[4].c_str());
                return true;
            }
            uint16_t newModel = GetModelId(slot, args[5]);
            if (newModel == UINT16_MAX)
            {
                pOutput->error_f("Invalid model specified. [$H%s$R]", args[5].c_str());
                return true;
            }

            singleFilter_t newFilter = singleFilter_t(newModel, args[4], args[5]);
            
            std::string name(FormatName(args[2]));
            std::map<std::string, charMask_t>::iterator iter = mSettings.CharOverrides.find(name);
            if (iter == mSettings.CharOverrides.end())
            {
                charMask_t mask;
                (mask.ModelFilters[slot])[model] = newFilter;
                mSettings.CharOverrides[name]    = mask;
            }
            else
            {
                (iter->second.ModelFilters[slot])[model] = newFilter;
            }

            UpdateAllModels(false);
            pOutput->message("Filter added.");
        }

        else if (CheckArg(1, "removemodelc"))
        {
            if (argcount < 4)
            {
                pOutput->error("Invalid format.  Correct usage: /sl removemodelc [Name] [Slot] [Model].");
                return true;
            }
            std::string name(FormatName(args[2]));
            std::map<std::string, charMask_t>::iterator iter = mSettings.CharOverrides.find(name);
            if (iter == mSettings.CharOverrides.end())
            {
                pOutput->error_f("Invalid char specified. [$H%s$R]", args[2].c_str());
                return true;
            }
            uint8_t slot = GetModelTable(args[3]);
            if (slot == UINT8_MAX)
            {
                pOutput->error_f("Invalid slot specified. [$H%s$R]", args[3].c_str());
                return true;
            }
            uint16_t model = GetModelId(slot, args[4]);
            if (model == UINT16_MAX)
            {
                pOutput->error_f("Invalid model specified. [$H%s$R]", args[4].c_str());
                return true;
            }
            std::map<uint16_t, singleFilter_t>::iterator iter2 = iter->second.ModelFilters[slot].find(model);
            if (iter2 != iter->second.ModelFilters[slot].end())
            {
                iter->second.ModelFilters->erase(iter2);
                UpdateAllModels(false);
                pOutput->message("Filter removed.");
            }
            else
            {
                pOutput->message("Filter did not exist.");
            }
        }

        else if (CheckArg(1, "clear"))
        {
            if (argcount < 3)
            {
                pOutput->error("Invalid format.  Correct usage: /sl clear [Name].");
                return true;
            }
            std::string name(FormatName(args[2]));

            std::map<std::string, charMask_t>::iterator iter2 = mSettings.CharOverrides.find(name);
            if (iter2 != mSettings.CharOverrides.end())
            {
                mSettings.CharOverrides.erase(iter2);
                pOutput->message("Filters cleared.");
                UpdateOneModel(name, charMask_t());
            }
            else
            {
                pOutput->message("Filters did not exist.");
            }
        }

        else if (CheckArg(1, "load"))
        {
            if (argcount == 2)
                LoadDefaultXml(true);
            else
                LoadSettings(args[2].c_str());

            UpdateAllModels(false);
        }

        else if (CheckArg(1, "reload"))
        {
            LoadSettings(pSettings->GetLoadedXmlPath().c_str());
            UpdateAllModels(false);
        }

        else if (CheckArg(1, "newconfig"))
        {
            if (argcount < 3)
            {
                pOutput->error("Invalid format.  Correct usage: /sl newconfig [Name].");
                return true;
            }
            mSettings = settings_t();
            UpdateAllModels(false);
            SaveSettings(args[2].c_str());
        }

        else if (CheckArg(1, "export"))
        {
            if (argcount == 2)
                SaveSettings(pSettings->GetLoadedXmlPath().c_str());
            else
            {
                SaveSettings(args[2].c_str());
            }
        }

        return true;
    }

	return false;
}

bool Stylist::HandleIncomingPacket(uint16_t id, uint32_t size, const uint8_t* data, uint8_t* modified, uint32_t sizeChunk, const uint8_t* dataChunk, bool injected, bool blocked)
{
	UNREFERENCED_PARAMETER(size);
	UNREFERENCED_PARAMETER(sizeChunk);
	UNREFERENCED_PARAMETER(dataChunk);
	UNREFERENCED_PARAMETER(injected);
	UNREFERENCED_PARAMETER(blocked);

    //Zonein
    if (id == 0x00A)
    {
        //Clear values on a zonein, since indices will have changed.
        mState.RealEquip.clear();

        //Update player name and index.
        mState.myIndex = Read16(modified, 0x08);
        std::string Name = std::string((const char*)(data + 0x84));
        if (Name != mState.myName)
        {
            mState.myName = Name;
            LoadDefaultXml(false);
        }

        //Then apply our personal changes.
        HandleModelPacket(modelPointers_t(modified + 0x44), mState.myIndex, mState.myName);
    }

    //PC updates.
    if ((id == 0x0D) && (Read16(data, 0x48) != 0))
    {
        const char* name = (const char*)(data + 0x5A);
        if (Ashita::BinaryData::UnpackBitsBE((uint8_t*)data, 0x0A, 3, 1) != 0)
        {
            HandleModelPacket(modelPointers_t(modified + 0x48), Read16(modified, 0x08), std::string(name));
        }
	}

    //Self updates.
	if (id == 0x51)
    {
        HandleModelPacket(modelPointers_t(modified + 0x04), mState.myIndex, mState.myName);
	}

	return false;
}