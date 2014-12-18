#include "log.h"
#include "common_include.h"
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

static FILE *f_ptr = NULL;

bool init_log(char *log_file)
{
	int error;
	if (log_file == NULL)
		return;

	//try and open a file for writing
	f_ptr = fopen(log_file, "w");
	if (f_ptr == NULL && errno == ENOENT)
	{
		f_ptr = fopen(log_file, "w+");
		if (f_ptr == NULL)
		{
			error = errno;
			printf("fopen error creating file: %s\n", strerror(error));
			return NULL;
		}
	}
	else
	{
		error = errno;
		printf("fopen error opening file: %s\n", strerror(error));
		return NULL;
	}
	
	return true;
}

void log_str(char *format, ...)
{
}

void close_log(void)
{
	if (f_ptr)
		fclose(f_ptr);
}
