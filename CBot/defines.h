#ifndef DEFINES_H
#define DEFINES_H

#define LOCAL static

#define LOG_ERROR "[!] Error: "
#define LOG_INFO  "[i] Info:  "
#define LOG(modifier, text) puts(modifier text)
#define FLOG(modifier, text, ...) printf(modifier text "\n", __VA_ARGS__)

#endif
