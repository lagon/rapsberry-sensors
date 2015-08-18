#include "mtk3339-gps.h"
#include <stdio.h>
#include <math.h>

struct mtk3339_gps_device {
	int port;
};

enum mtk3339_command {
	PMTK_ACK, 
	PMTK_TXT_MSG, 
	PMTK_CMD_HOT_START,
	PMTK_CMD_WARM_START,
	PMTK_CMD_COLD_START,
	PMTK_CMD_FULL_COLD_START,
	PMTK_SET_NMEA_UPDATERATE,
	PMTK_SET_NMEA_BAUDRATE,
	PMTK_API_SET_NMEA_OUTPUT,
	PMTK_Q_RELEASE,
	PMTK_Q_EPO_INFO,
	PMTK_CMD_STANDBY_MODE,
	PMTK_CMD_PERIODIC_MODE,
	PMTK_CMD_AIC_MODE,
	PMTK_API_SET_DATUM
};

enum nmea_sentence_type_enum {
	GGA, GSA, GSV, RMC, VTG
};

enum fixType_t {
	fix_noFix, fix_gpsFix, fix_differentialFix
};

struct ggaDetails_t {
	uint8_t utcHours;
	uint8_t utcMinutes;
	uint8_t utcSeconds;

	double latitude;
	double longitude;

	enum fixType_t fixType;

	uint8_t satelitesUsed;
	double horizontalDOP;
	double altitudeInM;
};

struct nmea_t {
	enum nmea_sentence_type_enum nmea_sentence_type;
	union {
		struct ggaDetails_t ggaDetails;
		// struct gsaDetails_t gsaDetails;
		// struct gsvDetails_t gsvDetails;
		// struct rmcDetails_t rmcDetails;
		// struct vtgDetails_t vtgDetails;
	};
};

const speed_t communicationSpeed = B9600;

struct mtk3339_gps_device *mtk3339_initialise(char *portName);
void mtk3339_getCheckSum(const char *command, char *checksumText, int checksumTextSize);
void mtk3339_finaliseCommand(const char *command, char *outputBuffer, int bufferSize);
char *mtk3339_getCommandString(enum mtk3339_command command);
int mtk3339_strcopy(char *tgt, int offset, char *src);
char *mtk3339_formatParameters(char *parameters[], int numParameters);
int mtk3339_sendCommand(enum mtk3339_command command, char *parameters[], int numParameters, int port);
int mtk3339_setSerialSpeed(int port, speed_t communicationSpeed);
int mtk3339_coldStart(struct mtk3339_gps_device *device);
int mtk3393_enableAicMode(struct mtk3339_gps_device *device);
int mtk3339_setNmeaUpdateRate(struct mtk3339_gps_device *device, uint32_t updateInterval);
char *mtk3339_getSingleCommand(struct mtk3339_gps_device *device);
int findFirstCharInString(const char *string, char toFind);
int mtk3339_nmeaValidateChecksum(char *singleSentence);
char *nmea_GetNextField(char **unparsedStart, char separator);
struct nmea_t *nmea_parse_ggaSentence(char *unparsedSentencePart);
struct nmea_t *nmea_parseSentence(char *singleSentence);
struct nmea_t *mtk3339_readNmeaInput(struct mtk3339_gps_device *device);


struct mtk3339_gps_device *mtk3339_initialise(char *portName) {
	int port = serial_openPort(portName);
	struct mtk3339_gps_device *device = (struct mtk3339_gps_device *) malloc(sizeof(struct mtk3339_gps_device));
	device->port = port;

	mtk3339_setSerialSpeed(port, communicationSpeed);
	serial_setSpeed(port, communicationSpeed);

	// mtk3339_coldStart(device);
	mtk3393_enableAicMode(device);
	mtk3339_setNmeaUpdateRate(device, 5000);
	mtk3339_setDatumToWGS84(struct mtk3339_gps_device *device);

	return device;
}

void mtk3339_getCheckSum(const char *command, char *checksumText, int checksumTextSize) {
	uint8_t checksum = 0;
	for (int i = 0; i < strlen(command); i++) {
		checksum = checksum ^ command[i];
	}
	snprintf(checksumText, checksumTextSize, "%02X", checksum);
	return;
}

void mtk3339_finaliseCommand(const char *command, char *outputBuffer, int bufferSize) {
	char checksum [10];
	mtk3339_getCheckSum(command, checksum, 10);
	snprintf(outputBuffer, bufferSize, "$%s*%s\r\n", command, checksum);
}

