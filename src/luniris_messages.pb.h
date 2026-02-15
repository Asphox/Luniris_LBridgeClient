/* Automatically generated nanopb header */
/* Generated manually for luniris_messages.proto */

#ifndef LUNIRIS_MESSAGES_PB_H
#define LUNIRIS_MESSAGES_PB_H

#include <pb.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Struct definitions */

/* Normalized eye coordinates (-1 to 1) */
typedef struct _EyeCoordinates
{
	float x; /* Horizontal position (-1: left, 1: right) */
	float y; /* Vertical position (-1: down, 1: up) */
} EyeCoordinates;

/* Eyelid closure state (0 to 1) */
typedef struct _EyelidState
{
	float closure_percentage; /* 0: fully open, 1: fully closed */
} EyelidState;

/* Brightness level (0 to 100) */
typedef struct _BrightnessMessage
{
	int32_t brightness_level; /* 0: low brightness, 100: max brightness */
} BrightnessMessage;

/* 3-axis sensor measurements */
typedef struct _InertialMeasurementValues
{
	float x; /* X-axis measurement */
	float y; /* Y-axis measurement */
	float z; /* Z-axis measurement */
} InertialMeasurementValues;

/* Temperature measurement */
typedef struct _TemperatureValue
{
	float temperature; /* Temperature in degrees Celsius */
} TemperatureValue;

/* Actions API - size limits */
#define LUNIRIS_ACTION_KEY_MAX_SIZE          48
#define LUNIRIS_ACTION_NAME_MAX_SIZE         48
#define LUNIRIS_ACTION_ARG_MAX_SIZE          64
#define LUNIRIS_ACTIONS_MAX_COUNT            32

/* Action source enumeration */
typedef enum _ActionSource
{
	ACTION_SOURCE_RESOURCE_PACK = 0,
	ACTION_SOURCE_FEATURE = 1
} ActionSource;

/* Message structure for sending actions */
typedef struct _ActionMessage
{
	char key[LUNIRIS_ACTION_KEY_MAX_SIZE + 1];
	char argument[LUNIRIS_ACTION_ARG_MAX_SIZE + 1];
} ActionMessage;

/* Complete action definition */
typedef struct _Action
{
	char key[LUNIRIS_ACTION_KEY_MAX_SIZE + 1];
	char name[LUNIRIS_ACTION_NAME_MAX_SIZE + 1];
	bool is_available;
	int32_t source;
} Action;

/* Collection of all available actions */
typedef struct _Actions
{
	pb_size_t actions_count;
	Action actions[LUNIRIS_ACTIONS_MAX_COUNT];
} Actions;

/* Initializer values for message structs */
#define EyeCoordinates_init_default              {0, 0}
#define EyelidState_init_default                 {0}
#define BrightnessMessage_init_default           {0}
#define EyeCoordinates_init_zero                 {0, 0}
#define EyelidState_init_zero                    {0}
#define BrightnessMessage_init_zero              {0}
#define InertialMeasurementValues_init_default   {0, 0, 0}
#define TemperatureValue_init_default            {0}
#define InertialMeasurementValues_init_zero      {0, 0, 0}
#define TemperatureValue_init_zero               {0}
#define ActionMessage_init_default               {"", ""}
#define ActionMessage_init_zero                  {"", ""}
#define Action_init_default                      {"", "", false, 0}
#define Action_init_zero                         {"", "", false, 0}
#define Actions_init_default                     {0, {Action_init_default}}
#define Actions_init_zero                        {0, {Action_init_zero}}

/* Field tags (for use in manual encoding/decoding) */
#define EyeCoordinates_x_tag                     1
#define EyeCoordinates_y_tag                     2
#define EyelidState_closure_percentage_tag       1
#define BrightnessMessage_brightness_level_tag   1
#define InertialMeasurementValues_x_tag          1
#define InertialMeasurementValues_y_tag          2
#define InertialMeasurementValues_z_tag          3
#define TemperatureValue_temperature_tag         1
#define ActionMessage_key_tag                    1
#define ActionMessage_argument_tag               2
#define Action_key_tag                           1
#define Action_name_tag                          2
#define Action_is_available_tag                  4
#define Action_source_tag                        5
#define Actions_actions_tag                      1

