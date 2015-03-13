#include "led_experiments.h"
#include <stdio.h>
#include <math.h>

double logisticRegression(double x, double offset, double slope) {
	return (1 / (1 + exp(-(x * slope - offset))));
}


uint16_t turn_on_fluorescent_lamp(int timestep, int max_timestep) {
	double ledOnProbability = (double) rand() / (double) RAND_MAX;
	double thresholdProbability = logisticRegression((double)timestep / (double)max_timestep, 5.0, 10.0);
	printf("%f\n", thresholdProbability);
	if (ledOnProbability < thresholdProbability) {
		return 65535;
	} else {
		return 0;
	}
}

uint16_t turn_off_decay(int timestep, int max_timestep) {
	double progress = (double)timestep / (double) max_timestep;
	double level = 65535.0 * (1 - logisticRegression(progress, 7.5, 15.0));
	printf("%d\n", (uint16_t)level);
	return (uint16_t)level;
}


void run_led_experiment() {

	int spidev = spi_initDevice(0, 0);
	if (spidev < 0) {
		perror("");
		exit(-1);
	}
	struct allLedControlStruct *ledctrl = initiateLEDControls(1);
	setGlobalBrightness(ledctrl, 127);
	struct ledPWMSettings led_settings;
	led_settings.blank  = 0;
	led_settings.dsprpt = 1;
	led_settings.tmgrst = 1;
	led_settings.extgck = 0;
	led_settings.outtmg = 1;
	setLedSettings(ledctrl, &led_settings);

	// int led_power = 0;
	// int timestep = 0;
	while (1) {
		for (int i = 0; i < 50; i++) {
			for (int j = 0; j < 6; j++) {
				setOneGrayscaleLed(ledctrl, j, turn_on_fluorescent_lamp(i, 50));
			}
			sendOutLedDataDefaults(ledctrl, spidev);
			usleep(100000);
		};

		for (int j = 0; j < 6; j++) {
			setOneGrayscaleLed(ledctrl, j, 0xFFFF);
		}
		sendOutLedDataDefaults(ledctrl, spidev);
		sleep(10);

		for (int i = 0; i < 100; i++) {
			for (int j = 0; j < 6; j++) {
				setOneGrayscaleLed(ledctrl, j, turn_off_decay(i, 100));
			}
			sendOutLedDataDefaults(ledctrl, spidev);
			usleep(100000);
		};


		for (int j = 0; j < 6; j++) {
			setOneGrayscaleLed(ledctrl, j, 0);
		}
		sendOutLedDataDefaults(ledctrl, spidev);
		sleep(10);
	}
}
