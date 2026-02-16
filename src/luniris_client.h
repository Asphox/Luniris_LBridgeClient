#ifndef LUNIRIS_CLIENT_H
#define LUNIRIS_CLIENT_H

#include "luniris_messages.pb.h"
#include "luniris_rpc_ids.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error codes returned by Luniris client functions.
 */
typedef enum luniris_error
{
	LUNIRIS_ERROR_NONE = 0,           /**< No error occurred. */
	LUNIRIS_ERROR_BAD_ALLOC,          /**< Memory allocation failed. */
	LUNIRIS_ERROR_BAD_ARGUMENT,       /**< Invalid argument passed to a function. */
	LUNIRIS_ERROR_CONNECTION_TIMEOUT, /**< Connection attempt timed out. */
	LUNIRIS_ERROR_CONNECTION_FAILED,  /**< Connection was refused by the remote host. */
	LUNIRIS_ERROR_NOT_CONNECTED,      /**< Operation attempted on a disconnected client. */
	LUNIRIS_ERROR_CONNECTION_LOST,    /**< Connection was lost during operation. */
	LUNIRIS_ERROR_SEND_TIMEOUT,       /**< Send operation timed out. */
	LUNIRIS_ERROR_SEND_FAILED,        /**< Send operation failed. */
	LUNIRIS_ERROR_RECEIVE_TIMEOUT,    /**< Receive operation timed out. */
	LUNIRIS_ERROR_HANDSHAKE_FAILED,   /**< Protocol handshake with remote peer failed. */
	LUNIRIS_ERROR_ENCODE_FAILED,      /**< Failed to encode protobuf message. */
	LUNIRIS_ERROR_DECODE_FAILED,      /**< Failed to decode protobuf message. */
	LUNIRIS_ERROR_UNKNOWN = 255,      /**< Unknown or unspecified error. */
} luniris_error_t;

/**
 * @brief Opaque handle to a Luniris client.
 */
typedef struct luniris_client* luniris_client_t;

/**
 * @brief Optional parameters for creating a Luniris client.
 *
 * All fields are optional. If NULL is passed to luniris_client_create(),
 * default values will be used.
 */
typedef struct luniris_client_params
{
	/**
	 * @brief Optional 256-bit encryption key for secure communication.
	 *
	 * If non-NULL, ChaCha20-Poly1305 encryption will be activated.
	 * Both client and server must use the same key.
	 */
	const uint8_t* encryption_key;

	/**
	 * @brief Timeout in milliseconds for operations.
	 *
	 * Use -1 for infinite timeout (default), 0 for non-blocking.
	 */
	int32_t timeout_ms;
} luniris_client_params_t;

/**
 * @brief Creates a new Luniris client instance.
 *
 * @param p_params Optional parameters. Pass NULL for defaults.
 * @return A new client handle, or NULL on failure.
 */
luniris_client_t luniris_client_create(const luniris_client_params_t* p_params);

/**
 * @brief Destroys a Luniris client and frees all resources.
 *
 * @param p_client The client to destroy.
 */
void luniris_client_destroy(luniris_client_t p_client);

/**
 * @brief Gets the last error code for the client.
 *
 * @param p_client The client to query.
 * @return The last error code, or LUNIRIS_ERROR_NONE if no error occurred.
 */
luniris_error_t luniris_client_get_last_error(luniris_client_t p_client);

/**
 * @brief Connects to a Luniris server via TCP.
 *
 * @param p_client The client instance.
 * @param p_host   Server hostname or IP address.
 * @param port     Server port.
 * @return true on success, false on failure.
 */
bool luniris_client_connect_tcp(luniris_client_t p_client, const char* p_host, uint16_t port);

/**
 * @brief Sends a ping to keep the connection alive.
 *
 * @param p_client The client instance.
 * @return true on success, false on failure.
 */
bool luniris_client_ping(luniris_client_t p_client);

/**
 * @brief Sends eye coordinates to the server.
 *
 * @param p_client The Luniris client instance.
 * @param p_coords Pointer to the eye coordinates to send.
 * @return true on success, false on failure.
 */
