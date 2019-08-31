﻿#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spam.h"
#include "defines.h"
#include "private.h"
#include "plugins.h"
#include "requests.h"
#include "utility.h"

#define FLOODTEXT "Hoomans must die!"

#define TARGET_MSG_PACK_COUNT 4

int plugin_spam(const PluginArgs* args)
{
	int error = CBOTE_OK;

	// Size in packs (25msg each)
	// Successfully sent message pack
	int sent_count = 0;
	// Target count of message packs (TARGET_MSG_PACK_COUNT if not specified)
	int target_count = TARGET_MSG_PACK_COUNT;
	int attempts = 0;     // Amount of VKApi call attempts
	if(args->count)
		target_count = atoi(args->args[0]);
	
	while (sent_count < target_count)
	{
		char code_buf[256];
		sprintf(
			code_buf,
			"var i=0;var r='';while(i<25){r=API.messages.send({'peer_id':%d,'message':'%s','random_id':%d+i,'access_token':'%s','v':'5.95'});i=i+1;}return r;",
			(int)args->peer_id,
			FLOODTEXT,
			rand(),
			ACCESS_TOKEN
		);
		const char* code = escape_url(code_buf, 0);
		if (NULL == code)
			return CBOTE_NOMEM;

		char request[1024];
		sprintf(request, BUILD_REQUEST("execute", "code=%s"), code);
		Response response = requests_send(request);
		if (0 == response.size)
			return CBOTE_NET;

		printf("Attempt %d\n", ++attempts);
		if (NULL == strstr(response.data, "error"))
		{
			++sent_count;
			printf("> %d/%d messages sent\n", sent_count, target_count);
		}

		// Sleep 150s when 250 attempts runs
		if (attempts % 250 == 0)
			sleep(150'000);

		free_escaped_url(code);
		free(response.data);

	}

	return error;
}