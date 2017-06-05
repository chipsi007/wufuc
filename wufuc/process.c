#include <windows.h>
#include <stdio.h>
#include <TlHelp32.h>
#include <tchar.h>
#include "util.h"
#include "process.h"

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
	_tdbgprintf(_T("%S %p => %p"), lpFuncName, *lpAddress, lpNewAddress);
	*lpAddress = lpNewAddress;
	VirtualProtect(lpAddress, sizeof(LPVOID), flOldProtect, &flNewProtect);
}

LPVOID *FindIAT(HMODULE hModule, LPSTR lpFuncName) {
	PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)hModule;
	PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((LPBYTE)dos + dos->e_lfanew);
	PIMAGE_IMPORT_DESCRIPTOR desc = (PIMAGE_IMPORT_DESCRIPTOR)((LPBYTE)dos + nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

	for (PIMAGE_IMPORT_DESCRIPTOR iid = desc; iid->Name != 0; iid++) {
		for (int i = 0; *(i + (LPVOID*)(iid->FirstThunk + (SIZE_T)hModule)) != NULL; i++) {
			LPSTR name = (LPSTR)(*(i + (SIZE_T*)(iid->OriginalFirstThunk + (SIZE_T)hModule)) + (SIZE_T)hModule + 2);
			const uintptr_t n = (uintptr_t)name;
			if (!(n & (sizeof(n) == 4 ? 0x80000000 : 0x8000000000000000)) && !_stricmp(lpFuncName, name)) {
				return i + (LPVOID*)(iid->FirstThunk + (SIZE_T)hModule);
			}
		}
	}
	return NULL;
}

BOOL FindPattern(LPCBYTE lpBytes, SIZE_T nNumberOfBytes, LPSTR lpszPattern, SIZE_T nStart, SIZE_T *lpOffset) {
	SIZE_T nPatternLength = strlen(lpszPattern);
	SIZE_T nMaskLength = nPatternLength / 2;
	if (nMaskLength > nNumberOfBytes || nPatternLength % 2) {
		return FALSE;
	}

	LPBYTE lpPattern = malloc(nMaskLength * sizeof(BYTE));
	BOOL *lpbMask = malloc(nMaskLength * sizeof(BOOL));

	LPSTR p = lpszPattern;
	BOOL valid = TRUE;
	for (SIZE_T i = 0; i < nMaskLength; i++) {
		if (lpbMask[i] = strncmp(p, "??", 2)) {
			if (sscanf_s(p, "%2hhx", &lpPattern[i]) != 1) {
				valid = FALSE;
				break;
			}
		}
		p += 2;
	}
	BOOL result = FALSE;
	if (valid) {
		for (SIZE_T i = nStart; i < nNumberOfBytes - nStart - (nMaskLength - 1); i++) {
			BOOL found = TRUE;
			for (SIZE_T j = 0; j < nMaskLength; j++) {
				if (lpbMask[j] && lpBytes[i + j] != lpPattern[j]) {
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
	free(lpPattern);
	free(lpbMask);
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

VOID SuspendProcess(HANDLE *lphThreads, SIZE_T dwSize, SIZE_T *lpcb) {
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
	THREADENTRY32 te;
	te.dwSize = sizeof(te);
	Thread32First(hSnap, &te);

	DWORD dwProcessId = GetCurrentProcessId();
	DWORD dwThreadId = GetCurrentThreadId();

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
