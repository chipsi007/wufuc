#pragma once

VOID DetourIAT(HMODULE hModule, LPSTR lpFuncName, LPVOID *lpOldAddress, LPVOID lpNewAddress);

#define DETOUR_IAT(x, y) \
	LPVOID __LPORIGINAL##y; \
	DetourIAT(x, #y, &__LPORIGINAL##y, &_##y)

#define RESTORE_IAT(x, y) \
	DetourIAT(x, #y, NULL, __LPORIGINAL##y)

LPVOID *FindIAT(HMODULE hModule, LPSTR lpFuncName);

BOOL FindPattern(LPCBYTE lpBytes, SIZE_T nNumberOfBytes, LPSTR lpszPattern, SIZE_T nStart, SIZE_T *lpOffset);

BOOL InjectLibrary(HANDLE hProcess, LPCTSTR lpLibFileName, DWORD cb);

VOID SuspendProcess(HANDLE *lphThreads, SIZE_T dwSize, SIZE_T *lpcb);

VOID ResumeAndCloseThreads(HANDLE *lphThreads, SIZE_T dwSize);
