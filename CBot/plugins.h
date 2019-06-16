#ifndef PLUGINS_H
#define PLUGINS_H

#include "echo.h"
#include "spam.h"

enum PluginError
{
	PE_OK = 0,
	PE_INPUT = 1,
	PE_MEM = 2,
	PE_NET = 3
};


#endif
