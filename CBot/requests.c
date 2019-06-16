#include <curl/curl.h>

#include "defines.h"
#include "requests.h"

LOCAL CURL* curl;

LOCAL size_t write(void* contents, size_t size, size_t nmemb, void* userp);

int requests_init()
{
	int error = RE_OK;

	if (CURLE_OK != curl_global_init(CURL_GLOBAL_ALL))
		return RE_CURLE;

	curl = curl_easy_init();
	if (NULL == curl)
		return RE_CURLE;

	error |= curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write);
	/* some servers don't like requests that are made without a user-agent
	field, so we provide one */
	error |= curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	error |= curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	error |= curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
	if (error)
		return RE_CURLE;

	return RE_OK;
}

void requests_free()
{
	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

Response requests_send(const char* url)
{
	CURLcode error = CURLE_OK;

	Response buf;
	buf.data = NULL;
	buf.size = 0;

	error |= curl_easy_setopt(curl, CURLOPT_URL, url);
	error |= curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)& buf);
	if (CURLE_OK != error)
		goto end;

	error = curl_easy_perform(curl);
	if (CURLE_OK != error)
	{
		if(NULL != buf.data)
			free(buf.data);
		buf.size = 0;
		goto end;
	}

end:
	return buf;
}

char* escape_url(const char* text, int length)
{
	return curl_easy_escape(curl, text, length);
}

void free_escaped_url(const char* url)
{
	curl_free(url);
}

LOCAL size_t write(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t realsize = size * nmemb;
	Response* buf = (Response*)userp;

	char* newbuf = realloc(buf->data, buf->size + realsize + 1);
	if (newbuf == NULL)
		return 0;

	buf->data = newbuf;
	memcpy(&(buf->data[buf->size]), contents, realsize);
	buf->size += realsize;
	buf->data[buf->size] = '\0'; // Zero-terminate input string

	return realsize;
}