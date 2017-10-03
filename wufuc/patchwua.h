#pragma once

#include <stdbool.h>

#include <phnt_windows.h>

bool patch_wua(void *lpBaseOfDll, size_t SizeOfImage);
