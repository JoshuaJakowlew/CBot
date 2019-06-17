#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "plugins.h"
#include "plugin_selector.h"
#include "defines.h"
#include "utility.h"

LOCAL void plugin_free_args(PluginArgs* args);
LOCAL int parse_command(const char* text, PluginArgs* args);
LOCAL PluginHandler select_plugin(const PluginArgs* args);

int plugin_execute(const Message* message)
{
	int error = CBOTE_OK;

	PluginArgs args;
	args.peer_id = message->peer_id;
	if (CBOTE_OK != parse_command(message->text, &args))
		return CBOTE_INPUT;

	FLOG(LOG_INFO, "Executing \"%s\" plugin...", args.command);
	PluginHandler plugin = select_plugin(&args);
	if (NULL == plugin)
	{
		LOG(LOG_ERROR, "Plugin not found!");
		error = CBOTE_WRONG_PLUGIN;
		goto end;
	}
	
	if (CBOTE_OK != plugin(&args))
	{
		LOG(LOG_ERROR, "Plugin failed!");
		error = CBOTE_PLUGIN_FAILED;
		goto end;
	}

	LOG(LOG_INFO, "Plugin has been successfully executed");

end:
	plugin_free_args(&args);
	return error;
}

LOCAL void plugin_free_args(PluginArgs* args)
{
	free((char*)args->command);
}

LOCAL int parse_command(const char* text, PluginArgs* args)
{
	int error = CBOTE_OK;

	char* pos = strchr(text, '/') + 1;
	if (NULL == pos - 1 || *pos == '\0')
		return CBOTE_INPUT;

	size_t size = strlen(pos) + 1;
	char* tokens = (char*)malloc(size);
	if (NULL == tokens)
		return CBOTE_NOMEM;
	memcpy(tokens, pos, size);

	args->command = strtok(tokens, " ");
	args->text = pos + strlen(args->command) + 1;
	args->count = 0;
	for (int i = 0; i < 16; ++i, ++args->count)
	{
		const char* token = strtok(NULL, " ");
		if (NULL == token)
			break;
		args->args[args->count] = token;
	}

	return error;
}

LOCAL PluginHandler select_plugin(const PluginArgs* args)
{
	if (!strcmp("echo", args->command))
		return plugin_echo;
	if (!strcmp("spam", args->command))
		return plugin_spam;

	return NULL;
}
