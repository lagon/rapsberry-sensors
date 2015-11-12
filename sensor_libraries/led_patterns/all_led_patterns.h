#ifndef __lagon_all_led_patterns_h__
#define __lagon_all_led_patterns_h__

#include "pattern_queue.h"

#include <sys/types.h>
#include <stdint.h>

struct pa_LedStatesResults *ledPattern_setIntensityInOneStep(int time_step, int totalLeds, uint16_t initial_intensity, uint16_t target_intensity);
struct pa_LedStatesResults *ledPattern_setIntensityMediumFade(int time_step, int totalLeds, uint16_t initial_intensity, uint16_t target_intensity);
struct pa_LedStatesResults *ledPattern_acknowledgeCommand(int time_step, int totalLeds, uint16_t initial_intensity, uint16_t target_intensity);
struct pa_LedStatesResults *ledPattern_darkSegmentSweep(int time_step, int totalLeds, uint16_t initial_intensity, uint16_t target_intensity);
struct pa_LedStatesResults *ledPattern_fiveMinuteDelay(int time_step, int totalLeds, uint16_t initial_intensity, uint16_t target_intensity);
struct pa_LedStatesResults *ledPattern_nightMode(int time_step, int totalLeds, uint16_t initial_intensity, uint16_t target_intensity);

#endif