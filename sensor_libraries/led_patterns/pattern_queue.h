#ifndef __lagon_pattern_queue_h__
#define __lagon_pattern_queue_h__

#include <sys/types.h>
#include <stdint.h>

const long long pa_neverCallAgain = -1;
const long long pa_wasLastStep = -2;
const long long pa_emptyActionQueue = -3;

#define pa_queueLength 5

struct pa_LedStatesResults {
	long long nextInvocation;
	int totalLeds;
	uint16_t *ledIntensities;
};


typedef struct pa_LedStatesResults *(*pattern_action)(int time_step, int totalLeds, uint16_t initial_intensity, uint16_t target_intensity);

struct pa_Queue {
	int totalLeds;
	int current_position;
	int next_empty_position;
	pattern_action ledActions[pa_queueLength];
	uint16_t initial_intensities[pa_queueLength];
	uint16_t target_intensities[pa_queueLength];
};

struct pa_Queue *pa_initialize(int totalLeds);

int pa_addNextLedAction(struct pa_Queue *queue, pattern_action *action, uint16_t initial_intensity, uint16_t target_intensity);
void pa_getToNextLedAction(struct pa_Queue *queue);

void pa_resetQueue(struct pa_Queue *queue);
int pa_isQueueEmpty(struct pa_Queue *queue);

struct pa_LedStatesResults *pa_executeCurrentLedAction(struct pa_Queue *queue, int time_step);

void pa_destroyLedStateResults(struct pa_LedStatesResults *ledStatesResult);

#endif