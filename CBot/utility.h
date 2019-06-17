#ifndef UTILITY_H
#define UTILITY_H

#ifdef _WIN64
#include <windows.h>
#define sleep(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep(ms) usleep(ms * 1000)
#endif

#endif