char *mtk3339_getCommandString(enum mtk3339_command command) {
	switch(command) {
		case PMTK_ACK                 : return "PMTK001";
		case PMTK_TXT_MSG             : return "PMTK011";
		case PMTK_CMD_HOT_START       : return "PMTK101";
		case PMTK_CMD_WARM_START      : return "PMTK102";
		case PMTK_CMD_COLD_START      : return "PMTK103";
		case PMTK_CMD_FULL_COLD_START : return "PMTK104";
		case PMTK_SET_NMEA_UPDATERATE : return "PMTK220";
		case PMTK_SET_NMEA_BAUDRATE   : return "PMTK251";
		case PMTK_API_SET_NMEA_OUTPUT : return "PMTK314";
		case PMTK_Q_RELEASE           : return "PMTK605";
		case PMTK_Q_EPO_INFO          : return "PMTK607";
		case PMTK_CMD_STANDBY_MODE    : return "PMTK161";
		case PMTK_CMD_PERIODIC_MODE   : return "PMTK225";
		case PMTK_CMD_AIC_MODE        : return "PMTK286";
		case PMTK_API_SET_DATUM       : return "PMTK330";
	}
	return NULL;
}

int mtk3339_strcopy(char *tgt, int offset, char *src) {
	for (int i = 0; i < strlen(src); i++) {
		tgt[offset + i] = src[i];
	}
	return offset + strlen(src);
}

char *mtk3339_formatParameters(char *parameters[], int numParameters) {
	int totalLength = 1;
	for (int i = 0; i < numParameters; i++) {
		totalLength = totalLength + strlen(parameters[i]) + 2;
	}
	char *paramString = (char *) malloc(sizeof(char) * totalLength);
	memset(paramString, 0, sizeof(char) * totalLength);
	if (numParameters == 0) {
		return paramString;
	}

	int psPosition = 0;
	for (int i = 0; i < numParameters; i++) {
		paramString[psPosition] = ',';
		psPosition = mtk3339_strcopy(paramString, psPosition+1, parameters[i]);
	}
	return paramString;
}

int mtk3339_sendCommand(enum mtk3339_command command, char *parameters[], int numParameters, int port) {
	char *commandText = mtk3339_getCommandString(command);
	char *paramText = mtk3339_formatParameters(parameters, numParameters);
	
	char text[1024];
	char buffer[1024];

	snprintf(text, 1024, "%s%s", commandText, paramText);

	mtk3339_finaliseCommand(text, buffer, 1024);
	printf("Command to send: %s\n", buffer);
	int bytesWritten = serial_write(port, buffer, strlen(buffer));
	free(paramText);
	return bytesWritten;
}

int mtk3339_setSerialSpeed(int port, speed_t communicationSpeed) {
	char *param [1];
	switch (communicationSpeed) {
		case B115200: param[0] = "115200"; break;
		case B57600:  param[0] = "57600" ; break;
		case B38400:  param[0] = "38400" ; break;
		case B19200:  param[0] = "19200" ; break;
		case B9600:   param[0] = "9600"  ; break;
		case B4800:   param[0] = "4800"  ; break;
		default: return -2;
	}
	return mtk3339_sendCommand(PMTK_SET_NMEA_BAUDRATE, param, 1, port);
}

int mtk3339_coldStart(struct mtk3339_gps_device *device) {
	char *param[1];
	return mtk3339_sendCommand(PMTK_CMD_FULL_COLD_START, param, 0, device->port);
}

int mtk3393_enableAicMode(struct mtk3339_gps_device *device) {
	char *param[1];
	param[0] = "1";
	return mtk3339_sendCommand(PMTK_CMD_AIC_MODE, param, 1, device->port);
}
	
int mtk3339_setNmeaUpdateRate(struct mtk3339_gps_device *device, uint32_t updateInterval) {
	char *param[1];
	param[0] = "1";
	return mtk3339_sendCommand(PMTK_SET_NMEA_UPDATERATE, param, 1, device->port);
}

int mtk3339_setDatumToWGS84(struct mtk3339_gps_device *device) {
	char *param[1];
	param[0] = "0";
	return mtk3339_sendCommand(PMTK_API_SET_DATUM, param, 1, device->port);	
}

