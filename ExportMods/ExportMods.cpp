/*
    SFSE Plugin to export mods deployed with Vortex.
    By BitMage
    Portions of code completely stolen from the SFSE Minimal Plugin Project
    https://www.nexusmods.com/starfield/mods/2033
// */

#include "pch.h"
#include "operations.h"

#pragma region SFSE_SETUP_CODE
#define MAKE_VERSION(major, minor, build) ((((major)&0xFF)<<24)|(((minor)&0xFF)<<16)|(((build)&0xFFF)<<4))

typedef uint32_t PluginHandle;

typedef struct SFSEPluginVersionData_t
{
    uint32_t    dataVersion;
    uint32_t    pluginVersion;
    char        name[256];
    char        author[256];
    uint32_t    addressIndependence;
    uint32_t    structureIndependence;
    uint32_t    compatibleVersions[16];
    uint32_t    seVersionRequired;
    uint32_t    reservedNonBreaking;
    uint32_t    reservedBreaking;
} SFSEPluginVersionData;

typedef struct SFSEPluginInfo_t
{
    uint32_t	infoVersion;
    const char* name;
    uint32_t	version;
} SFSEPluginInfo;

typedef struct SFSEInterface_t
{
    uint32_t	sfseVersion;
    uint32_t	runtimeVersion;
    uint32_t	interfaceVersion;
    void* (*QueryInterface)(uint32_t id);
    PluginHandle(*GetPluginHandle)(void);
    SFSEPluginInfo* (*GetPluginInfo)(const char* name);
} SFSEInterface;

typedef struct SFSEMessage_t
{
    const char* sender;
    uint32_t    type;
    uint32_t    dataLen;
    void* data;
} SFSEMessage;

typedef void (*SFSEMessageEventCallback)(SFSEMessage* msg);

typedef struct SFSEMessagingInterface_t
{
    uint32_t    interfaceVersion;
    bool        (*RegisterListener)(PluginHandle listener, const char* sender, SFSEMessageEventCallback handler);
    bool	    (*Dispatch)(PluginHandle sender, uint32_t messageType, void* data, uint32_t dataLen, const char* receiver);
} SFSEMessagingInterface;

static void OnPostLoad();

static void MyMessageEventCallback(SFSEMessage* msg)
{
    if (msg->type == 0) // postload
    {
        OnPostLoad();
    }
}

extern "C" __declspec(dllexport) void SFSEPlugin_Preload(const SFSEInterface * sfse)
{
    PluginHandle my_handle = sfse->GetPluginHandle();
    SFSEMessagingInterface* msg = (SFSEMessagingInterface*)sfse->QueryInterface(1 /* messaging interface */);
    msg->RegisterListener(my_handle, "SFSE", MyMessageEventCallback);
}
#pragma endregion SFSE Setup Code

extern "C" __declspec(dllexport) SFSEPluginVersionData SFSEPlugin_Version =
{
        1, // SFSE api version
        1, // Plugin version
        "Mod Exporter",
        "Bit Mage",
        1, // AddressIndependence::Signatures
        1, // StructureIndependence::NoStructs
        {MAKE_VERSION(0, 0, 0), 0}, // Any game version
        0, // Does not rely on any SFSE version
        0, 0 // Reserved fields
};

// This is used to run the code without running the game.
// rundll32 ExportMods.dll,Manual_load
extern "C" __declspec(dllexport) void Manual_load() { OnPostLoad(); }

// Called once when SFSE sends the postload event after static initialization
static void OnPostLoad()
{
    if (!StartUp())
    {
        MessageBoxA(NULL, "Unable to create log!", "Mod Export Error", 0);
    }
}
