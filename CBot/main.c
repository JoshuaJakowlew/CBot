#include <stdio.h>

#include "defines.h"
#include "core.h"

int main()
{
	if( bot_start())
		LOG(LOG_ERROR, "Unknown error");
}