char *mtk3339_getSingleCommand(struct mtk3339_gps_device *device) {
	enum singleCommandState_t {NO_START_CHAR, READ_COMMAND, FINISHED};

	char buffer[1024];
	int position = 0;
	memset(buffer, 0, 1024);
	enum singleCommandState_t state = NO_START_CHAR;
	int timeout = 0;

	while(state != FINISHED) {
		char ch;
		int readBytes = read(device->port, &ch, 1);
		if (readBytes == 0) {
			timeout = timeout + 1;
			if (timeout > 1000) {
				return NULL;
			}
		} else {
			timeout = 0;
			switch(state) {
				case NO_START_CHAR : 
					if (ch == '$') {
						state = READ_COMMAND;
						buffer[position] = ch;
						position++;
					}
					break;
				case READ_COMMAND : 
					buffer[position] = ch;
					position++;
					if (ch == '\n') {
						state = FINISHED;
						break;
					}
				case FINISHED :
					break;
			}
		}
	}
	char *retBuffer = (char *)malloc(sizeof(char) * (strlen(buffer) + 1));
	strcpy(retBuffer, buffer);
	return retBuffer;
}

int findFirstCharInString(const char *string, char toFind) {
	for (int i = 0; i < strlen(string); i++) {
		if (string[i] == toFind) {
			return i;
		}
	}
	return -1;
}

int mtk3339_nmeaValidateChecksum(char *singleSentence) {
	int checksumStart = findFirstCharInString(singleSentence, '*');
	if (checksumStart == -1) {
		return 1;
	}
	char *checksumText = singleSentence + checksumStart + 1;
	singleSentence[checksumStart] = '\0';
	char calculatedCheckSum[5];
	mtk3339_getCheckSum(singleSentence+1, calculatedCheckSum, 5);
	//printf("Recieved checksum is %s should be %s\n", calculatedCheckSum, checksumText);
	int result = strncmp(calculatedCheckSum, checksumText, 2);
	singleSentence[checksumStart] = '*';	
	return result;
}

char *nmea_GetNextField(char **unparsedStart, char separator) {
	int nextSeparatorPos = findFirstCharInString(*unparsedStart, separator);
	// printf("Next comma at %d\n", nextSeparatorPos);
	if (nextSeparatorPos == -1) {
		return *unparsedStart;
	}
	(*unparsedStart)[nextSeparatorPos] = '\0';
	char *ret = *unparsedStart;
	*unparsedStart = (*unparsedStart) + nextSeparatorPos + 1;
	return ret;
}

struct nmea_t *nmea_parse_ggaSentence(char *unparsedSentencePart) {
	struct nmea_t *gga = (struct nmea_t *) malloc(sizeof(struct nmea_t));
	gga->nmea_sentence_type = GGA;
	char *utcTimeStr = nmea_GetNextField(&unparsedSentencePart, ',');
	char *latitudeStr = nmea_GetNextField(&unparsedSentencePart, ',');
	char *latitudeIndicatorStr = nmea_GetNextField(&unparsedSentencePart, ',');
	char *longitudeStr = nmea_GetNextField(&unparsedSentencePart, ',');
	char *longitudeIndicatorStr = nmea_GetNextField(&unparsedSentencePart, ',');
	char *fixTypeStr = nmea_GetNextField(&unparsedSentencePart, ',');
	char *satelitesUsedStr = nmea_GetNextField(&unparsedSentencePart, ',');
	char *horizontalDopStr = nmea_GetNextField(&unparsedSentencePart, ',');
	char *mslAltitudeStr = nmea_GetNextField(&unparsedSentencePart, ',');
//	char *mslAltitudeUnitsStr = nmea_GetNextField(&unparsedSentencePart, ',');

	if (strlen(utcTimeStr) != 10) {
		gga->ggaDetails.utcHours = 0;
		gga->ggaDetails.utcMinutes = 0;
		gga->ggaDetails.utcSeconds = 0;
	} else {
		char buffer[3];
		buffer[2] = '\0';

		buffer[0] = utcTimeStr[0];
		buffer[1] = utcTimeStr[1];
		gga->ggaDetails.utcHours = strtol(buffer, NULL, 10);
		buffer[0] = utcTimeStr[2];
		buffer[1] = utcTimeStr[3];
		gga->ggaDetails.utcMinutes = strtol(buffer, NULL, 10);
		buffer[0] = utcTimeStr[4];
		buffer[1] = utcTimeStr[5];		
		gga->ggaDetails.utcSeconds = strtol(buffer, NULL, 10);
	}


	// printf("Latitude - %s, %s, %d\n", latitudeIndicatorStr, latitudeStr, strlen(latitudeStr));
	char *check;
	gga->ggaDetails.latitude = strtod(latitudeStr, &check);
	if (check == latitudeStr) {
		gga->ggaDetails.latitude = INFINITY;
	} else {
		gga->ggaDetails.latitude = gga->ggaDetails.latitude / 100;
	}

