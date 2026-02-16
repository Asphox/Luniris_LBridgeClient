# Luniris LBridge Client

The official C client library for connecting to a [Luniris](https://luniris.net) system via the [LBridge](https://github.com/Asphox/LBridge) protocol.

## Overview

**Luniris LBridge Client** is a C11 static library that provides a simple, typed API to control and read data from a Luniris digital fursuit eye system. It connects to a running [Luniris API LBridge](https://github.com/Asphox/Luniris_LBridgeAPI) server instance and communicates over the [LBridge](https://github.com/Asphox/LBridge) protocol.

The library abstracts away all transport and serialization details — just create a client, connect, and call the functions you need.

## Requirements

- CMake 3.16+
- A C11-capable compiler (MSVC, GCC, Clang)
- Git (to fetch submodules)

## Integration

The library is designed to be integrated as a CMake subdirectory.

```bash
# Add as a git submodule
git submodule add https://github.com/Asphox/Luniris_LBridgeClient external/luniris_client
git submodule update --init --recursive
```

Then in your `CMakeLists.txt`:

```cmake
add_subdirectory(external/luniris_client)
target_link_libraries(your_target PRIVATE luniris_client)
```

The `luniris_client` target automatically pulls in LBridge and nanopb as transitive dependencies.

## Building Standalone

```bash
# Clone with submodules
git clone --recurse-submodules https://github.com/Asphox/Luniris_LBridgeClient.git
cd Luniris_LBridgeClient

# Configure
cmake -B build -DCMAKE_BUILD_TYPE=Release

# Build
cmake --build build --config Release
```

## Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `LUNIRIS_ENABLE_SECURE` | `ON` | Enable ChaCha20-Poly1305 encryption support |

```bash
# Build without encryption
cmake -B build -DLUNIRIS_ENABLE_SECURE=OFF
```

> **Note:** If the server has encryption enabled, the client must be built with `LUNIRIS_ENABLE_SECURE=ON` and use the same pre-shared key.

## Quick Start

```c
#include "luniris_client.h"

int main(void)
{
    // Create client with default settings
    luniris_client_t client = luniris_client_create(NULL);

    // Connect to the Luniris API LBridge server
    if (!luniris_client_connect_tcp(client, "127.0.0.1", 26412))
    {
        // Check error with luniris_client_get_last_error(client)
        luniris_client_destroy(client);
        return 1;
    }

    // Send eye position (x: -1 left / 1 right, y: -1 down / 1 up)
    EyeCoordinates coords = { .x = 0.5f, .y = -0.2f };
    luniris_send_eye_coordinates(client, &coords);

    luniris_client_destroy(client);
    return 0;
}
```

### With Encryption

```c
#include "luniris_client.h"

int main(void)
{
    // 256-bit pre-shared key (must match the server configuration)
    const uint8_t key[32] = { /* your key bytes */ };

    luniris_client_params_t params = {
        .encryption_key = key,
        .timeout_ms = 5000  // 5 second timeout (-1 for infinite)
    };

    luniris_client_t client = luniris_client_create(&params);
    luniris_client_connect_tcp(client, "127.0.0.1", 26412);

    // ...

    luniris_client_destroy(client);
    return 0;
}
```

## API Reference

### Lifecycle

```c
luniris_client_t luniris_client_create(const luniris_client_params_t* p_params);
void             luniris_client_destroy(luniris_client_t p_client);
bool             luniris_client_connect_tcp(luniris_client_t p_client, const char* p_host, uint16_t port);
bool             luniris_client_ping(luniris_client_t p_client);
luniris_error_t  luniris_client_get_last_error(luniris_client_t p_client);
```

### Eye Position

```c
// Send normalized eye coordinates (range: -1.0 to 1.0)
bool luniris_send_eye_coordinates(luniris_client_t p_client, const EyeCoordinates* p_coords);
bool luniris_get_eye_coordinates(luniris_client_t p_client, EyeCoordinates* p_out_coords);

// Send eyelid closure (0.0 = fully open, 1.0 = fully closed)
bool luniris_send_eyelid_state(luniris_client_t p_client, const EyelidState* p_state);
bool luniris_get_eyelid_state(luniris_client_t p_client, EyelidState* p_out_state);
```

### Display

```c
// Brightness level (0–100)
bool luniris_send_brightness_level(luniris_client_t p_client, const BrightnessMessage* p_brightness);
bool luniris_get_brightness_level(luniris_client_t p_client, BrightnessMessage* p_out_brightness);
```

### IMU Sensors

```c
// Gyroscope values (rad/s, 3 axes)
bool luniris_get_gyroscope_values(luniris_client_t p_client, InertialMeasurementValues* p_out_values);

// Accelerometer values (m/s², 3 axes)
bool luniris_get_accelerometer_values(luniris_client_t p_client, InertialMeasurementValues* p_out_values);

// Temperature (°C)
bool luniris_get_temperature_value(luniris_client_t p_client, TemperatureValue* p_out_value);
```

### LEDs

```c
// Control the left/right LEDs with color and priority.
// Set is_active = true to take control, false to release it.
bool luniris_send_led_settings(luniris_client_t p_client, const LedSettings* p_settings);
```

### Actions

```c
// Retrieve all registered actions available on the server
bool luniris_get_registered_actions(luniris_client_t p_client, Actions* p_out_actions);

// Poll the latest streamed action
bool luniris_get_action(luniris_client_t p_client, ActionMessage* p_out_action);

// Trigger an action by key
bool luniris_send_action(luniris_client_t p_client, const ActionMessage* p_action);
```

### Error Codes

| Code | Description |
|------|-------------|
| `LUNIRIS_ERROR_NONE` | No error |
| `LUNIRIS_ERROR_BAD_ALLOC` | Memory allocation failed |
| `LUNIRIS_ERROR_BAD_ARGUMENT` | Invalid argument |
| `LUNIRIS_ERROR_CONNECTION_TIMEOUT` | Connection timed out |
| `LUNIRIS_ERROR_CONNECTION_FAILED` | Connection refused |
| `LUNIRIS_ERROR_NOT_CONNECTED` | Operation attempted while not connected |
| `LUNIRIS_ERROR_CONNECTION_LOST` | Connection lost during operation |
| `LUNIRIS_ERROR_SEND_TIMEOUT` | Send timed out |
| `LUNIRIS_ERROR_SEND_FAILED` | Send failed |
| `LUNIRIS_ERROR_RECEIVE_TIMEOUT` | Receive timed out |
| `LUNIRIS_ERROR_HANDSHAKE_FAILED` | LBridge handshake failed |
| `LUNIRIS_ERROR_ENCODE_FAILED` | Protobuf encoding failed |
| `LUNIRIS_ERROR_DECODE_FAILED` | Protobuf decoding failed |

## Server

The server side is provided by the **Luniris API LBridge** feature, which runs on the Luniris system itself:

[**Luniris API LBridge**](https://github.com/Asphox/Luniris_LBridgeAPI)

## Documentation

For the full Luniris API reference, see the [Luniris API Documentation](https://luniris.net/docs/category/api).

## License

MIT License — See LICENSE file for details.
