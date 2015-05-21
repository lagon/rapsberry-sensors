#include "actionDescriptorStructure.h"


//Convenience values for empty inputs/changed inputs...
struct inputsChanged_t noInputsChanged = {
	.numInputsChanged = 0,
	.newInputValues = {}
};

struct inputNotifications_t noInputsToWatch = {
	.numInputsWatched = 0,
	.watchedInputs = {}
};
