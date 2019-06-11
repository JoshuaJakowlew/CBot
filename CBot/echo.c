#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

#include "echo.h"

int plugin_echo(const pPluginArgs args)
{
	printf("[i] Plugin: echo has been invoked\n");
	char params[2048];
	sprintf(params, BUILD_REQUEST("messages.send", "peer_id=-%d&message=%s&random_id=%d"), (int)args->peer_id, args->text, rand());
	Buffer response = send_request(params);
	return 0;
}