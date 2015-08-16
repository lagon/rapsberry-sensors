#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <errno.h>
#include <syslog.h>
#include <glib.h>
#include <unistd.h>

#include "sqliteDbUtilityFunctions.h"
#include "main_event_loop.h"

#include "actionQueue.h"
#include "h21df_action.h"
#include "printSensorsStat_action.h"
#include "sqlite_store_sensor_stat.h"
#include "bmp183_action.h"
#include "save2file_action.h"
#include "ina219_action.h"
#include "mcp9808_action.h"
#include "kbInput_action.h"
#include "led_driver_action.h"

// EXPERIMENTAL --------- 
// #include "led_experiments.h"
// #include "ina219_power_monitor.h"
//#include "ssd1306_oled_display.h"
#include "mcp9808_temperature.h"
#include "mtk3339-gps.h"

// GHashTable *sensorStatus;
// GHashTable *actionOutputs;
// struct actionQueue* aq;

struct mainEventLoopControl_t* eventLoop;

void free_action_output(void *actionOutput) {
    free(actionOutput);
}

GList *getAllActions() {
    GList *allActions = NULL;

    allActions = g_list_append(allActions, &bmp183ActionStructure);
    allActions = g_list_append(allActions, &h21dfActionStructure);
    allActions = g_list_append(allActions, &printActionStructure);
    allActions = g_list_append(allActions, &mcp9808ActionStructure);
    allActions = g_list_append(allActions, &kbInputActionStructure);
    allActions = g_list_append(allActions, &ledDriverActionStructure);
    allActions = g_list_append(allActions, &save2SqlActionStructure);

    return allActions;
}

void initialize() {
    prepareSQLDatabase();

    // sensorStatus = g_hash_table_new(&g_str_hash, &g_str_equal);
    // actionOutputs = g_hash_table_new_full(&g_str_hash, &g_str_equal, NULL, &free_action_output);
    GList *allActions = getAllActions();
    eventLoop = el_initializeEventLoop(allActions);

    //enureAllSensorDescriptionInDB(allActions);

    // aq = aq_initQueue();

    // if (h21df_initActionFunction(sensorStatus) > 0) {
    //     aq_addAction(aq, 1000, &h21df_actionFunction);
    // } else {
    //     syslog(LOG_ERR, "H21DF Humidity sensor initiation failed. Sensor will not run.");
    // }

    // if (bmp183_initActionFunction(sensorStatus) > 0) {
    //     aq_addAction(aq, 5000, &bmp183_actionFunction);
    // } else {
    //     syslog(LOG_ERR, "BMP183 Pressure sensor initiation failed. Sensor will not run.");
    // }

    // if (ina219_initActionFunction(sensorStatus) > 0) {
    //     aq_addAction(aq, 6000, &ina219_actionFunction);
    // } else {
    //     syslog(LOG_ERR, "INA219 Power Consumption sensor initiation failed. Sensor will not run.");
    // }

    // if (mcp9808_initActionFunction(sensorStatus) > 0) {
    //     aq_addAction(aq, 6500, &mcp9808_actionFunction);
    // } else {
    //     syslog(LOG_ERR, "MCP9808 Temperature sensor initiation failed. Sensor will not run.");
    // }

    // if (print_initActionFunction(sensorStatus) > 0) {
    //     aq_addAction(aq, 500000, &print_actionFunction);
    // } else {
    //     syslog(LOG_ERR, "Print on terminal initiation failed. Sensor will not run.");
    // }

    // if (save_actual_initActionFunction(sensorStatus) > 0) {
    //     aq_addAction(aq, 550000, &save_actual_actionFunction);
    // } else {
    //     syslog(LOG_ERR, "Store current value initiation failed. Sensor will not run.");
    // }

    // if (sqliteStore_initActionFunction(sensorStatus) > 0) {
    //     aq_addAction(aq, 600000, &sqliteStore_actionFunction);
    // } else {
    //     syslog(LOG_ERR, "Archive measurements to SQLite initiation failed. Sensor will not run.");
    // }
}

// void mainEventLoop() {
//     while(1) {
//         long usecsToSleep = aq_usecsToNextAction(aq);
//         if (usecsToSleep > 0) {
//             usleep(usecsToSleep);
//         }

//         actionFunction *af = aq_getAction(aq);
//         long long startTime = getCurrentUSecs();
//         long long nextAction = (*af)(actionOutputs, sensorStatus);
//         long long stopTime = getCurrentUSecs();
//         printf("Action %p needed %lld useconds to finish\n", af, stopTime - startTime);
//         aq_addAction(aq, nextAction, af);
//     }
// }


int main(int argc, char **argv) {
    // ssd1306_demo();
    mtk3339_demo();

    // //Syslog start
    // openlog("sensor controller", LOG_CONS | LOG_PERROR, LOG_USER);

    // initialize();    
    // el_runEventLoop(eventLoop);
    return 0;
}