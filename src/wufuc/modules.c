#include "stdafx.h"
#include "modules.h"

HMODULE Toolhelp32GetModuleHandle(DWORD th32ProcessID, LPCWSTR lpModuleName)
{
        HANDLE Snapshot;
        MODULEENTRY32W me = { sizeof me };
        HMODULE result = NULL;

        Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, th32ProcessID);
        if ( !Snapshot ) return NULL;

        if ( Module32FirstW(Snapshot, &me) ) {
                do {
                        if ( !_wcsicmp(me.szExePath, lpModuleName) ) {
                                result = me.hModule;
                                break;
                        }
                } while ( Module32NextW(Snapshot, &me) );
        }
        CloseHandle(Snapshot);

        return result;
}
