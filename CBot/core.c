#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <curl/curl.h>
#include <nxjson.h>

#include "core.h"
#include "plugins.h"

#define LOCAL static

/* Longpoll connection */
typedef struct
{
	const char* key;
	const char* server;
	const char* ts;
} LongpollServer, * pLongpollServer;

LOCAL int start_longpoll(const pLongpollServer longpoll);
LOCAL int get_longpoll(pLongpollServer longpoll);

/* Parsing functions */
typedef struct
{
	const char* text;
	long long peer_id;
} Message, * pMessage;

typedef struct
{
	pMessage messages;
	const char* ts;
	int count;
} MessageEvent, * pMessageEvent;

LOCAL int parse_command(const pMessage msg, pPluginArgs args);
LOCAL Message parse_message(const nx_json* msg);
LOCAL int parse_longpoll(pBuffer response, pMessageEvent event);

/* Plugin loaders */
typedef int(*PluginHandler)(const pPluginArgs);

LOCAL PluginHandler select_plugin(const pPluginArgs args);

/* Https request helpers */
LOCAL size_t write(void* contents, size_t size, size_t nmemb, void* userp);

/* Utility functions */
LOCAL const char* strclone(const char* str);

/* Bot core starter */
int bot_start()
{
	int error = 0;
	if (CURLE_OK != curl_global_init(CURL_GLOBAL_ALL))
	{
		LOG("[!] Error: cannot initialise cURL\n");
		error = 1;
		goto end;
	}

	LongpollServer longpoll;
	if (get_longpoll(&longpoll))
	{
		LOG("[!] Error: cannot get longpoll server\n");
		error = 2;
		goto end;
	}

	if (start_longpoll(&longpoll))
	{
		LOG("[!] Error: lonpoll loop failed\n");
		error = 3;
	}
		
end:
	free(longpoll.key);
	free(longpoll.server);
	free(longpoll.ts);
	curl_global_cleanup();
	return error;
}

/* Longpoll functions */
LOCAL int start_longpoll(const pLongpollServer longpoll)
{
	int error = 0;
	char request[512];
	while (1)
	{
		// Send request
		sprintf(request, "%s?act=a_check&key=%s&ts=%s&wait=25", longpoll->server, longpoll->key, longpoll->ts);
		Buffer response = send_request(request);
		if (0 == response.size)
		{
			LOG("[!] Error: send_request(%s) failed\n", request);
			error = 1;
			goto end;
		}

		LOG("[i] Longpoll result: %s\n", response.data);

		// Parse JSON
		MessageEvent event;
		if (parse_longpoll(&response, &event))
		{
			LOG("[!] Error: parse_lonpoll failed\n");
			error = 2;
			goto end;
		}

		for (int i = 0; i < event.count; ++i)
			LOG("[i] Message %d\npeer_id:%d\ntext:%s\n",
				i, (int)event.messages[i].peer_id, event.messages[i].text);

		// Parse command
		for (int i = 0; i < event.count; ++i)
		{
			PluginArgs args;
			if (parse_command(&event.messages[i], &args))
			{
				LOG("[!] Error: parse_command failed\n");
				error = 3;
				goto end;
			}

			LOG("[i] Command %d: %s\n", i, args.commmand);
			for (int i = 0; i < args.argscnt; ++i)
				LOG("[i] args[%d]: (%s)\n", i, args.args[i]);

			PluginHandler plugin = select_plugin(&args);
			if(plugin) plugin(&args);
		}

		free(longpoll->ts);
		longpoll->ts = strclone(event.ts);

		free(event.messages);
		free(response.data);
	}

end:
	return error;
}

LOCAL int get_longpoll(pLongpollServer longpoll)
{
	int error = 0;
	Buffer response = send_request(BUILD_REQUEST("groups.getLongPollServer", "group_id=" GROUP_ID));
	if (0 == response.size)
	{
		error = 1;
		goto end;
	}

	const nx_json* json = nx_json_parse_utf8(response.data);
	const nx_json* root = nx_json_get(json, "response");

	const char* key = nx_json_get(root, "key")->text_value;
	const char* server = nx_json_get(root, "server")->text_value;
	const char* ts = nx_json_get(root, "ts")->text_value;

	printf("key: %s\nserver: %s\nts: %s\n", key, server, ts);

	longpoll->key = strclone(key);
	longpoll->server = strclone(server);
	longpoll->ts = strclone(ts);

	if (NULL == longpoll->key ||
		NULL == longpoll->server ||
		NULL == longpoll->ts)
	{
		error = 2;
		goto end;
	}

end:
	//nx_json_free(json);
	free(response.data);
	return error;
}
/* End of longpoll functions */

