#ifndef __lagon_pattern_queue_h__
#define __lagon_pattern_queue_h__

#include <sys/types.h>
#include <stdint.h>

const extern long long pa_neverCallAgain;
const extern long long pa_wasLastStep;
const extern long long pa_emptyActionQueue;

#define pa_queueLength 5

struct pa_LedStatesResults {
	long long nextInvocation;
	int totalLeds;
	uint16_t *ledIntensities;
};


typedef struct pa_LedStatesResults *(*patternAction_t)(int time_step, int totalLeds, uint16_t initial_intensity, uint16_t target_intensity);

struct pa_Queue {
	int totalLeds;
	int current_position;
	int next_empty_position;
	int time_step;
	patternAction_t ledActions[pa_queueLength];
	uint16_t initial_intensities[pa_queueLength];
	uint16_t target_intensities[pa_queueLength];
};

struct pa_Queue *pa_initialize(int totalLeds);

int pa_addNextLedAction(struct pa_Queue *queue, patternAction_t action, uint16_t initial_intensity, uint16_t target_intensity);
void pa_getToNextLedAction(struct pa_Queue *queue);

void pa_resetQueue(struct pa_Queue *queue);
int pa_isQueueEmpty(struct pa_Queue *queue);

struct pa_LedStatesResults *pa_executeCurrentLedAction(struct pa_Queue *queue);

void pa_destroyLedStateResults(struct pa_LedStatesResults *ledStatesResult);

#endif