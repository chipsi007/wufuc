#include <phnt_windows.h>
#include <shellapi.h>

void CALLBACK RUNDLL32_DeleteFileW(HWND hwnd, HINSTANCE hinst, LPWSTR lpszCmdLine, int nCmdShow)
{
        int argc;
        wchar_t **argv = CommandLineToArgvW(lpszCmdLine, &argc);

        if ( argv ) {
                if ( !DeleteFileW(argv[0]) 
                        && GetLastError() == ERROR_ACCESS_DENIED )
                        MoveFileExW(argv[0], NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

                LocalFree((HLOCAL)argv);
        }

}

void CALLBACK RUNDLL32_LegacyUnloadW(HWND hwnd, HINSTANCE hinst, LPWSTR lpszCmdLine, int nCmdShow)
{
        HANDLE Event = OpenEventW(EVENT_MODIFY_STATE, FALSE, L"Global\\wufuc_UnloadEvent");
        if ( Event ) {
                SetEvent(Event);
                CloseHandle(Event);
        }
}
