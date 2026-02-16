#include "luniris_client.h"
#include "lbridge.h"
#include <pb_encode.h>
#include <pb_decode.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#else
#include <fcntl.h>
#include <unistd.h>
#endif

struct luniris_client
{
	lbridge_context_t lbridge_context;
	lbridge_client_t lbridge_client;
	luniris_error_t last_error;
};

#ifdef LBRIDGE_ENABLE_SECURE
static bool luniris_generate_nonce(lbridge_context_t context, uint8_t out_nonce[12])
{
	(void)context;
#ifdef _WIN32
	return BCryptGenRandom(NULL, out_nonce, 12, BCRYPT_USE_SYSTEM_PREFERRED_RNG) == 0;
#else
	int fd = open("/dev/urandom", O_RDONLY);
	if (fd < 0) return false;
	ssize_t ret = read(fd, out_nonce, 12);
	close(fd);
	return ret == 12;
#endif
}
#endif

static luniris_error_t translate_lbridge_error(enum lbridge_error_code err)
{
	switch (err)
	{
	case LBRIDGE_ERROR_NONE:               return LUNIRIS_ERROR_NONE;
	case LBRIDGE_ERROR_BAD_ALLOC:          return LUNIRIS_ERROR_BAD_ALLOC;
	case LBRIDGE_ERROR_BAD_ARGUMENT:       return LUNIRIS_ERROR_BAD_ARGUMENT;
	case LBRIDGE_ERROR_CONNECTION_TIMEOUT: return LUNIRIS_ERROR_CONNECTION_TIMEOUT;
	case LBRIDGE_ERROR_CONNECTION_FAILED:  return LUNIRIS_ERROR_CONNECTION_FAILED;
	case LBRIDGE_ERROR_NOT_CONNECTED:      return LUNIRIS_ERROR_NOT_CONNECTED;
	case LBRIDGE_ERROR_CONNECTION_LOST:    return LUNIRIS_ERROR_CONNECTION_LOST;
	case LBRIDGE_ERROR_SEND_TIMEOUT:       return LUNIRIS_ERROR_SEND_TIMEOUT;
	case LBRIDGE_ERROR_SEND_FAILED:        return LUNIRIS_ERROR_SEND_FAILED;
	case LBRIDGE_ERROR_RECEIVE_TIMEOUT:    return LUNIRIS_ERROR_RECEIVE_TIMEOUT;
	case LBRIDGE_ERROR_HANDSHAKE_FAILED:   return LUNIRIS_ERROR_HANDSHAKE_FAILED;
	default:                               return LUNIRIS_ERROR_UNKNOWN;
	}
}

luniris_client_t luniris_client_create(const luniris_client_params_t* p_params)
{
	luniris_client_t client = (luniris_client_t)malloc(sizeof(struct luniris_client));
	if (client == NULL)
	{
		return NULL;
	}

	client->last_error = LUNIRIS_ERROR_NONE;
	client->lbridge_context = NULL;
	client->lbridge_client = NULL;

	// Create LBridge context with internal defaults
	struct lbridge_context_params ctx_params = {0};
#ifdef LBRIDGE_ENABLE_SECURE
	ctx_params.fp_generate_nonce = luniris_generate_nonce;
#endif
	ctx_params.fp_malloc = malloc;
	ctx_params.fp_free = free;

	client->lbridge_context = lbridge_context_create(&ctx_params);
	if (client->lbridge_context == NULL)
	{
		free(client);
		return NULL;
	}

	// Create LBridge client with default frame/payload sizes
	client->lbridge_client = lbridge_client_create(client->lbridge_context, 1024, 65536);
	if (client->lbridge_client == NULL)
	{
		lbridge_context_destroy(client->lbridge_context);
		free(client);
		return NULL;
	}

	// Apply optional parameters
	if (p_params != NULL)
	{
#ifdef LBRIDGE_ENABLE_SECURE
		if (p_params->encryption_key != NULL)
		{
			lbridge_activate_encryption(client->lbridge_client, p_params->encryption_key);
		}
#endif
		if (p_params->timeout_ms != 0)
		{
			lbridge_set_timeout(client->lbridge_client, p_params->timeout_ms);
		}
	}

	return client;
}

