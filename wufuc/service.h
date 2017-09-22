#ifndef SERVICE_H_INCLUDED
#define SERVICE_H_INCLUDED
#pragma once

#include <Windows.h>

BOOL GetServiceCommandLine(SC_HANDLE hSCManager, LPCTSTR lpServiceName, LPTSTR lpBinaryPathName, SIZE_T dwSize);
#endif
