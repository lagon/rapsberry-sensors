#include "actionQueue.h"

#ifndef __lagon_mpr121_action_h__
#define __lagon_mpr121_action_h__

#include <glib.h>
#include <syslog.h>
#include <malloc.h>
#include "mpr121.h"

#define __MPR_Key1  "MPR_Key1"
#define __MPR_Key2  "MPR_Key2"
#define __MPR_Key3  "MPR_Key3"
#define __MPR_Key4  "MPR_Key4"
#define __MPR_Key5  "MPR_Key5"
#define __MPR_Key6  "MPR_Key6"
#define __MPR_Key7  "MPR_Key7"
#define __MPR_Key8  "MPR_Key8"
#define __MPR_Key9  "MPR_Key9"
#define __MPR_Key10 "MPR_Key10"
#define __MPR_Key11 "MPR_Key11"
#define __MPR_Key12 "MPR_Key12"
#define __MPR_KeyProximity "MPR_KeyProximity"



const char *mpr121_keyNames[] = {__MPR_Key1,  __MPR_Key2,  __MPR_Key3,
						  		 __MPR_Key4,  __MPR_Key5,  __MPR_Key6,
						    	 __MPR_Key7,  __MPR_Key8,  __MPR_Key9,
						  		 __MPR_Key10, __MPR_Key11, __MPR_Key12,
						  		 __MPR_KeyProximity};

extern const char* mpr121TouchSensorName;
extern const char* mpr121TouchSensorStateName;

extern struct actionDescriptorStructure_t mcp9808ActionStructure;

#endif

