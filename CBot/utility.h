#ifndef UTILITY_H
#define UTILITY_H

#include "defines.h"

#ifdef _WIN64
#include <windows.h>
#define sleep(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep(ms) usleep(ms * 1000)
#endif

enum BotError
{
	CBOTE_OK = 0,
	CBOTE_NOMEM = 1,
	CBOTE_NET = 2,
	CBOTE_INPUT = 3,
	CBOTE_WRONG_PLUGIN = 4,
	CBOTE_PLUGIN_FAILED = 5,
	CBOTE_CURL_INIT_FAILED = 256,
	CBOTE_CORE_INIT_FAILED = 257
};

#endif
