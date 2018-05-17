#include "stdafx.h"
#include "stdafx.h"
#include "memory.h"

BOOL WriteProcessMemoryBOOL(HANDLE hProcess, LPBOOL lpBaseAddress, BOOL bValue)
{
        return WriteProcessMemory(hProcess, lpBaseAddress, &bValue, sizeof bValue, NULL);
}