#include "global.h"
#include <ctime>

void get_timestamp(char *buffer, size_t size) {
	time_t timestamp = time(NULL);
	struct tm *time_info = localtime(&timestamp);
	strftime(buffer, sizeof(char *) * size, "%Y-%m-%d %H:%M:%S", time_info);
}