bool luniris_send_eye_coordinates(luniris_client_t p_client, const EyeCoordinates* p_coords);

/**
 * @brief Sends eyelid state to the server.
 *
 * @param p_client The Luniris client instance.
 * @param p_state  Pointer to the eyelid state to send.
 * @return true on success, false on failure.
 */
bool luniris_send_eyelid_state(luniris_client_t p_client, const EyelidState* p_state);

/**
 * @brief Gets the current eye coordinates from the server.
 *
 * @param p_client    The Luniris client instance.
 * @param p_out_coords Pointer to store the received coordinates.
 * @return true on success, false on failure.
 */
bool luniris_get_eye_coordinates(luniris_client_t p_client, EyeCoordinates* p_out_coords);

/**
 * @brief Gets the current eyelid state from the server.
 *
 * @param p_client   The Luniris client instance.
 * @param p_out_state Pointer to store the received state.
 * @return true on success, false on failure.
 */
bool luniris_get_eyelid_state(luniris_client_t p_client, EyelidState* p_out_state);

/**
 * @brief Gets the current brightness level from the server.
 *
 * @param p_client      The Luniris client instance.
 * @param p_out_brightness Pointer to store the received brightness.
 * @return true on success, false on failure.
 */
bool luniris_get_brightness_level(luniris_client_t p_client, BrightnessMessage* p_out_brightness);

/**
 * @brief Sends a brightness level to the server.
 *
 * @param p_client    The Luniris client instance.
 * @param p_brightness Pointer to the brightness level to send.
 * @return true on success, false on failure.
 */
bool luniris_send_brightness_level(luniris_client_t p_client, const BrightnessMessage* p_brightness);

/**
 * @brief Gets the current gyroscope values from the server.
 *
 * @param p_client   The Luniris client instance.
 * @param p_out_values Pointer to store the received values (rad/s).
 * @return true on success, false on failure.
 */
bool luniris_get_gyroscope_values(luniris_client_t p_client, InertialMeasurementValues* p_out_values);

/**
 * @brief Gets the current accelerometer values from the server.
 *
 * @param p_client   The Luniris client instance.
 * @param p_out_values Pointer to store the received values (m/s^2).
 * @return true on success, false on failure.
 */
bool luniris_get_accelerometer_values(luniris_client_t p_client, InertialMeasurementValues* p_out_values);

/**
 * @brief Gets the current temperature value from the server.
 *
 * @param p_client   The Luniris client instance.
 * @param p_out_value Pointer to store the received temperature (Celsius).
 * @return true on success, false on failure.
 */
bool luniris_get_temperature_value(luniris_client_t p_client, TemperatureValue* p_out_value);

/**
 * @brief Sends LED settings to the server.
 *
 * Controls the LED colors and priority. Set is_active to true to take control,
 * or false to release control.
 *
 * @param p_client  The Luniris client instance.
 * @param p_settings Pointer to the LED settings to send.
 * @return true on success, false on failure.
 */
bool luniris_send_led_settings(luniris_client_t p_client, const LedSettings* p_settings);

/**
 * @brief Gets all registered actions from the server.
 *
 * @param p_client     The Luniris client instance.
 * @param p_out_actions Pointer to store the received actions.
 * @return true on success, false on failure.
 */
bool luniris_get_registered_actions(luniris_client_t p_client, Actions* p_out_actions);

/**
 * @brief Gets the last streamed action from the server (polling).
 *
 * @param p_client    The Luniris client instance.
 * @param p_out_action Pointer to store the received action.
 * @return true on success, false on failure.
 */
bool luniris_get_action(luniris_client_t p_client, ActionMessage* p_out_action);

/**
 * @brief Sends an action to the server.
 *
 * @param p_client The Luniris client instance.
 * @param p_action Pointer to the action to send.
 * @return true on success, false on failure.
 */
bool luniris_send_action(luniris_client_t p_client, const ActionMessage* p_action);

#ifdef __cplusplus
}
#endif

#endif // LUNIRIS_CLIENT_H
