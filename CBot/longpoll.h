#ifndef LONGPOLL_H
#define LONGPOLL_H

enum LongpollError
{
	LPE_OK = 0,
	LPE_NET = 1,
	LPE_JSON = 2,
	LPE_MEM = 3
};

typedef struct _Message
{
	const char* text;
	long long peer_id;
} Message;

int longpoll_init();
void longpoll_free();
int longpoll_getmessages(Message** messages, size_t* count, char** buffer);

#endif