void luniris_client_destroy(luniris_client_t p_client)
{
	if (p_client == NULL)
	{
		return;
	}

	if (p_client->lbridge_client != NULL)
	{
		lbridge_client_destroy(p_client->lbridge_client);
	}

	if (p_client->lbridge_context != NULL)
	{
		lbridge_context_destroy(p_client->lbridge_context);
	}

	free(p_client);
}

luniris_error_t luniris_client_get_last_error(luniris_client_t p_client)
{
	if (p_client == NULL)
	{
		return LUNIRIS_ERROR_BAD_ARGUMENT;
	}
	return p_client->last_error;
}

bool luniris_client_connect_tcp(luniris_client_t p_client, const char* p_host, uint16_t port)
{
	if (p_client == NULL || p_client->lbridge_client == NULL || p_host == NULL)
	{
		if (p_client != NULL) p_client->last_error = LUNIRIS_ERROR_BAD_ARGUMENT;
		return false;
	}

	bool result = lbridge_client_connect_tcp(p_client->lbridge_client, p_host, port);
	if (!result)
	{
		p_client->last_error = translate_lbridge_error(lbridge_get_last_error(p_client->lbridge_client));
	}
	else
	{
		p_client->last_error = LUNIRIS_ERROR_NONE;
	}
	return result;
}

bool luniris_client_ping(luniris_client_t p_client)
{
	if (p_client == NULL || p_client->lbridge_client == NULL)
	{
		if (p_client != NULL) p_client->last_error = LUNIRIS_ERROR_BAD_ARGUMENT;
		return false;
	}

	bool result = lbridge_client_ping(p_client->lbridge_client);
	if (!result)
	{
		p_client->last_error = translate_lbridge_error(lbridge_get_last_error(p_client->lbridge_client));
	}
	else
	{
		p_client->last_error = LUNIRIS_ERROR_NONE;
	}
	return result;
}

bool luniris_send_eye_coordinates(luniris_client_t p_client, const EyeCoordinates* p_coords)
{
	if (p_client == NULL || p_client->lbridge_client == NULL || p_coords == NULL)
	{
		if (p_client != NULL) p_client->last_error = LUNIRIS_ERROR_BAD_ARGUMENT;
		return false;
	}

	uint8_t buffer[EyeCoordinates_size];
	pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

	if (!pb_encode(&stream, EyeCoordinates_fields, p_coords))
	{
		p_client->last_error = LUNIRIS_ERROR_ENCODE_FAILED;
		return false;
	}

	uint32_t size = (uint32_t)stream.bytes_written;

	bool result = lbridge_client_call_rpc(
		p_client->lbridge_client,
		LUNIRIS_RPC_SEND_EYE_COORDINATES,
		buffer,
		&size,
		0
	);

	if (!result)
	{
		p_client->last_error = translate_lbridge_error(lbridge_get_last_error(p_client->lbridge_client));
	}
	else
	{
		p_client->last_error = LUNIRIS_ERROR_NONE;
	}
	return result;
}

bool luniris_send_eyelid_state(luniris_client_t p_client, const EyelidState* p_state)
{
	if (p_client == NULL || p_client->lbridge_client == NULL || p_state == NULL)
	{
		if (p_client != NULL) p_client->last_error = LUNIRIS_ERROR_BAD_ARGUMENT;
		return false;
	}

	uint8_t buffer[EyelidState_size];
	pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

	if (!pb_encode(&stream, EyelidState_fields, p_state))
	{
		p_client->last_error = LUNIRIS_ERROR_ENCODE_FAILED;
		return false;
	}

	uint32_t size = (uint32_t)stream.bytes_written;

	bool result = lbridge_client_call_rpc(
		p_client->lbridge_client,
		LUNIRIS_RPC_SEND_EYELID_STATE,
		buffer,
		&size,
		0
	);

	if (!result)
	{
		p_client->last_error = translate_lbridge_error(lbridge_get_last_error(p_client->lbridge_client));
	}
	else
	{
		p_client->last_error = LUNIRIS_ERROR_NONE;
	}
	return result;
}

