#include <sensorConfigParser.h>
#include <stdio.h>
#include <utilityFunctions.h>
#include <jsmn.h>

#define __bufferSize 4096
#define __maxTokens 1024

GList *readConfigurationFile(const char* filename) {
	char configutation[__bufferSize];
	FILE *cfgf = fopen(filename, "r");
	int bytesRead = fread(configutation, 1, __bufferSize, cfgf);
	if (bytesRead < 0) {
		perror("Unable to read the configuration file.");
		return NULL;
	}
	if (bytesRead == 0) {
		logErrorMessage("Configuration file %s is completely empty.", filename);
		return NULL;
	}
	if (bytesRead == __bufferSize) {
		logErrorMessage("Configuration file %s appears to be bigger than expected, recompile config parser with bigger buffer.", filename);
		return NULL;
	}

	jsmn_parser parser;
	jsmn_init(&parser);

	jsmntok_t tokens[__maxTokens];

	int tokensParsed = jsmn_parse(&parser, configutation, strlen(configutation), &tokens, __maxTokens);
	if (tokensParsed < 0) {
		int len =  1024
		char *err_msg = (char *) malloc(sizeof(char) * len);
		snprintf(err_msg, len, "Persing has finished with error value of %d\n", tokensParsed);
		syslog(LOG_ERR, err_msg);
		free(err_msg);
		return NULL;
	}

	tokens[0].type == JSMN_STRING
	strncmp(configutation + tokens[0].start, "Configuration", tokens[i].end - tokens[i].start) == 0
	tokens[1].type == JSMN_ARRAY
	

	for (int i = 0; i < tokensParsed; i++) {
		char str [__bufferSize] ;
		memset(str, 0, __bufferSize);
		strncpy(str, configutation + tokens[i].start, tokens[i].end - tokens[i].start);
		printf("Token ID: %2d ; type : %2d ; text : %s \n", i, tokens[i].type, str);
	}

}