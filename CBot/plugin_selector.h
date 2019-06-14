#ifndef PLUGIN_SELECTOR_H
#define PLUGIN_SELECTOR_H

#include "longpoll.h"

enum PluginSelectorError
{
	PSE_OK = 0,
	PSE_INPUT = 1,
	PSE_MEM = 2,
	PSE_NO_PLUGIN = 3,
	PSE_PLUGIN_FAILED = 4
};

typedef struct _PluginArgs
{
	long long peer_id;
	const char* text;
	const char* command;
	const char* args[16];
	int count;
} PluginArgs;

typedef int(*PluginHandler)(const PluginArgs*);

int plugin_execute(const Message* message);

#endif