bool luniris_get_eye_coordinates(luniris_client_t p_client, EyeCoordinates* p_out_coords)
{
	if (p_client == NULL || p_client->lbridge_client == NULL || p_out_coords == NULL)
	{
		if (p_client != NULL) p_client->last_error = LUNIRIS_ERROR_BAD_ARGUMENT;
		return false;
	}

	uint8_t buffer[EyeCoordinates_size];
	uint32_t size = 0;

	if (!lbridge_client_call_rpc(
		p_client->lbridge_client,
		LUNIRIS_RPC_GET_EYE_COORDINATES,
		buffer,
		&size,
		sizeof(buffer)))
	{
		p_client->last_error = translate_lbridge_error(lbridge_get_last_error(p_client->lbridge_client));
		return false;
	}

	pb_istream_t stream = pb_istream_from_buffer(buffer, size);
	if (!pb_decode(&stream, EyeCoordinates_fields, p_out_coords))
	{
		p_client->last_error = LUNIRIS_ERROR_DECODE_FAILED;
		return false;
	}

	p_client->last_error = LUNIRIS_ERROR_NONE;
	return true;
}

bool luniris_get_eyelid_state(luniris_client_t p_client, EyelidState* p_out_state)
{
	if (p_client == NULL || p_client->lbridge_client == NULL || p_out_state == NULL)
	{
		if (p_client != NULL) p_client->last_error = LUNIRIS_ERROR_BAD_ARGUMENT;
		return false;
	}

	uint8_t buffer[EyelidState_size];
	uint32_t size = 0;

	if (!lbridge_client_call_rpc(
		p_client->lbridge_client,
		LUNIRIS_RPC_GET_EYELID_STATE,
		buffer,
		&size,
		sizeof(buffer)))
	{
		p_client->last_error = translate_lbridge_error(lbridge_get_last_error(p_client->lbridge_client));
		return false;
	}

	pb_istream_t stream = pb_istream_from_buffer(buffer, size);
	if (!pb_decode(&stream, EyelidState_fields, p_out_state))
	{
		p_client->last_error = LUNIRIS_ERROR_DECODE_FAILED;
		return false;
	}

	p_client->last_error = LUNIRIS_ERROR_NONE;
	return true;
}

bool luniris_get_brightness_level(luniris_client_t p_client, BrightnessMessage* p_out_brightness)
{
	if (p_client == NULL || p_client->lbridge_client == NULL || p_out_brightness == NULL)
	{
		if (p_client != NULL) p_client->last_error = LUNIRIS_ERROR_BAD_ARGUMENT;
		return false;
	}

	uint8_t buffer[BrightnessMessage_size];
	uint32_t size = 0;

	if (!lbridge_client_call_rpc(
		p_client->lbridge_client,
		LUNIRIS_RPC_GET_BRIGHTNESS_LEVEL,
		buffer,
		&size,
		sizeof(buffer)))
	{
		p_client->last_error = translate_lbridge_error(lbridge_get_last_error(p_client->lbridge_client));
		return false;
	}

	pb_istream_t stream = pb_istream_from_buffer(buffer, size);
	if (!pb_decode(&stream, BrightnessMessage_fields, p_out_brightness))
	{
		p_client->last_error = LUNIRIS_ERROR_DECODE_FAILED;
		return false;
	}

	p_client->last_error = LUNIRIS_ERROR_NONE;
	return true;
}

bool luniris_send_brightness_level(luniris_client_t p_client, const BrightnessMessage* p_brightness)
{
	if (p_client == NULL || p_client->lbridge_client == NULL || p_brightness == NULL)
	{
		if (p_client != NULL) p_client->last_error = LUNIRIS_ERROR_BAD_ARGUMENT;
		return false;
	}

	uint8_t buffer[BrightnessMessage_size];
	pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

	if (!pb_encode(&stream, BrightnessMessage_fields, p_brightness))
	{
		p_client->last_error = LUNIRIS_ERROR_ENCODE_FAILED;
		return false;
	}

	uint32_t size = (uint32_t)stream.bytes_written;

	bool result = lbridge_client_call_rpc(
		p_client->lbridge_client,
		LUNIRIS_RPC_SEND_BRIGHTNESS_LEVEL,
		buffer,
		&size,
		0
	);

	if (!result)
	{
		p_client->last_error = translate_lbridge_error(lbridge_get_last_error(p_client->lbridge_client));
	}
	else
	{
		p_client->last_error = LUNIRIS_ERROR_NONE;
	}
	return result;
}

