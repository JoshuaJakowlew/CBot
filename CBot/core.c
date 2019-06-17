#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#include "core.h"
#include "utility.h"
#include "requests.h"
#include "longpoll.h"
#include "plugin_selector.h"

LOCAL int start_main_loop();

int bot_start()
{
	int error = CBOTE_OK;

	srand(time(NULL));
	if (CBOTE_OK != requests_init() ||
		CBOTE_OK != longpoll_init())
	{
		LOG(LOG_ERROR, "Bot initialisation failed");
		error = CBOTE_CORE_INIT_FAILED;
		goto end;
	}

	LOG(LOG_INFO, "Bot has been successfully started!");
	start_main_loop();

end:
	LOG(LOG_INFO, "Shutting down bot");
	longpoll_free();
	requests_free();
	return error;
}

LOCAL int start_main_loop()
{
	int error = CBOTE_OK;

	while (true)
	{
		Message* msg = NULL;
		size_t count = 0;
		char* buffer = NULL;
		
		error = longpoll_getmessages(&msg, &count, &buffer);
		if (CBOTE_OK != error)
			if (CBOTE_NET == error)
				continue;
			else goto enditer;

		for (int i = 0; i < count; ++i)
			plugin_execute(msg + i);

	enditer:
		free(buffer);
	}
}
