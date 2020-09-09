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
    InitModelInfo();
    LoadSettings();
    SaveInitialModels();
    UpdateAllModels(false);

    return true;
}

void Stylist::Release(void)
{
    UpdateAllModels(true);
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
            if (argcount < 5)
            {
                pOutput->error("Invalid format.  Correct usage: /sl add [Name] [Slot] [Model].");
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

            std::map<std::string, charmask_t>::iterator iter2 = mSettings.CharOverrides.find(name);
            if (iter2 != mSettings.CharOverrides.end())
            {
                iter2->second.Override[slot] = true;
                iter2->second.Params[slot]   = model;
                iter2->second.String[slot]   = args[4];
            }
            else
            {
                charmask_t newMask = {0};
                newMask.Override[slot] = true;
                newMask.Params[slot]       = model;
                newMask.String[slot] = args[4];
                mSettings.CharOverrides.insert(std::make_pair(name, newMask));
            }

            UpdateOneModel(name);
            pOutput->message("Filter added.");
        }

        else if (CheckArg(1, "remove"))
        {
            if (argcount < 4)
            {
                pOutput->error("Invalid format.  Correct usage: /sl remove [Name] [Slot].");
                return true;
            }
            std::string name(FormatName(args[2]));
            uint8_t slot = GetModelTable(args[3]);
            if (slot == UINT8_MAX)
            {
                pOutput->error_f("Invalid slot specified. [$H%s$R]", args[3].c_str());
                return true;
            }

            std::map<std::string, charmask_t>::iterator iter2 = mSettings.CharOverrides.find(name);
            if (iter2 != mSettings.CharOverrides.end())
            {
                iter2->second.Override[slot] = false;
                iter2->second.Params[slot]   = 0;
                pOutput->message("Filter removed.");
                UpdateOneModel(name);
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

            std::map<std::string, charmask_t>::iterator iter2 = mSettings.CharOverrides.find(name);
            if (iter2 != mSettings.CharOverrides.end())
            {
                mSettings.CharOverrides.erase(iter2);
                pOutput->message("Filters cleared.");
                UpdateOneModel(name);
            }
            else
            {
                pOutput->message("Filters did not exist.");
            }
        }

        else if (CheckArg(1, "reload"))
        {
            LoadSettings();
            UpdateAllModels(false);
            pOutput->message("Settings reloaded.");
        }

        else if (CheckArg(1, "export"))
        {
            SaveSettings();
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
        mRealEquipValues.clear();

        //Then apply our personal changes.
        StoreRealValues((appearance_t*)(modified + 0x44), m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0));
        ApplyModelChanges((appearance_t*)(modified + 0x44), Read16(modified, 0x08), std::string((const char*)(data + 0x84)));
    }

    //PC updates.
    if ((id == 0x0D) && (Read16(data, 0x48) != 0))
    {
        StoreRealValues((appearance_t*)(modified + 0x48), Read16(modified, 0x08));
        ApplyModelChanges((appearance_t*)(modified + 0x48), Read16(modified, 0x08), std::string((const char*)(data + 0x5A)));
	}

    //Self updates.
	if (id == 0x51)
    {
        StoreRealValues((appearance_t*)(modified + 0x04), m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0));
        ApplyModelChanges((appearance_t*)(modified + 0x04), m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberTargetIndex(0), std::string(m_AshitaCore->GetMemoryManager()->GetParty()->GetMemberName(0)));
	}

	return false;
}