bool luniris_get_gyroscope_values(luniris_client_t p_client, InertialMeasurementValues* p_out_values)
{
	if (p_client == NULL || p_client->lbridge_client == NULL || p_out_values == NULL)
	{
		if (p_client != NULL) p_client->last_error = LUNIRIS_ERROR_BAD_ARGUMENT;
		return false;
	}

	uint8_t buffer[InertialMeasurementValues_size];
	uint32_t size = 0;

	if (!lbridge_client_call_rpc(
		p_client->lbridge_client,
		LUNIRIS_RPC_GET_GYROSCOPE_VALUES,
		buffer,
		&size,
		sizeof(buffer)))
	{
		p_client->last_error = translate_lbridge_error(lbridge_get_last_error(p_client->lbridge_client));
		return false;
	}

	pb_istream_t stream = pb_istream_from_buffer(buffer, size);
	if (!pb_decode(&stream, InertialMeasurementValues_fields, p_out_values))
	{
		p_client->last_error = LUNIRIS_ERROR_DECODE_FAILED;
		return false;
	}

	p_client->last_error = LUNIRIS_ERROR_NONE;
	return true;
}

bool luniris_get_accelerometer_values(luniris_client_t p_client, InertialMeasurementValues* p_out_values)
{
	if (p_client == NULL || p_client->lbridge_client == NULL || p_out_values == NULL)
	{
		if (p_client != NULL) p_client->last_error = LUNIRIS_ERROR_BAD_ARGUMENT;
		return false;
	}

	uint8_t buffer[InertialMeasurementValues_size];
	uint32_t size = 0;

	if (!lbridge_client_call_rpc(
		p_client->lbridge_client,
		LUNIRIS_RPC_GET_ACCELEROMETER_VALUES,
		buffer,
		&size,
		sizeof(buffer)))
	{
		p_client->last_error = translate_lbridge_error(lbridge_get_last_error(p_client->lbridge_client));
		return false;
	}

	pb_istream_t stream = pb_istream_from_buffer(buffer, size);
	if (!pb_decode(&stream, InertialMeasurementValues_fields, p_out_values))
	{
		p_client->last_error = LUNIRIS_ERROR_DECODE_FAILED;
		return false;
	}

	p_client->last_error = LUNIRIS_ERROR_NONE;
	return true;
}

bool luniris_get_temperature_value(luniris_client_t p_client, TemperatureValue* p_out_value)
{
	if (p_client == NULL || p_client->lbridge_client == NULL || p_out_value == NULL)
	{
		if (p_client != NULL) p_client->last_error = LUNIRIS_ERROR_BAD_ARGUMENT;
		return false;
	}

	uint8_t buffer[TemperatureValue_size];
	uint32_t size = 0;

	if (!lbridge_client_call_rpc(
		p_client->lbridge_client,
		LUNIRIS_RPC_GET_TEMPERATURE_VALUE,
		buffer,
		&size,
		sizeof(buffer)))
	{
		p_client->last_error = translate_lbridge_error(lbridge_get_last_error(p_client->lbridge_client));
		return false;
	}

	pb_istream_t stream = pb_istream_from_buffer(buffer, size);
	if (!pb_decode(&stream, TemperatureValue_fields, p_out_value))
	{
		p_client->last_error = LUNIRIS_ERROR_DECODE_FAILED;
		return false;
	}

	p_client->last_error = LUNIRIS_ERROR_NONE;
	return true;
}

