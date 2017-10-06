#include "appverifier.h"
#include "hooks.h"
#include "callbacks.h"
#include "helpers.h"

#include <stdbool.h>

#include <phnt_windows.h>
#include <phnt.h>

RTL_VERIFIER_THUNK_DESCRIPTOR g_vfThunkDescriptors[] = {
        { "RegQueryValueExW", NULL, (PVOID)&RegQueryValueExW_Hook },
        { "LoadLibraryExW", NULL, (PVOID)&LoadLibraryExW_Hook },
        { 0 } };

RTL_VERIFIER_DLL_DESCRIPTOR g_vfDllDescriptors[2];

RTL_VERIFIER_PROVIDER_DESCRIPTOR g_vfProviderDescriptor = {
        sizeof(RTL_VERIFIER_PROVIDER_DESCRIPTOR),
        g_vfDllDescriptors/*,
        (RTL_VERIFIER_DLL_LOAD_CALLBACK)&VerifierDllLoadCallback,
        (RTL_VERIFIER_DLL_UNLOAD_CALLBACK)&VerifierDllUnloadCallback*/ };

LPFN_REGQUERYVALUEEXW *g_plpfnRegQueryValueExW;
LPFN_LOADLIBRARYEXW *g_plpfnLoadLibraryExW;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
        switch ( ul_reason_for_call ) {
        case DLL_PROCESS_ATTACH:
                LdrDisableThreadCalloutsForDll((PVOID)hModule);
                break;
        case DLL_PROCESS_DETACH:
                break;
        case DLL_PROCESS_VERIFIER:
                if ( verify_win7() || verify_win81 ) {
                        UNICODE_STRING ImagePath;
                        RtlInitUnicodeString(&ImagePath, NULL);

                        RTL_QUERY_REGISTRY_TABLE QueryTable[2];
                        RtlSecureZeroMemory(&QueryTable, sizeof(QueryTable));
                        QueryTable[0].Name = L"ImagePath";
                        QueryTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT;
                        QueryTable[0].EntryContext = &ImagePath;

                        if ( RtlQueryRegistryValues(RTL_REGISTRY_SERVICES, L"wuauserv", QueryTable, NULL, NULL) == STATUS_SUCCESS
                                && !RtlCompareUnicodeString(&NtCurrentPeb()->ProcessParameters->CommandLine, &ImagePath, TRUE) ) {

                                if ( verify_win7() )
                                        g_vfDllDescriptors[0].DllName = L"kernel32.dll";
                                else if ( verify_win81() )
                                        g_vfDllDescriptors[0].DllName = L"kernelbase.dll";
                                
                                g_vfDllDescriptors[0].DllThunks = g_vfThunkDescriptors;

                                g_plpfnRegQueryValueExW = (LPFN_REGQUERYVALUEEXW *)&g_vfThunkDescriptors[0].ThunkOldAddress;
                                g_plpfnLoadLibraryExW = (LPFN_LOADLIBRARYEXW *)&g_vfThunkDescriptors[1].ThunkOldAddress;
                        }
                        RtlFreeUnicodeString(&ImagePath);
                }
                *(PRTL_VERIFIER_PROVIDER_DESCRIPTOR *)lpReserved = &g_vfProviderDescriptor;
                break;
        default:
                break;
        }

        return TRUE;
}
