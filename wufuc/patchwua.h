#ifndef PATCHWUA_H_INCLUDED
#define PATCHWUA_H_INCLUDED
#pragma once

#include <Windows.h>

BOOL PatchWUA(void *lpBaseOfDll, size_t SizeOfImage);
#endif