bool luniris_send_led_settings(luniris_client_t p_client, const LedSettings* p_settings)
{
	if (p_client == NULL || p_client->lbridge_client == NULL || p_settings == NULL)
	{
		if (p_client != NULL) p_client->last_error = LUNIRIS_ERROR_BAD_ARGUMENT;
		return false;
	}

	uint8_t buffer[LedSettings_size];
	pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

	if (!pb_encode(&stream, LedSettings_fields, p_settings))
	{
		p_client->last_error = LUNIRIS_ERROR_ENCODE_FAILED;
		return false;
	}

	uint32_t size = (uint32_t)stream.bytes_written;

	bool result = lbridge_client_call_rpc(
		p_client->lbridge_client,
		LUNIRIS_RPC_SEND_LED_SETTINGS,
		buffer,
		&size,
		0
	);

	if (!result)
	{
		p_client->last_error = translate_lbridge_error(lbridge_get_last_error(p_client->lbridge_client));
	}
	else
	{
		p_client->last_error = LUNIRIS_ERROR_NONE;
	}
	return result;
}

bool luniris_get_registered_actions(luniris_client_t p_client, Actions* p_out_actions)
{
	if (p_client == NULL || p_client->lbridge_client == NULL || p_out_actions == NULL)
	{
		if (p_client != NULL) p_client->last_error = LUNIRIS_ERROR_BAD_ARGUMENT;
		return false;
	}

	uint8_t* buffer = (uint8_t*)malloc(Actions_size);
	if (buffer == NULL)
	{
		p_client->last_error = LUNIRIS_ERROR_BAD_ALLOC;
		return false;
	}

	uint32_t size = 0;

	if (!lbridge_client_call_rpc(
		p_client->lbridge_client,
		LUNIRIS_RPC_GET_REGISTERED_ACTIONS,
		buffer,
		&size,
		Actions_size))
	{
		p_client->last_error = translate_lbridge_error(lbridge_get_last_error(p_client->lbridge_client));
		free(buffer);
		return false;
	}

	pb_istream_t stream = pb_istream_from_buffer(buffer, size);
	if (!pb_decode(&stream, Actions_fields, p_out_actions))
	{
		p_client->last_error = LUNIRIS_ERROR_DECODE_FAILED;
		free(buffer);
		return false;
	}

	free(buffer);
	p_client->last_error = LUNIRIS_ERROR_NONE;
	return true;
}

bool luniris_get_action(luniris_client_t p_client, ActionMessage* p_out_action)
{
	if (p_client == NULL || p_client->lbridge_client == NULL || p_out_action == NULL)
	{
		if (p_client != NULL) p_client->last_error = LUNIRIS_ERROR_BAD_ARGUMENT;
		return false;
	}

	uint8_t buffer[ActionMessage_size];
	uint32_t size = 0;

	if (!lbridge_client_call_rpc(
		p_client->lbridge_client,
		LUNIRIS_RPC_GET_ACTION,
		buffer,
		&size,
		sizeof(buffer)))
	{
		p_client->last_error = translate_lbridge_error(lbridge_get_last_error(p_client->lbridge_client));
		return false;
	}

	pb_istream_t stream = pb_istream_from_buffer(buffer, size);
	if (!pb_decode(&stream, ActionMessage_fields, p_out_action))
	{
		p_client->last_error = LUNIRIS_ERROR_DECODE_FAILED;
		return false;
	}

	p_client->last_error = LUNIRIS_ERROR_NONE;
	return true;
}

bool luniris_send_action(luniris_client_t p_client, const ActionMessage* p_action)
{
	if (p_client == NULL || p_client->lbridge_client == NULL || p_action == NULL)
	{
		if (p_client != NULL) p_client->last_error = LUNIRIS_ERROR_BAD_ARGUMENT;
		return false;
	}

	uint8_t buffer[ActionMessage_size];
	pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

	if (!pb_encode(&stream, ActionMessage_fields, p_action))
	{
		p_client->last_error = LUNIRIS_ERROR_ENCODE_FAILED;
		return false;
	}

	uint32_t size = (uint32_t)stream.bytes_written;

	bool result = lbridge_client_call_rpc(
		p_client->lbridge_client,
		LUNIRIS_RPC_SEND_ACTION,
		buffer,
		&size,
		0
	);

	if (!result)
	{
		p_client->last_error = translate_lbridge_error(lbridge_get_last_error(p_client->lbridge_client));
	}
	else
	{
		p_client->last_error = LUNIRIS_ERROR_NONE;
	}
	return result;
}
