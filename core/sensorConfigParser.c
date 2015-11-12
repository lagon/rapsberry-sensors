#include <sensorConfigParser.h>
#include <stdio.h>
#include <utilityFunctions.h>
#include <jsmn.h>

#define __bufferSize 4096
#define __maxTokens 1024

char *extractStringFromJson(const char *jsonString, jsmntok_t token) {
	int size = token.end - token.start;
	char *str = (char *) malloc(sizeof(char) * (size + 1));
	memset(str, 0, size + 1);
	strncpy(str, jsonString + token.start, size);
	return str;
}

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

	int tokensParsed = jsmn_parse(&parser, configutation, strlen(configutation), tokens, __maxTokens);
	printf("%d configutation tokens parsed\n", tokensParsed);
	if (tokensParsed < 0) {
		int len =  1024;
		char *err_msg = (char *) malloc(sizeof(char) * len);
		snprintf(err_msg, len, "Persing has finished with error value of %d\n", tokensParsed);
		syslog(LOG_ERR, err_msg);
		free(err_msg);
		return NULL;
	}

	if ((tokens[0].type != JSMN_STRING) || (strncmp(configutation + tokens[0].start, "Configuration", tokens[0].end - tokens[0].start) != 0)) {
		*(configutation + tokens[0].end) = '\0';
		logErrorMessage("Configuration file has wrong structure. It has to start with a string Configuration - instead it starts with %s.", configutation + tokens[0].start);
		return NULL;		
	}

	if (tokens[1].type != JSMN_ARRAY) {
		*(configutation + tokens[0].end) = '\0';
		logErrorMessage("Configuration file has wrong structure. The Configuration field has to contain the array%s.", "");
		return NULL;
	}
	
	GList *allSensorCfg = NULL;
	for (int i = 2; i < tokensParsed; i = i + 7) {
		printf("Processing token %d - type %d\n", i, tokens[i].type);
		if (tokens[i].type != JSMN_OBJECT) {
			logErrorMessage("Configuration token is not an object%s", "");
			return NULL;
		}

		if ((tokens[i+1].type != JSMN_STRING) ||
			(tokens[i+2].type != JSMN_STRING) ||
			(tokens[i+3].type != JSMN_STRING) ||
			(tokens[i+4].type != JSMN_STRING) ||
			(tokens[i+5].type != JSMN_STRING) ||
			(tokens[i+6].type != JSMN_STRING)) {
			logErrorMessage("Configuration for one sensor may consist only of strings. No embedded structures allowed.%s", "");
			return NULL;
		}

		char * sensorTypeStr   = extractStringFromJson(configutation, tokens[i+1]);
		char * sensorTypeValue = extractStringFromJson(configutation, tokens[i+2]);
		char * nameStr         = extractStringFromJson(configutation, tokens[i+3]);
		char * nameValue       = extractStringFromJson(configutation, tokens[i+4]);
		char * sensorAddrStr   = extractStringFromJson(configutation, tokens[i+5]);
		char * sensorAddrValue = extractStringFromJson(configutation, tokens[i+6]);

		if (strcmp(sensorTypeStr, "SensorType") != 0) {
			logErrorMessage("Options in configutation file has to be in strict order. First field has to be SensorType, but %s was found.", sensorTypeStr);
			return NULL;
		}
		if (strcmp(nameStr, "Name") != 0) {
			logErrorMessage("Options in configutation file has to be in strict order. Second field has to be Name, but %s was found.", nameStr);
			return NULL;
		}
		if (strcmp(sensorAddrStr, "SensorAddress") != 0) {
			logErrorMessage("Options in configutation file has to be in strict order. Third field has to be SensorAddress, but %s was found.", sensorAddrStr);
			return NULL;
		}
		struct sensorConfig_t *singleSensor = (struct sensorConfig_t *) malloc(sizeof(struct sensorConfig_t));
		singleSensor->sensorType = sensorTypeValue;
		singleSensor->sensorNameAppendix = nameValue;
		singleSensor->sensorAddress = sensorAddrValue;
		printf("Added following sensor type: %s - name appendix %s - address (if required) %s\n", sensorTypeValue, nameValue, sensorAddrValue);
		allSensorCfg = g_list_append(allSensorCfg, (gpointer) singleSensor);
	}

	return allSensorCfg;
}

void freeSingleSensorConfiguration(gpointer ptr) {
	struct sensorConfig_t *singleSensor = (struct sensorConfig_t *) ptr;
	free(singleSensor->sensorType);
	free(singleSensor->sensorNameAppendix);
	free(singleSensor->sensorAddress);
	free(singleSensor);
}
