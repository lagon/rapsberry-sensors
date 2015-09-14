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
#include "sensorConfigParser.h"

// EXPERIMENTAL --------- 
// #include "led_experiments.h"
// #include "ina219_power_monitor.h"
//#include "ssd1306_oled_display.h"
#include "mtk3339-gps.h"

struct mainEventLoopControl_t* mainEventLoop;

void free_action_output(void *actionOutput) {
    free(actionOutput);
}

GHashTable *getAllAvailableActions() {
    GHashTable *allAvailableActions = g_hash_table_new(&g_str_hash, &g_str_equal);
    g_hash_table_replace(allAvailableActions, bmp183ActionStructure.sensorType,    &bmp183ActionStructure);
    g_hash_table_replace(allAvailableActions, h21dfActionStructure.sensorType,     &h21dfActionStructure);
    g_hash_table_replace(allAvailableActions, printActionStructure.sensorType,     &printActionStructure);
    g_hash_table_replace(allAvailableActions, mcp9808ActionStructure.sensorType,   &mcp9808ActionStructure);
    g_hash_table_replace(allAvailableActions, kbInputActionStructure.sensorType,   &kbInputActionStructure);
    g_hash_table_replace(allAvailableActions, ledDriverActionStructure.sensorType, &ledDriverActionStructure);
    g_hash_table_replace(allAvailableActions, save2SqlActionStructure.sensorType,  &save2SqlActionStructure);

    return allAvailableActions;
}

void initialize(const char *configurationFile) {
    prepareSQLDatabase();

    GHashTable *allActions = getAllAvailableActions();
    GList *configuredSensors = readConfigurationFile(configurationFile);
    mainEventLoop = el_initializeEventLoop(allActions, configuredSensors);
    g_list_free_full(configuredSensors, &freeSingleSensorConfiguration);
}

int main(int argc, char **argv) {
    readConfigurationFile("homepi.sensors.json");
    exit(-1);
    //Syslog start
    openlog("sensor controller", LOG_CONS | LOG_PERROR, LOG_USER);

    initialize();    
    el_runEventLoop(mainEventLoop);
    return 0;
}