/* Struct field encoding specification for nanopb */
#define EyeCoordinates_FIELDLIST(X, a) \
	X(a, STATIC, SINGULAR, FLOAT, x, 1) \
	X(a, STATIC, SINGULAR, FLOAT, y, 2)
#define EyeCoordinates_CALLBACK               NULL
#define EyeCoordinates_DEFAULT                NULL

#define EyelidState_FIELDLIST(X, a) \
	X(a, STATIC, SINGULAR, FLOAT, closure_percentage, 1)
#define EyelidState_CALLBACK                  NULL
#define EyelidState_DEFAULT                   NULL

#define BrightnessMessage_FIELDLIST(X, a) \
	X(a, STATIC, SINGULAR, INT32, brightness_level, 1)
#define BrightnessMessage_CALLBACK            NULL
#define BrightnessMessage_DEFAULT             NULL

#define InertialMeasurementValues_FIELDLIST(X, a) \
	X(a, STATIC, SINGULAR, FLOAT, x, 1) \
	X(a, STATIC, SINGULAR, FLOAT, y, 2) \
	X(a, STATIC, SINGULAR, FLOAT, z, 3)
#define InertialMeasurementValues_CALLBACK    NULL
#define InertialMeasurementValues_DEFAULT     NULL

#define TemperatureValue_FIELDLIST(X, a) \
	X(a, STATIC, SINGULAR, FLOAT, temperature, 1)
#define TemperatureValue_CALLBACK             NULL
#define TemperatureValue_DEFAULT              NULL

#define ActionMessage_FIELDLIST(X, a) \
	X(a, STATIC, SINGULAR, STRING, key, 1) \
	X(a, STATIC, SINGULAR, STRING, argument, 2)
#define ActionMessage_CALLBACK                NULL
#define ActionMessage_DEFAULT                 NULL

#define Action_FIELDLIST(X, a) \
	X(a, STATIC, SINGULAR, STRING, key, 1) \
	X(a, STATIC, SINGULAR, STRING, name, 2) \
	X(a, STATIC, SINGULAR, BOOL, is_available, 4) \
	X(a, STATIC, SINGULAR, INT32, source, 5)
#define Action_CALLBACK                       NULL
#define Action_DEFAULT                        NULL

#define Actions_FIELDLIST(X, a) \
	X(a, STATIC, REPEATED, MESSAGE, actions, 1)
#define Actions_actions_MSGTYPE               Action
#define Actions_CALLBACK                      NULL
#define Actions_DEFAULT                       NULL

extern const pb_msgdesc_t EyeCoordinates_msg;
extern const pb_msgdesc_t EyelidState_msg;
extern const pb_msgdesc_t BrightnessMessage_msg;
extern const pb_msgdesc_t InertialMeasurementValues_msg;
extern const pb_msgdesc_t TemperatureValue_msg;
extern const pb_msgdesc_t ActionMessage_msg;
extern const pb_msgdesc_t Action_msg;
extern const pb_msgdesc_t Actions_msg;

/* Defines for backwards compatibility with code written before nanopb-0.4.0 */
#define EyeCoordinates_fields &EyeCoordinates_msg
#define EyelidState_fields &EyelidState_msg
#define BrightnessMessage_fields &BrightnessMessage_msg
#define InertialMeasurementValues_fields &InertialMeasurementValues_msg
#define TemperatureValue_fields &TemperatureValue_msg
#define ActionMessage_fields &ActionMessage_msg
#define Action_fields &Action_msg
#define Actions_fields &Actions_msg

/* Maximum encoded size of messages (where known) */
#define EyeCoordinates_size                      10
#define EyelidState_size                         5
#define BrightnessMessage_size                   11
#define InertialMeasurementValues_size           15
#define TemperatureValue_size                    5
#define ActionMessage_size                       (2 + LUNIRIS_ACTION_KEY_MAX_SIZE + 2 + LUNIRIS_ACTION_ARG_MAX_SIZE)
#define Action_size                              (2 + LUNIRIS_ACTION_KEY_MAX_SIZE + 2 + LUNIRIS_ACTION_NAME_MAX_SIZE + 2 + 6)
#define Actions_size                             (4 + LUNIRIS_ACTIONS_MAX_COUNT * (Action_size + 2))
#define LUNIRIS_MESSAGES_PB_H_MAX_SIZE           Actions_size

#ifdef __cplusplus
}
#endif

#endif /* LUNIRIS_MESSAGES_PB_H */