	if (latitudeIndicatorStr[0] == 'S') {
		gga->ggaDetails.latitude = -1 * gga->ggaDetails.latitude;
	}

	// printf("Longitude - %s, %s, %d\n", longitudeIndicatorStr, longitudeStr, strlen(longitudeStr));
	gga->ggaDetails.longitude = strtod(longitudeStr, &check);
	if (check == longitudeStr) {
		gga->ggaDetails.longitude = INFINITY;
	} else {
		gga->ggaDetails.longitude = gga->ggaDetails.longitude / 100;
	}

	if (longitudeIndicatorStr[0] == 'W') {
		gga->ggaDetails.longitude = -1 * gga->ggaDetails.longitude;
	}

	if(fixTypeStr[0] == '0') {
		gga->ggaDetails.fixType = fix_noFix;
	} else if (fixTypeStr[0] == '1') {
		gga->ggaDetails.fixType = fix_gpsFix;
	} else if (fixTypeStr[0] == '2') {
		gga->ggaDetails.fixType = fix_differentialFix;
	}

	gga->ggaDetails.satelitesUsed = strtol(satelitesUsedStr, NULL, 10);
	gga->ggaDetails.horizontalDOP = strtod(horizontalDopStr, NULL);
	gga->ggaDetails.altitudeInM   = strtod(mslAltitudeStr, NULL);

	return gga;
}

struct nmea_t *nmea_parseSentence(char *singleSentence) {
	if (strncmp("$GP", singleSentence, 3) != 0) {
		printf("Sentence does not start with $GP\n");
		return NULL;
	}

	if (mtk3339_nmeaValidateChecksum(singleSentence) != 0) {
		printf("Checksum invalid\n");
		return NULL;
	}

	char *unparsedStart = singleSentence;
	char *nmeaSentenceType = nmea_GetNextField(&unparsedStart, ',');
	struct nmea_t *nmea;
	if (strcmp(nmeaSentenceType, "$GPGGA") == 0) {
		nmea = nmea_parse_ggaSentence(unparsedStart);
	} else {
		//printf("Sentence type not recognized\n");
		nmea = NULL;
	}

	return nmea;
}

struct nmea_t *mtk3339_readNmeaInput(struct mtk3339_gps_device *device) {
	char *nmeaSentence = mtk3339_getSingleCommand(device);
//	printf("Command received: %s\n", nmeaSentence);
	if (nmeaSentence == NULL) {
		return NULL;
	}
	struct nmea_t *nmea = nmea_parseSentence(nmeaSentence);
	free(nmeaSentence);
	return nmea;
}

void mtk3339_recordPositionToFile(const char *filename, const struct nmea_t *nmea) {
	FILE *f = fopen(filename, "a");
	if (f == NULL) {
		return;
	}
	printf("[%d-%d-%d] - Lat: %g, lon: %g\n", nmea->ggaDetails.utcHours, nmea->ggaDetails.utcMinutes, nmea->ggaDetails.utcSeconds, nmea->ggaDetails.latitude, nmea->ggaDetails.longitude);
	fprintf(f, "%02d:%02d:%02d;%g;%g;%g;%g;%d\n", 
		nmea->ggaDetails.utcHours, 
		nmea->ggaDetails.utcMinutes, 
		nmea->ggaDetails.utcSeconds, 
		nmea->ggaDetails.latitude, 
		nmea->ggaDetails.longitude,
		nmea->ggaDetails.altitudeInM,
		nmea->ggaDetails.horizontalDOP,
		nmea->ggaDetails.satelitesUsed
	);
	fclose(f);
}

void mtk3339_demo() {
	struct mtk3339_gps_device *gps = mtk3339_initialise("/dev/ttyAMA0");
	printf("Initialition complete\n");
	while(1) {
		struct nmea_t *nmea = mtk3339_readNmeaInput(gps);
		if ((nmea != NULL) && (nmea->nmea_sentence_type == GGA)) {
			if (nmea->ggaDetails.satelitesUsed > 0) {
				mtk3339_recordPositionToFile("./positions.csv", nmea);
		 	} else {
			 	printf("[%d-%d-%d] - No satelites available\n", nmea->ggaDetails.utcHours, nmea->ggaDetails.utcMinutes, nmea->ggaDetails.utcSeconds);
			}
			free(nmea);
		}
	}
}