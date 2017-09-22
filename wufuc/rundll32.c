#include "service.h"
#include "tracing.h"

#include <Windows.h>
#include <tchar.h>

void CALLBACK Rundll32Unload(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow) {
    HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, _T("Global\\wufuc_UnloadEvent"));
    if ( hEvent ) {
        trace(_T("Setting unload event..."));
        SetEvent(hEvent);
        CloseHandle(hEvent);
    }
}
