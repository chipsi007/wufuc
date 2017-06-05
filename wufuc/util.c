#include <Windows.h>
#include <stdio.h>
#include <VersionHelpers.h>
#include <TlHelp32.h>
#include <tchar.h>
#include "util.h"

BOOL IsWindows7Or8Point1(void) {
	return IsWindows7() || IsWindows8Point1();
}

BOOL IsWindows7(void) {
	return IsWindows7OrGreater() && !IsWindows8OrGreater();
}

BOOL IsWindows8Point1(void) {
	return IsWindows8Point1OrGreater() && !IsWindows10OrGreater();
}

VOID DetourIAT(HMODULE hModule, LPSTR lpFuncName, LPVOID *lpOldAddress, LPVOID lpNewAddress) {
	LPVOID *lpAddress = FindIAT(hModule, lpFuncName);
	if (!lpAddress || *lpAddress == lpNewAddress) {
		return;
	}

	DWORD flOldProtect;
	DWORD flNewProtect = PAGE_READWRITE;
	VirtualProtect(lpAddress, sizeof(LPVOID), flNewProtect, &flOldProtect);
	if (lpOldAddress) {
		*lpOldAddress = *lpAddress;
	}
	_dbgprintf("%s %p => %p", lpFuncName, *lpAddress, lpNewAddress);
	*lpAddress = lpNewAddress;
	VirtualProtect(lpAddress, sizeof(LPVOID), flOldProtect, &flNewProtect);
}

LPVOID *FindIAT(HMODULE hModule, LPSTR lpFunctionName) {
	SIZE_T hm = (SIZE_T)hModule;

	for (PIMAGE_IMPORT_DESCRIPTOR iid = (PIMAGE_IMPORT_DESCRIPTOR)(hm +
		((PIMAGE_NT_HEADERS)(hm + ((PIMAGE_DOS_HEADER)hm)->e_lfanew))->OptionalHeader
			.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT]
			.VirtualAddress); iid->Name; iid++) {

		LPVOID *p;
		for (SIZE_T i = 0; *(p = i + (LPVOID *)(hm + iid->FirstThunk)); i++) {
			LPSTR fn = (LPSTR)(hm + *(i + (SIZE_T *)(hm + iid->OriginalFirstThunk)) + 2);
			if (!((uintptr_t)fn & IMAGE_ORDINAL_FLAG) && !_stricmp(lpFunctionName, fn)) {
				return p;
			}
		}
	}
	return NULL;
}

BOOL FindPattern(LPCBYTE pvData, SIZE_T nNumberOfBytes, LPSTR lpszPattern, SIZE_T nStart, SIZE_T *lpOffset) {
	SIZE_T length = strlen(lpszPattern);
	SIZE_T nBytes;
	if (length % 2 || (nBytes = length / 2) > nNumberOfBytes) {
		return FALSE;
	}

	LPBYTE lpBytes = malloc(nBytes * sizeof(BYTE));
	BOOL *lpbwc = malloc(nBytes * sizeof(BOOL));

	LPSTR p = lpszPattern;
	BOOL valid = TRUE;
	for (SIZE_T i = 0; i < nBytes; i++) {
		if ((lpbwc[i] = strncmp(p, "??", 2)) && sscanf_s(p, "%2hhx", &lpBytes[i]) != 1) {
				valid = FALSE;
				break;
		}
		p += 2;
	}
	BOOL result = FALSE;
	if (valid) {
		for (SIZE_T i = nStart; i < nNumberOfBytes - nStart - (nBytes - 1); i++) {
			BOOL found = TRUE;
			for (SIZE_T j = 0; j < nBytes; j++) {
				if (lpbwc[j] && pvData[i + j] != lpBytes[j]) {
					found = FALSE;
					break;
				}
			}
			if (found) {
				*lpOffset = i;
				result = TRUE;
				break;
			}
		}
	}
	free(lpBytes);
	free(lpbwc);
	return result;
}

BOOL InjectLibrary(HANDLE hProcess, LPCTSTR lpLibFileName, DWORD cb) {
	LPVOID lpBaseAddress = VirtualAllocEx(hProcess, NULL, cb, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (!WriteProcessMemory(hProcess, lpBaseAddress, lpLibFileName, cb, NULL)) {
		return FALSE;
	}
	DWORD dwProcessId = GetProcessId(hProcess);
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);
	MODULEENTRY32 me;
	me.dwSize = sizeof(me);

	Module32First(hSnap, &me);
	do {
		if (!_tcsicmp(me.szModule, _T("kernel32.dll"))) {
			break;
		}
	} while (Module32Next(hSnap, &me));
	CloseHandle(hSnap);
	_tdbgprintf(_T("Injecting %s into process %d"), lpLibFileName, dwProcessId);
	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)GetProcAddress(me.hModule, _CRT_STRINGIZE(LoadLibrary)), lpBaseAddress, 0, NULL);
	CloseHandle(hThread);
	return TRUE;
}

VOID SuspendProcessThreads(DWORD dwProcessId, DWORD dwThreadId, HANDLE *lphThreads, SIZE_T dwSize, SIZE_T *lpcb) {
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	THREADENTRY32 te;
	te.dwSize = sizeof(te);
	Thread32First(hSnap, &te);

	SIZE_T count = 0;

	do {
		if (te.th32OwnerProcessID != dwProcessId || te.th32ThreadID == dwThreadId) {
			continue;
		}
		lphThreads[count] = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
		SuspendThread(lphThreads[count]);
		count++;
	} while (count < dwSize && Thread32Next(hSnap, &te));
	CloseHandle(hSnap);

	*lpcb = count;
	_tdbgprintf(_T("Suspended other threads."));
}

VOID ResumeAndCloseThreads(HANDLE *lphThreads, SIZE_T cb) {
	for (SIZE_T i = 0; i < cb; i++) {
		ResumeThread(lphThreads[i]);
		CloseHandle(lphThreads[i]);
	}
	_tdbgprintf(_T("Resumed threads."));
}

VOID _wdbgprintf(LPCWSTR format, ...) {
	WCHAR buffer[0x1000];
	va_list argptr;
	va_start(argptr, format);
	vswprintf_s(buffer, _countof(buffer), format, argptr);
	va_end(argptr);
	OutputDebugStringW(buffer);
}

VOID _dbgprintf(LPCSTR format, ...) {
	CHAR buffer[0x1000];
	va_list argptr;
	va_start(argptr, format);
	vsprintf_s(buffer, _countof(buffer), format, argptr);
	va_end(argptr);
	OutputDebugStringA(buffer);
}
