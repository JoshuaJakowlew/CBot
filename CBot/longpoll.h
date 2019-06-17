#ifndef LONGPOLL_H
#define LONGPOLL_H

typedef struct _Message
{
	const char* text;
	long long peer_id;
} Message;

int longpoll_init();
void longpoll_free();
int longpoll_getmessages(Message** messages, size_t* count, char** buffer);

#endif
