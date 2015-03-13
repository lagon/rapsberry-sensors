#include <glib.h>

#ifndef __lagon_action_queue_h__
#define __lagon_action_queue_h__

typedef long (actionFunction)(GHashTable*, GHashTable*);

struct actionOutputItem {
	char *sensorDisplayName;
	long long timeValueMeasured;
	double sensorValue;
} actionOutputItem;

struct actionQueueItem {
	struct actionQueueItem *next;
	long long usecActionTimestamp;
	actionFunction *fnc;
} actionQueueItem;

struct actionQueue {
	struct actionQueueItem *first;
} actionQueue;

long long getCurrentUSecs();

struct actionQueue *aq_initQueue();

struct actionQueue *aq_addAction(struct actionQueue * queue, long long usecsToAction, actionFunction fnc);

long long aq_usecsToNextAction(struct actionQueue * queue);

actionFunction *aq_getAction(struct actionQueue * queue);

void destroyQueue(struct actionQueue *queue);

#endif
