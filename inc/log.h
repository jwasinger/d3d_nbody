#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <string>
#include <algorithm>

#include "common_include.h"
#include "misc.h"



/*
*
* create the log file, or if NULL is passed only output to stderr
*
*/
bool init_log(char *log_file);

/*
*
* Write a formatted string to the log file (if file logging is enabled
* and write the string to stderr
*
*/
void log_str(char *format, ...);

/*
* 
* Close the file opened by init_log (or do nothing if stderr output is used)
*
*/
void close_log(void);

#endif