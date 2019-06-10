#ifndef CORE_H
#define CORE_H

#include <stdint.h>

#define VKAPI_VERSION "5.95"

#define BUILD_REQUEST(method, params) "https://api.vk.com/method/" \
									  method "?" params "&access_token="\
									  ACCESS_TOKEN "&v=" VKAPI_VERSION

#define LOG(text, ...) printf(text, __VA_ARGS__)

/* Https request helpers */
typedef struct
{
	char* data;
	size_t size;
} Buffer, * pBuffer;

Buffer send_request(const char* url);

/* Bot core starter */
int bot_start();

#endif
