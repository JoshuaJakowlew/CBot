#ifndef CORE_H
#define CORE_H

#define VKAPI_VERSION "5.95"
#define ACCESS_TOKEN "cb4d574e37710440dd1fb2f39478bb4bb5a7ae63049066d00e5823c46d2fbbad04b5bccd02435dc6191c4"
#define GROUP_ID "183208139"

#define BUILD_REQUEST(method, params) "https://api.vk.com/method/" \
									  method "?" params "&access_token="\
									  ACCESS_TOKEN "&v=" VKAPI_VERSION

int bot_start();

#endif
