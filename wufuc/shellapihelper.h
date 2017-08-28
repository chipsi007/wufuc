#ifndef SHELLAPIHELPER_H
#define SHELLAPIHELPER_H
#pragma once

#ifdef UNICODE
#define CommandLineToArgv  CommandLineToArgvW
#else
#define CommandLineToArgv  CommandLineToArgvA
#endif
#endif
