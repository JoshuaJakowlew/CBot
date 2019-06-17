#include <stdio.h>
#include <stdlib.h>

#include "echo.h"
#include "utility.h" 
#include "defines.h"
#include "private.h"
#include "plugins.h"
#include "requests.h"

int plugin_echo(const PluginArgs* args)
{
	int error = CBOTE_OK;

	const char* message = escape_url(args->text, 0);
	if (NULL == message)
		return CBOTE_NOMEM;

	char request[2048];
	sprintf(request, BUILD_REQUEST("messages.send", "peer_id=%d&message=%s&random_id=%d"), (int)args->peer_id, message, rand());
	Response response = requests_send(request);
	if (0 == response.size)
		return CBOTE_NET;

	free(response.data);
	free_escaped_url(message);
	return error;
}