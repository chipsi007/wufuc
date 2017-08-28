#ifndef PATCHWUA_H
#define PATCHWUA_H
#pragma once

#include <Windows.h>

BOOL PatchWUA(void *lpBaseOfDll, size_t SizeOfImage);
#endif
