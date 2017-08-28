#include <Windows.h>
#include <Msiquery.h>
#include <tchar.h>

__declspec(dllexport)
UINT __stdcall AIHelper_SetUnloadEvent(MSIHANDLE hInstall) {
    HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, _T("Global\\wufuc_UnloadEvent"));
    if ( hEvent ) {
        SetEvent(hEvent);
        CloseHandle(hEvent);
    }
    return 1;
}
