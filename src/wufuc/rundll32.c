#include "stdafx.h"
#include "callbacks.h"
#include "eventhelper.h"
#include "log.h"
#include "modulehelper.h"
#include "mutexhelper.h"
#include "ptrlist.h"
#include "registryhelper.h"
#include "servicehelper.h"
#include "wufuc.h"

const wchar_t m_szUnloadEventName[] = L"Global\\wufuc_UnloadEvent";

void CALLBACK RUNDLL32_StartW(HWND hwnd,
        HINSTANCE hinst,
        LPWSTR lpszCmdLine,
        int nCmdShow)
{
        ptrlist_t list;
        HANDLE hEvent;
        DWORD dwDesiredAccess;
        DWORD dwProcessId;
        bool Unloading = false;
        bool Lagging;
        SC_HANDLE hSCM;
        SC_HANDLE hService;
        SERVICE_NOTIFYW NotifyBuffer;
        void **values;
        uint32_t *tags;
        size_t count;
        DWORD e;
        DWORD r;
        size_t index;
        size_t crashes = 0;

        if ( !ptrlist_create(&list, 0, MAXIMUM_WAIT_OBJECTS) ) return;

        g_hMainMutex = mutex_create_new(true,
                L"Global\\25020063-b5a7-4227-9fdf-25cb75e8c645");
        if ( !g_hMainMutex ) goto destroy_list;

        hEvent = event_create_with_string_security_descriptor(
                true, false, m_szUnloadEventName, L"D:(A;;0x001F0003;;;BA)");
        if ( !hEvent ) goto release_mutex;
        if ( !ptrlist_add(&list, hEvent, 0) ) goto set_event;

        dwDesiredAccess = SERVICE_QUERY_STATUS | SERVICE_QUERY_CONFIG;
        do {
                Lagging = false;
                hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
                if ( !hSCM ) goto set_event;

                hService = OpenServiceW(hSCM, L"wuauserv", dwDesiredAccess);
                if ( !hService ) goto close_scm;

                if ( (dwDesiredAccess & SERVICE_QUERY_CONFIG) == SERVICE_QUERY_CONFIG ) {
                        dwDesiredAccess &= ~SERVICE_QUERY_CONFIG;

                        dwProcessId = svc_heuristic_process_id(hSCM, hService);
                        if ( dwProcessId )
                                wufuc_inject(dwProcessId, (LPTHREAD_START_ROUTINE)cb_start, &list);
                }
                ZeroMemory(&NotifyBuffer, sizeof NotifyBuffer);
                NotifyBuffer.dwVersion = SERVICE_NOTIFY_STATUS_CHANGE;
                NotifyBuffer.pfnNotifyCallback = (PFN_SC_NOTIFY_CALLBACK)cb_service_notify;
                NotifyBuffer.pContext = (PVOID)&list;
                while ( !Unloading && !Lagging ) {
                        e = NotifyServiceStatusChangeW(hService,
                                SERVICE_NOTIFY_START_PENDING | SERVICE_NOTIFY_RUNNING,
                                &NotifyBuffer);
                        switch ( e ) {
                        case ERROR_SUCCESS:
                                do {
                                        if ( !ptrlist_copy(&list, &values, &tags, &count) ) {
                                                Unloading = true;
                                                break;
                                        }
                                        r = WaitForMultipleObjectsEx((DWORD)count,
                                                values, FALSE, INFINITE, TRUE);

                                        if ( r >= WAIT_OBJECT_0 && r < WAIT_OBJECT_0 + count ) {
                                                // object signaled
                                                index = r - WAIT_OBJECT_0;
                                                if ( !index ) {
                                                        // Unload event
                                                        Unloading = true;
                                                } else {
                                                        // crash mutex was released cleanly
                                                        ptrlist_remove(&list, values[index]);
                                                        ReleaseMutex(values[index]);
                                                        CloseHandle(values[index]);
                                                }
                                        } else if ( r >= WAIT_ABANDONED_0 && r < WAIT_ABANDONED_0 + count ) {
                                                // object abandoned
                                                // crash mutex was abandoned, process has most likely crashed.
                                                index = r - WAIT_ABANDONED_0;

                                                ptrlist_remove(&list, values[index]);
                                                ReleaseMutex(values[index]);
                                                CloseHandle(values[index]);

                                                crashes++;
                                                trace(L"A process that wufuc injected into has crashed %Iu time%ls!!! (PID=%lu)",
                                                        crashes, crashes != 1 ? L"s" : L"", tags[index]);

                                                if ( crashes >= SVCHOST_CRASH_THRESHOLD ) {
                                                        trace(L"Crash threshold has been reached, disabling wufuc until next reboot!");
                                                        Unloading = true;
                                                }
                                        } else if ( r == WAIT_FAILED ) {
                                                trace(L"WTF - Unexpected result from wait function!!!");
                                                Unloading = true;
                                        }
                                        free(values);
                                        free(tags);
                                } while ( r != WAIT_IO_COMPLETION && !Unloading );
                                break;
                        case ERROR_SERVICE_NOTIFY_CLIENT_LAGGING:
                                trace(L"Client lagging!");
                                Lagging = true;
                                break;
                        default:
                                trace(L"NotifyServiceStatusChange failed, return value: %lu (%08X)", e);
                                Unloading = true;
                                break;
                        }
                }
                CloseServiceHandle(hService);
close_scm:
                CloseServiceHandle(hSCM);
        } while ( Lagging );
set_event:
        // signal event in case it is open in any other processes
        SetEvent(hEvent);
release_mutex:
        ReleaseMutex(g_hMainMutex);
destroy_list:
        ptrlist_for_each_stdcall(&list, CloseHandle);
        ptrlist_destroy(&list);
}

void CALLBACK RUNDLL32_UnloadW(
        HWND hwnd,
        HINSTANCE hinst,
        LPWSTR lpszCmdLine,
        int nCmdShow)
{
        HANDLE hEvent;

        hEvent = OpenEventW(EVENT_MODIFY_STATE, FALSE, m_szUnloadEventName);
        if ( hEvent ) {
                SetEvent(hEvent);
                CloseHandle(hEvent);
        }
}

void CALLBACK RUNDLL32_DeleteFileW(
        HWND hwnd,
        HINSTANCE hinst,
        LPWSTR lpszCmdLine,
        int nCmdShow)
{
        int argc;
        wchar_t **argv;

        argv = CommandLineToArgvW(lpszCmdLine, &argc);
        if ( argv ) {
                if ( !DeleteFileW(argv[0]) && GetLastError() == ERROR_ACCESS_DENIED )
                        MoveFileExW(argv[0], NULL, MOVEFILE_DELAY_UNTIL_REBOOT);

                LocalFree((HLOCAL)argv);
        }
}
