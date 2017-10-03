#include "appverifier.h"
#include "hooks.h"
#include "callbacks.h"
#include "helpers.h"

#include <stdbool.h>

#include <phnt_windows.h>
#include <phnt.h>

RTL_VERIFIER_THUNK_DESCRIPTOR g_vfK32ThunkDescriptors[] = {
        { "RegQueryValueExW", NULL, (PVOID)&RegQueryValueExW_Hook },
        { "LoadLibraryExW", NULL, (PVOID)&LoadLibraryExW_Hook },
        { 0 } };
RTL_VERIFIER_DLL_DESCRIPTOR g_vfDllDescriptors[] = {
        { L"kernel32.dll", 0, NULL, g_vfK32ThunkDescriptors },
        { 0 } };
RTL_VERIFIER_DLL_DESCRIPTOR g_vfNullDllDescriptor[] = { { 0 } };
RTL_VERIFIER_PROVIDER_DESCRIPTOR g_vfProviderDescriptor = {
        sizeof(RTL_VERIFIER_PROVIDER_DESCRIPTOR),
        g_vfNullDllDescriptor,
        //(RTL_VERIFIER_DLL_LOAD_CALLBACK)&VerifierDllLoadCallback,
        //(RTL_VERIFIER_DLL_UNLOAD_CALLBACK)&VerifierDllUnloadCallback
};

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
        switch ( ul_reason_for_call ) {
        case DLL_PROCESS_ATTACH:
                LdrDisableThreadCalloutsForDll((PVOID)hModule);
                break;
        case DLL_PROCESS_DETACH:
                break;
        case DLL_PROCESS_VERIFIER:;
                if ( verify_winver(6, 1, 0, 0, 0, VER_EQUAL, VER_EQUAL, 0, 0, 0)
                        || verify_winver(6, 3, 0, 0, 0, VER_EQUAL, VER_EQUAL, 0, 0, 0) ) {

                        RTL_QUERY_REGISTRY_TABLE QueryTable[1];
                        RtlSecureZeroMemory(&QueryTable, sizeof(QueryTable));
                        QueryTable[0].Name = L"ImagePath";
                        QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT | RTL_QUERY_REGISTRY_REQUIRED;
                        UNICODE_STRING ImagePath;
                        RtlInitUnicodeString(&ImagePath, NULL);
                        QueryTable[0].EntryContext = &ImagePath;
                        NTSTATUS Status = RtlQueryRegistryValues(RTL_REGISTRY_SERVICES,
                                L"wuauserv",
                                QueryTable,
                                NULL,
                                NULL);

                        // TODO: check status and maybe fix implementation? idk...
                        if ( !RtlCompareUnicodeString(&NtCurrentPeb()->ProcessParameters->CommandLine, &ImagePath, TRUE) ) {
                                g_vfProviderDescriptor.ProviderDlls = g_vfDllDescriptors;
                        }
                }
                *(PRTL_VERIFIER_PROVIDER_DESCRIPTOR *)lpReserved = &g_vfProviderDescriptor;
                break;
        default:
                break;
        }

        return TRUE;
}
