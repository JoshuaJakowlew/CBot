#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>

#include <curl/curl.h>

#include "echo.h"

int plugin_echo(const pPluginArgs args)
{
	printf("[i] Plugin: echo has been invoked\n");

	if (NULL == args->text)
		return 0;

	const char* message = curl_easy_escape(curl, args->text, 0);

	char request[2048];
	int length = sprintf(request, BUILD_REQUEST("messages.send", "peer_id=%d&message=%s&random_id=%d"), (int)args->peer_id, message, rand());
	Buffer response = send_request(request, length);

	curl_free(message);
	return 0;
}