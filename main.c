#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include <syslog.h>
#include <glib.h>
#include <unistd.h>

#include "actionQueue.h"
#include "h21df_action.h"
#include "printSensorsStat_action.h"
#include "sqlite_store_sensor_stat.h"
#include "bmp183_action.h"
#include "save2file_action.h"

// EXPERIMENTAL --------- 
#include "led_experiments.h"
#include "ina219_power_monitor.h"
#include "ssd1306_oled_display.h"

GHashTable *sensorStatus;
GHashTable *actionOutputs;
struct actionQueue* aq;

void free_action_output(void *actionOutput) {
    free(actionOutput);
}

void initialize() {
    aq = aq_initQueue();
    sensorStatus = g_hash_table_new(&g_str_hash, &g_str_equal);
    actionOutputs = g_hash_table_new_full(&g_str_hash, &g_str_equal, NULL, &free_action_output);

    h21df_initActionFunction(sensorStatus);
    aq_addAction(aq, 1000, &h21df_actionFunction);

    bmp183_initActionFunction(sensorStatus);
    aq_addAction(aq, 5000, &bmp183_actionFunction);

    print_initActionFunction(sensorStatus);
    aq_addAction(aq, 500000, &print_actionFunction);

   save_actual_initActionFunction(sensorStatus);
   aq_addAction(aq, 550000, &save_actual_actionFunction);

  //  sqliteStore_initActionFunction(sensorStatus);
  // aq_addAction(aq, 600000, &sqliteStore_actionFunction);
}

void mainEventLoop() {
    while(1) {
        long usecsToSleep = aq_usecsToNextAction(aq);
        if (usecsToSleep > 0) {
            usleep(usecsToSleep);
        }

        actionFunction *af = aq_getAction(aq);
        long long startTime = getCurrentUSecs();
        long long nextAction = (*af)(actionOutputs, sensorStatus);
        long long stopTime = getCurrentUSecs();
        printf("Action %p needed %lld useconds to finish\n", af, stopTime - startTime);
        aq_addAction(aq, nextAction, af);
    }
}


int main(int argc, char **argv) {

    //Syslog start
    openlog("sensor controller", LOG_CONS | LOG_PERROR, LOG_USER);


    ina219_testMeasurement();
    ssd1306_demo();


    // initialize();
    // mainEventLoop();
    // run_led_experiment();
    closelog();
}