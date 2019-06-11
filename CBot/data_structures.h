#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

typedef struct
{
	long long peer_id;
	const char* text;
	const char* commmand;
	const char* args[16];
	int argscnt;
} PluginArgs, * pPluginArgs;

#endif
