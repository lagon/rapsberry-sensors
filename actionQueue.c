#include "actionQueue.h"
#include <malloc.h>
#include <syslog.h>
#include <sys/time.h>

struct actionQueue *aq_initQueue() {
	struct actionQueue *aq = (struct actionQueue *) malloc(sizeof(struct actionQueue));
	aq->first = NULL;
	return aq;
}

long long getCurrentUSecs() {
	struct timeval  tv;
	gettimeofday(&tv, NULL);

	long long currentUSecs = (long long)tv.tv_sec * (long long)1000 * (long long)1000 + (long long)tv.tv_usec;
	return currentUSecs;
}

struct actionOutputItem *generateOutputItem(const char* name, double value) {
	struct actionOutputItem *ao_value = (struct actionOutputItem *) malloc(sizeof(struct actionOutputItem));
	ao_value->sensorDisplayName = name;
	ao_value->timeValueMeasured = getCurrentUSecs();
	ao_value->sensorValue       = value;
	return ao_value;
}

long long getUSecTimestamp(long long usecToAction) {
	return getCurrentUSecs() + usecToAction;
}

struct actionQueueItem *getItemBeforeTime(struct actionQueueItem *first, long long usec_timestamp) {
	if (first == NULL) {
		return NULL;
	}

	if (first->usecActionTimestamp > usec_timestamp) {
		return NULL;
	}


	struct actionQueueItem *item = first;

	while (item->next != NULL) {
		if (item->next->usecActionTimestamp > usec_timestamp) {
			return item;
		}
		item = item->next;
	}
	return item;
}

void printActionQueue(struct actionQueue * queue) {
	printf("Action queue:");
	struct actionQueueItem * item = queue->first;
	while (item != NULL) {
		printf("%p (%lld), ", item->fnc, item->usecActionTimestamp / 1000);
		item = item->next;
	}
	printf("\n");
}

struct actionQueue *aq_addAction(struct actionQueue * queue, long long usecsToAction, actionFunction fnc) {
	long long actionUSecTime = getUSecTimestamp(usecsToAction);
	struct actionQueueItem *item = getItemBeforeTime(queue->first, actionUSecTime);
	struct actionQueueItem *tmp  = (struct actionQueueItem *)malloc(sizeof(struct actionQueueItem));

	if (item == NULL) {
		tmp->next = queue->first;
		queue->first = tmp;
		item = queue->first;
	} else {
		tmp->next = item->next;
		item->next = tmp;
		item = tmp;
	}

	item->usecActionTimestamp = actionUSecTime;
	item->fnc = fnc;
//	printf("Adding action function %p to comence at %lld (in %lld us)\n", fnc, actionUSecTime, actionUSecTime - getCurrentUSecs());
//	printActionQueue(queue);
	return queue;
}

long long aq_usecsToNextAction(struct actionQueue * queue) {
	if (queue->first != NULL) {
		long long diff = queue->first->usecActionTimestamp - getCurrentUSecs();
		return diff > 0 ? diff : 0;
	} else {
		syslog(LOG_WARNING, "No action recorded in the action queue.");
		return -1;
	}
}

actionFunction *aq_getAction(struct actionQueue * queue) {
	if (queue->first == NULL) {
		syslog(LOG_WARNING, "No action recorded in the action queue.");
		return NULL;
	}
	struct actionQueueItem *tmp = queue->first;
	actionFunction *af = tmp->fnc;
	queue->first = queue->first->next;
	free(tmp);
	return af;
}

void destroyQueue(struct actionQueue *queue) {
	while (queue->first != NULL) {
		struct actionQueueItem *tmp = queue->first;
		queue->first = queue->first->next;
		free(tmp);
	}
}
