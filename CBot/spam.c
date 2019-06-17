﻿#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "spam.h"
#include "defines.h"
#include "private.h"
#include "plugins.h"
#include "requests.h"
#include "utility.h"

const char* tokens[] = {
		TOKEN1,
		TOKEN2,
		TOKEN3,
		TOKEN4,
		TOKEN5
};

const char* floodtext = 
"\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A"
"\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A"
"\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A"
"\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A"
"\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A"
"\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A"
"\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A"
"\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A"
"\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A"
"\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A"
"\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A"
"\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A"
"\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A\xF0\x9F\x8C\x9A";

int plugin_spam(const PluginArgs* args)
{
	int error = CBOTE_OK;

	// Size in packs (25msg each)
	int sent_count = 0;
	int target_count = 4;
	int attempts = 0;
	if(args->count)
		target_count = atoi(args->args[0]);
	
	while (sent_count < target_count)
	{
		int token = attempts % (sizeof(tokens) / sizeof(tokens[0]));
		char code_buf[256];
		sprintf(
			code_buf,
			"var i=0;var r='';while(i<25){r=API.messages.send({'peer_id':%d,'message':'%s','random_id':%d+i,'access_token':'%s','v':'5.95'});i=i+1;}return r;",
			(int)args->peer_id,
			floodtext,
			rand(),
			tokens[token]
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