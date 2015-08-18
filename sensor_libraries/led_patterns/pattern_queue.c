#include "pattern_queue.h"
#include <stdlib.h>

int pa_nextPosition(int pos) {
	return (pos + 1) % pa_queueLength;
}

int pa_addNextLedAction(struct pa_Queue *queue, pattern_action *action, uint16_t initial_intensity, uint16_t target_intensity) {
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
}

struct pa_LedStatesResults *pa_executeCurrentLedAction(struct pa_Queue *queue, int time_step) {
	if (queue->current_position == queue->next_empty_position) {
		struct pa_LedStatesResults * ret = (struct pa_LedStatesResults *) malloc(sizeof(struct pa_LedStatesResults));
		ret->nextInvocation = pa_emptyActionQueue;
		ret->totalLeds = queue->totalLeds;
		ret->ledIntensities = (uint16_t *)malloc(sizeof(uint16_t) * ret->totalLeds);
		return ret;
	}

	int totalLeds = queue->totalLeds;
	uint16_t initial_intensity = queue->initial_intensities[queue->current_position];
	uint16_t target_intensity = queue->target_intensities[queue->current_position];
	return queue->ledActions[queue->current_position](time_step, totalLeds, initial_intensity, target_intensity);
}

void pa_resetQueue(struct pa_Queue *queue) {
	queue->current_position = 0;
	queue->next_empty_position = 0;
}

void pa_destroyLedStateResults(struct pa_LedStatesResults *ledStatesResult) {
	free(ledStatesResult->ledIntensities);
	free(ledStatesResult);
}
