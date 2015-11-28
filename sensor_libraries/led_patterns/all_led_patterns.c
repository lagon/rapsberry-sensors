#include "pattern_queue.h"
#include "all_led_patterns.h"

#include <malloc.h>


struct pa_LedStatesResults *allocateReturnStruct(int totalLeds) {
	struct pa_LedStatesResults *ret = (struct pa_LedStatesResults *) malloc(sizeof(struct pa_LedStatesResults));
	ret->ledIntensities = (uint16_t *)malloc(sizeof(uint16_t) * totalLeds);
	ret->totalLeds = totalLeds;
	return ret;
}

void setIntensityToAllLeds(struct pa_LedStatesResults *ledStates, uint16_t intensity) {
	for (int ledID = 0; ledID < ledStates->totalLeds; ledID++) {
		ledStates->ledIntensities[ledID] = intensity;
	}
}

struct pa_LedStatesResults *ledPattern_setIntensityInOneStep(int time_step, int totalLeds, uint16_t initial_intensity, uint16_t target_intensity) {
	struct pa_LedStatesResults * ret = allocateReturnStruct(totalLeds);
	setIntensityToAllLeds(ret, target_intensity);
	ret->nextInvocation = pa_neverCallAgain;
	return ret;
}

struct pa_LedStatesResults *ledPattern_setIntensityMediumFade(int time_step, int totalLeds, uint16_t initial_intensity, uint16_t target_intensity) {
	const int max_time_step = 100;
	struct pa_LedStatesResults * ret = allocateReturnStruct(totalLeds);
	double step = ((double)(target_intensity - initial_intensity)) / ((double) max_time_step);
	uint16_t intensity = (uint16_t)((step * time_step) + initial_intensity);

	setIntensityToAllLeds(ret, intensity);
	if (time_step >= max_time_step) {
		ret->nextInvocation = pa_wasLastStep;
	} else {
		ret->nextInvocation = 10 * 1000;
	}
	return ret;
}

struct pa_LedStatesResults *ledPattern_acknowledgeCommand(int time_step, int totalLeds, uint16_t initial_intensity, uint16_t target_intensity) {
	struct pa_LedStatesResults * ret = allocateReturnStruct(totalLeds);
	uint16_t intensity;
	if (initial_intensity > 0x8000) {
		intensity = initial_intensity - 0x4000;
	} else {
		intensity = initial_intensity + 0x4000;
	}
	
	switch(time_step) {
		case 0:
			setIntensityToAllLeds(ret, initial_intensity);
			ret->nextInvocation = 500 * 1000;
			break;
		case 1:
			setIntensityToAllLeds(ret, intensity);
			ret->nextInvocation = 25 * 1000;
			break;
		case 2:
			setIntensityToAllLeds(ret, initial_intensity);
			ret->nextInvocation = 500 * 1000;
			break;
		case 3:
			setIntensityToAllLeds(ret, intensity);
			ret->nextInvocation = 25 * 1000;
			break;
		case 4:
			setIntensityToAllLeds(ret, initial_intensity);
			ret->nextInvocation = pa_wasLastStep;
			break;
	}
	return ret;
}

struct pa_LedStatesResults *ledPattern_fiveMinuteDelay(int time_step, int totalLeds, uint16_t initial_intensity, uint16_t target_intensity) {
	int max_time_step = 61;
	struct pa_LedStatesResults * ret = allocateReturnStruct(totalLeds);
	setIntensityToAllLeds(ret, initial_intensity);
	int ledID = (time_step * totalLeds) / max_time_step;

	if (time_step % 2 == 1) {
		ret->nextInvocation = 10 * 1000 * 1000;
		ret->ledIntensities[ledID] = initial_intensity;
	} else {
		ret->nextInvocation = 50 * 1000;
		ret->ledIntensities[ledID] = target_intensity;
	}

	if (time_step >= max_time_step) {
		ret->nextInvocation = pa_wasLastStep;
	}

	return ret;
}

struct pa_LedStatesResults *ledPattern_nightMode(int time_step, int totalLeds, uint16_t initial_intensity, uint16_t target_intensity) {
	struct pa_LedStatesResults * ret = allocateReturnStruct(totalLeds);
	setIntensityToAllLeds(ret, 0x0000);

	ret->ledIntensities[0] = 0x1000;
	ret->ledIntensities[1] = 0x0800;
	ret->ledIntensities[2] = 0x0100;
	ret->ledIntensities[3] = 0x0080;
	ret->ledIntensities[4] = 0x0010;
	ret->ledIntensities[5] = 0x0008;
	ret->nextInvocation = pa_neverCallAgain;
	return ret;
}
