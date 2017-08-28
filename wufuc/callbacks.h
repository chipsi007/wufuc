#ifndef CALLBACKS_H
#define CALLBACKS_H
#pragma once

#include "ntdllhelper.h"

#include <Windows.h>

DWORD WINAPI ThreadProcCallback(LPVOID lpParam);
VOID CALLBACK LdrDllNotificationCallback(ULONG NotificationReason, PCLDR_DLL_NOTIFICATION_DATA NotificationData, PVOID Context);
#endif
