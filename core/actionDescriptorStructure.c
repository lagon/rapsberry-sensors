#include "actionDescriptorStructure.h"
#include <malloc.h>

struct inputsChanged_t *generateNoInputsChanged() {
	struct inputsChanged_t *noInputsChanged = (struct inputsChanged_t *) malloc(sizeof(struct inputsChanged_t));
	noInputsChanged->numInputsChanged = 0;
	return noInputsChanged;
}

struct inputNotifications_t noInputsToWatch = {
	.numInputsWatched = 0,
	.watchedInputs = {}
};
