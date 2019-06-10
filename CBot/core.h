#ifndef CORE_H
#define CORE_H

#define VKAPI_VERSION "5.95"

#define BUILD_REQUEST(method, params) "https://api.vk.com/method/" \
									  method "?" params "&access_token="\
									  ACCESS_TOKEN "&v=" VKAPI_VERSION

#define LOG(text, ...) printf(text, __VA_ARGS__)

int bot_start();

#endif
