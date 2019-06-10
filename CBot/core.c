#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <curl/curl.h>
#include <nxjson.h>

#include "core.h"

typedef struct
{
	char* data;
	size_t size;
} Buffer, * pBuffer;

typedef struct
{
	const char* key;
	const char* server;
	const char* ts;
} LongpollServer, * pLongpollServer;

typedef struct
{
	int peer_id;
	const char* text;
} Event, pEvent;

/* Https request helper */
static size_t write(void* contents, size_t size, size_t nmemb, void* userp);
static Buffer send_request(const char* url);

/* Longpoll connection */
static int get_longpoll(pLongpollServer longpoll);
static int start_longpoll(const pLongpollServer longpoll);

/* Parsing functions */
typedef struct
{
	long long peer_id;
	const char* text;
} Message, * pMessage;

typedef struct
{
	const char* ts;
	int length;
	pMessage messages;
} MessageEvent, * pMessageEvent;

typedef struct
{
	const char* commmand;
	const char* args[16];
	int argscnt;
	long long peer_id;
} PluginArgs, * pPluginArgs;

static void parse_longpoll(Buffer response, pMessageEvent event);
static Message parse_message(const nx_json* msg);
static void parse_command(const pMessageEvent event);

static const char* strclone(const char* str);


int bot_start()
{
	if (CURLE_OK != curl_global_init(CURL_GLOBAL_ALL))
	{
		puts("[!] Error: cannot initialise cURL");
		return 1;
	}

	LongpollServer longpoll;
	if (get_longpoll(&longpoll))
		puts("[!] Error: cannot get longpoll server");
	start_longpoll(&longpoll);

	curl_global_cleanup();
}

static int start_longpoll(const pLongpollServer longpoll)
{
	char request[512];
	while (1)
	{
		sprintf(request, "%s?act=a_check&key=%s&ts=%s&wait=25", longpoll->server, longpoll->key, longpoll->ts);
		Buffer response = send_request(request);
		printf("Longpoll result: %s", response.data);

		MessageEvent event;
		parse_longpoll(response, &event);

		free(longpoll->ts);
		longpoll->ts = event.ts;

		for (int i = 0; i < event.length; ++i)
		{
			printf("-----\nMessage %d\npeer_id:%d\ntext:%s\n", i, (int)event.messages[i].peer_id, event.messages[i].text);
		}

		parse_command(&event);
	}
}

static void parse_command(const pMessageEvent event)
{
	for (int i = 0; i < event->length; ++i)
	{
		char* command = NULL;
		char* args[16];
		int argscnt;
		const char* text = event->messages[i].text;
		char* pos = strstr(text, "[club183208139 | @purecbot]");
		if (NULL == pos) // Backslash command
		{
			if (text[0] != '/') continue;
			++text;
		}
		else text += 27;

		char* token = strtok(text, " ");
		if (NULL == token) continue;

		command = token;
		for (argscnt = 0; argscnt < 16; ++argscnt)
		{
			token = strtok(NULL, " ");
			if (NULL == token) break;
			args[argscnt] = token;
		}
	}
}

static void parse_longpoll(Buffer response, pMessageEvent event)
{
	const nx_json* json = nx_json_parse_utf8(response.data);

	event->ts = strclone(nx_json_get(json, "ts")->text_value);

	const nx_json* updates = nx_json_get(json, "updates");
	event->messages = (pMessage)malloc(sizeof(Message) * updates->length);
	event->length = updates->length;

	for (int i = 0; i < updates->length; ++i)
	{
		const nx_json* msg = nx_json_get(nx_json_item(updates, i), "object");
		event->messages[i] = parse_message(msg);
	}

	nx_json_free(json);
	free(response.data);
}

static Message parse_message(const nx_json* msg)
{
	Message message;
	message.peer_id = nx_json_get(msg, "peer_id")->int_value;
	message.text = strclone(nx_json_get(msg, "text")->text_value);
	return message;
}

static int get_longpoll(pLongpollServer longpoll)
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

	nx_json_free(json);
end:
	free(response.data);
	return error;
}

static Buffer send_request(const char* url)
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
		fprintf(stderr, "curl_easy_perform() failed: %s\n", strerror);

		free(buf.data);
		buf.size = 0;
		goto cleanup;
	}

	printf("%lu bytes retrieved\n", (long)buf.size);

cleanup:
	curl_easy_cleanup(curl);
	return buf;
}

static size_t write(void* contents, size_t size, size_t nmemb, void* userp)
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

static const char* strclone(const char* str)
{
	size_t strsize = strlen(str) + 1;
	char* copy = (char*)malloc(strsize);
	if (NULL == copy)
		return NULL;
	memcpy(copy, str, strsize);
	return copy;
}