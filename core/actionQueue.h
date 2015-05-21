#include <glib.h>

#ifndef __lagon_action_queue_h__
#define __lagon_action_queue_h__

#include "utilityFunctions.h"

typedef long (actionFunction)(GHashTable*, GHashTable*);

struct actionOutputItem {
	const char *sensorDisplayName;
	long long timeValueMeasured;
	double sensorValue;
} actionOutputItem;

struct actionQueueItem {
	struct actionQueueItem *next;
	long long usecActionTimestamp;
	struct actionDescriptorStructure_t *fnc;
} actionQueueItem;

struct actionQueue {
	struct actionQueueItem *first;
} actionQueue;

// long long getCurrentUSecs();

struct actionQueue *aq_initQueue();

struct actionQueue *aq_addAction(struct actionQueue * queue, long long usecsToAction, struct actionDescriptorStructure_t *fnc);

long long aq_usecsToNextAction(struct actionQueue * queue);

struct actionDescriptorStructure_t *aq_getAction(struct actionQueue * queue);

void destroyQueue(struct actionQueue *queue);

struct actionOutputItem *generateOutputItem(const char* name, double value);

#endif
