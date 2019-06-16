#ifndef REQUESTS_H
#define REQUESTS_H

#include <stdint.h>

#define VKAPI_VERSION "5.95"

#define BUILD_REQUEST(method, params) "https://api.vk.com/method/" \
									  method "?" params "&access_token="\
									  ACCESS_TOKEN "&v=" VKAPI_VERSION

enum ResponseError
{
	RE_OK = 0,
	RE_CURLE = 1,
	RE_NOMEM = 2
};

typedef struct _Response
{
	char* data;
	size_t size;
} Response;

int requests_init();
void requests_free();
Response requests_send(const char* url);

char* escape_url(const char* text, int length);
void free_escaped_url(const char* url);

#endif
