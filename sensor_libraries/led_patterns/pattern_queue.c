#include "pattern_queue.h"
#include <stdlib.h>

const long long pa_neverCallAgain = -1;
const long long pa_wasLastStep = -2;
const long long pa_emptyActionQueue = -3;


struct pa_Queue *pa_initialize(int totalLeds) {
	struct pa_Queue *queue = (struct pa_Queue *) malloc(sizeof(struct pa_Queue));

	queue->totalLeds = totalLeds;
	queue->time_step = 0;
	pa_resetQueue(queue);

	return queue;
}

int pa_nextPosition(int pos) {
	return (pos + 1) % pa_queueLength;
}

int pa_addNextLedAction(struct pa_Queue *queue, patternAction_t action, uint16_t initial_intensity, uint16_t target_intensity) {
	if (pa_nextPosition(queue->next_empty_position) == queue->current_position) {
		return -1;
	}
	queue->ledActions[queue->next_empty_position] = action;
	queue->initial_intensities[queue->next_empty_position] = initial_intensity;
	queue->target_intensities[queue->next_empty_position] = target_intensity;
	queue->next_empty_position = pa_nextPosition(queue->next_empty_position);
	return 0;
}

void pa_getToNextLedAction(struct pa_Queue *queue) {
	if (queue->current_position == queue->next_empty_position) {
		return;
	}
	queue->current_position = pa_nextPosition(queue->current_position);
	queue->time_step = 0;
}

struct pa_LedStatesResults *pa_executeCurrentLedAction(struct pa_Queue *queue) {
	if (queue->current_position == queue->next_empty_position) {
		struct pa_LedStatesResults * ret = (struct pa_LedStatesResults *) malloc(sizeof(struct pa_LedStatesResults));
		ret->nextInvocation = pa_emptyActionQueue;
		ret->totalLeds = queue->totalLeds;
		ret->ledIntensities = (uint16_t *)malloc(sizeof(uint16_t) * ret->totalLeds);
		return ret;
	}

	int totalLeds = queue->totalLeds;
	int time_step = queue->time_step;
	queue->time_step = queue->time_step + 1;
	uint16_t initial_intensity = queue->initial_intensities[queue->current_position];
	uint16_t target_intensity = queue->target_intensities[queue->current_position];
	patternAction_t action = queue->ledActions[queue->current_position];
	return action(time_step, totalLeds, initial_intensity, target_intensity);
}

void pa_resetQueue(struct pa_Queue *queue) {
	queue->current_position = 0;
	queue->next_empty_position = 0;
	queue->time_step = 0;
}

int pa_isQueueEmpty(struct pa_Queue *queue) {
	return (queue->current_position == queue->next_empty_position);
}

void pa_destroyLedStateResults(struct pa_LedStatesResults *ledStatesResult) {
	free(ledStatesResult->ledIntensities);
	free(ledStatesResult);
}
