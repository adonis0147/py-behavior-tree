#include "global.h"
#include <ctime>

void print_timestamp() {
	time_t timestamp = time(NULL);
	struct tm *time_info = localtime(&timestamp);
	char buffer[80];
	strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", time_info);
	PySys_WriteStdout(buffer);
}
