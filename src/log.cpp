#include "log.h"

static FILE *f_ptr = NULL;
static bool log_init = false;

bool init_log(char *log_file)
{
	char err_buf[256];
	int error;
	if (log_file != NULL)
	{
		//try and open a file for writing
		error = fopen_s(&f_ptr, log_file, "w");
		if (error == ENOENT)
		{
			fopen_s(&f_ptr, log_file, "w+");
			if (error != 0)
			{
				strerror_s(err_buf, 256, error);
				printf("fopen error creating file: %s\n", err_buf);
				return false;
			}
		}
		if (error != 0 && error != ENOENT)
		{
			strerror_s(err_buf, 256, error);
			printf("fopen_s error: %s\n", err_buf);
			return false;
		}
	}
	
	log_init = true;
	return true;
}

//print a 
void log_str(char *format, ...)
{
	if (!log_init)
		return;

	va_list args;
	std::vector<std::string> strs = split_no_remove(std::string(format), '%');
	int n = strs.size();
	char *val = NULL;
	std::string output(strs[0]);
	char buf[256];

	va_start(args, format);
	for (int i = 1; i < n; i++)
	{
		val = va_arg(args, char *);
		sprintf_s(buf, strlen(strs[i].data()), strs[i].data(), val);
		buf[strlen(strs[i].data())] = '\0';
		output.append(buf);
	}
	va_end(args);

	printf("%s\n", output);
	return;
}

void close_log(void)
{
	if (f_ptr)
		fclose(f_ptr);
}
