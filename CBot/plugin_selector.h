#ifndef PLUGIN_SELECTOR_H
#define PLUGIN_SELECTOR_H

#include "longpoll.h"

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