/* Parsing functions */
LOCAL int parse_command(const pMessage msg, pPluginArgs args)
{
	// Still trash and spagetti
	args->commmand = NULL;
	args->argscnt = 0;
	args->peer_id = msg->peer_id;
	const char* text = msg->text;
	char* pos = strstr(text, "[club" GROUP_ID " | @purecbot]");
	if (NULL != pos)
		text += sizeof("[club" GROUP_ID " | @purecbot]") - 1;

	if (text[0] != '/')
		return 0;
	++text;
	
	args->text = "ECHO PLUGIN RESPONSE";

	char* token = strtok(text, " ");
	if (NULL == token)
		return 0;

	args->commmand = token;
	for (args->argscnt = 0; args->argscnt < 16; ++args->argscnt)
	{
		token = strtok(NULL, "  ");
		if (NULL == token)
			break;
		args->args[args->argscnt] = token;
	}
	return 0;
}

LOCAL Message parse_message(const nx_json* msg)
{
	Message message;
	message.peer_id = nx_json_get(msg, "peer_id")->int_value;
	message.text = nx_json_get(msg, "text")->text_value;
	return message;
}

LOCAL int parse_longpoll(pBuffer response, pMessageEvent event)
{
	int error = 0;

	const nx_json* json = nx_json_parse_utf8(response->data);

	event->ts = nx_json_get(json, "ts")->text_value;

	const nx_json* updates = nx_json_get(json, "updates");
	event->messages = (pMessage)malloc(sizeof(Message) * updates->length);
	if (NULL == event->messages)
	{
		error = 1;
		goto end;
	}
	event->count = updates->length;

	for (int i = 0; i < event->count; ++i)
	{
		const nx_json* msg = nx_json_get(nx_json_item(updates, i), "object");
		event->messages[i] = parse_message(msg);
	}

end:
	nx_json_free(json);
	return error;
}
/* End of parsing functions */

/* Plugin loaders */
LOCAL PluginHandler select_plugin(const pPluginArgs args )
{
	if (!strcmp("echo", args->commmand))
		return &plugin_echo;
	return NULL;
}
/* End of plugin loaders */

/* Https request helpers */
Buffer send_request(const char* url)
{
	CURL* curl;
	CURLcode res;

	Buffer buf;
	buf.data = NULL;
	buf.size = 0;

	curl = curl_easy_init();

	curl_easy_setopt(curl, CURLOPT_URL, url);

	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)& buf);

	/* some servers don't like requests that are made without a user-agent
	field, so we provide one */
	curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

	res = curl_easy_perform(curl);

	if (res != CURLE_OK)
	{
		const char* strerror = curl_easy_strerror(res);
		LOG("[!] Error: curl_easy_perform() failed: %s\n", strerror);

		free(buf.data);
		buf.size = 0;
		goto end;
	}

end:
	curl_easy_cleanup(curl);
	return buf;
}

LOCAL size_t write(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t realsize = size * nmemb;
	pBuffer buf = (pBuffer)userp;

	char* newbuf = realloc(buf->data, buf->size + realsize + 1);
	if (newbuf == NULL)
	{
		printf("not enough memory (realloc returned NULL)\n");
		return 0;
	}
	buf->data = newbuf;
	memcpy(&(buf->data[buf->size]), contents, realsize);
	buf->size += realsize;
	buf->data[buf->size] = '\0'; // Zero-terminate input string

	return realsize;
}
/* End of https request helpers */

/* Utility functions */
LOCAL const char* strclone(const char* str)
{
	size_t strsize = strlen(str) + 1;
	char* copy = (char*)malloc(strsize);
	if (NULL == copy)
		return NULL;
	memcpy(copy, str, strsize);
	return copy;
}
/* End of utility functions */