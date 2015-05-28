#include "utilityFunctions.h"

long long getCurrentUSecs() {
	struct timeval  tv;
	gettimeofday(&tv, NULL);

	long long currentUSecs = (long long)tv.tv_sec * (long long)1000 * (long long)1000 + (long long)tv.tv_usec;
	return currentUSecs;
}


// void sqlite_log_error(const char *msg, const char *db_msg) {
// 	int len = strnlen(msg, 1024) + strnlen(db_msg, 1024) +  10;
// 	char *sys_msg = (char *) malloc(len * sizeof(char));
// 	snprintf(sys_msg, len, "%s %s", msg, db_msg);
// 	syslog(LOG_ERR, sys_msg);
// 	free(sys_msg);
// }


void logErrorMessage(const char *format, const char *detail) {
	int len =  (strnlen(format, 1024) + strnlen(detail, 1024)) + 10;
	char *err_msg = (char *) malloc(sizeof(char) * len);
	snprintf(err_msg, len, format, detail);
	syslog(LOG_ERR, err_msg);
	free(err_msg);	
}

