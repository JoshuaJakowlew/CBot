#include <curl/curl.h>
#include <nxjson.h>

#include "longpoll.h"
#include "requests.h"
#include "private.h"
#include "defines.h"

typedef struct _LongpollServer
{
	const char* buffer; // text buffer that should be freed
	const char* key;
	const char* server;
	const char* ts;
} LongpollServer;

LOCAL LongpollServer longpoll;
LOCAL int longpoll_parse_server(char* text, LongpollServer* server);
LOCAL int longpoll_parse_ts(char* text, char* ts);
int longpoll_parse_message_event(char* text, Message** messages, size_t* count);

int longpoll_init()
{
	int error = LPE_OK;
	
	Response response = requests_send(
		BUILD_REQUEST("groups.getLongPollServer", "group_id=" GROUP_ID)
	);
	if (0 == response.size)
		return LPE_NET;

	longpoll.buffer = response.data;
	error = longpoll_parse_server(response.data, &longpoll);
	if (LPE_OK != error)
		return error;

	return error;
}

void longpoll_free()
{
	free((char*)longpoll.buffer);
}

// FIXME
int longpoll_getmessages(Message** messages, size_t* count, char** buffer)
{
	int error = LPE_OK;

	char request[512];
	sprintf(request, "%s?act=a_check&key=%s&ts=%s&wait=25", longpoll.server, longpoll.key, longpoll.ts);
	Response response = requests_send(request);
	if (0 == response.size)
		return LPE_NET;

	*buffer = response.data;

	// FIXME: DON'T LET IT LEAK! 
	char* ts = (char*)malloc(sizeof(char) * 16);
	if (NULL == ts)
		return LPE_MEM;
	error = longpoll_parse_ts(response.data, ts);
	if (LPE_OK != error)
	{
		free(ts);
		return error;
	}
	longpoll.ts = ts;

	error = longpoll_parse_message_event(response.data, messages, count);
	if (LPE_OK != error)
		return error;

	return error;
}

LOCAL int longpoll_parse_server(char* text, LongpollServer* server)
{
	int error = LPE_OK;

	const nx_json* json = nx_json_parse_utf8(text);
	const nx_json* root = nx_json_get(json, "response");

	server->key = nx_json_get(root, "key")->text_value;
	server->server = nx_json_get(root, "server")->text_value;
	server->ts = nx_json_get(root, "ts")->text_value;

	if (NULL == server->key || NULL == server->server || NULL == server->ts)
	{
		error = LPE_JSON;
		goto end;
	}

end:
	nx_json_free(json);
	return error;
}

LOCAL int longpoll_parse_ts(char* text, char* ts)
{
	int error = LPE_OK;

	char* tspos = strstr(text, "\"ts\":\"");
	if (NULL == tspos)
		return LPE_JSON;

	tspos += sizeof("\"ts\":\"") - 1;
	int i;
	for (i = 0; i < 16; ++i, ++tspos)
		if (*tspos != '"')
			ts[i] = *tspos;
		else break;
	ts[i] = '\0';
	
	return error;
}

int longpoll_parse_message_event(char* text, Message** messages, size_t* count)
{
	int error = LPE_OK;

	const nx_json* json = nx_json_parse_utf8(text);
	const nx_json* updates = nx_json_get(json, "updates");
	if (0 > updates->length)
	{
		error = LPE_JSON;
		goto end;
	}

	Message* temp_messages = (Message*)malloc(sizeof(Message) * updates->length);
	if (NULL == temp_messages)
	{
		error = LPE_MEM;
		goto end;
	}

	*messages = temp_messages;
	*count = 0;

	for (int i = 0; i < updates->length; ++i, ++(*count))
	{
		const nx_json* message_json = nx_json_get(nx_json_item(updates, i), "object");

		long long peer_id = nx_json_get(message_json, "peer_id")->int_value;
		const char* text = nx_json_get(message_json, "text")->text_value;
		if (0 == peer_id || NULL == text)
		{
			error = LPE_JSON;
			goto end;
		}

		(*messages)[i].peer_id = peer_id;
		(*messages)[i].text = text;
	}

end:
	nx_json_free(json);
	return error;
}
