# Scripting language API

Euphonium contains a berry-lang scripting language that can be used to tweak the system for your needs. The following page documents the internal API used in all scripts. Feel free to check `euphonium/scripts` to see how this is used internally.

## `globals`

Global utilities

### Commands

| Command    | Signature                                                                                 | Supported platforms |
|:-----------|-------------------------------------------------------------------------------------------|---------------------|
| `sleep_ms` | `(milliseconds: int) -> void`. <br/>Pauses execution for given amount of milliseconds. | All                 |

## `core`

Manages euphonium's core functionality, mostly shared utils. 

**Implemented by `Core.cpp`**

### Commands

| Command              | Signature                                                                                                              | Supported platforms |
|:---------------------|------------------------------------------------------------------------------------------------------------------------|---------------------|
| `core.start_plugin`  | `(pluginName: string, pluginConfig: map) -> void`<br/>Starts given plugin's audio thread with following configuration. | All                 |
| `core.platform`      | `() -> string`.<br/>Returns platform on which euphonium is currently running. Result being either `esp32` or `desktop` | All                 |
| `core.version`       | `() -> string`.<br/>Returns current version of the system. Example result: `0.0.14`                                    | All                 |

## `FormContext`

Class used in plugins that handles UI creation and interaction in the web-ui.

**Implemented by `form_ctx.be`**

### Class methods

All of the following methods are available on the `FormContext` class instance.

| Command          | Signature                                                                                                                                          | Supported platforms |
|:-----------------|----------------------------------------------------------------------------------------------------------------------------------------------------|---------------------|
| `text_field`     | `(key: string, config: FieldConfig) -> void`<br/>Adds a text field to the configuration. See `FieldConfig` below for parameters.                   | All                 |
| `select_field`   | `(key: string, config: FieldConfig) -> void`<br/>Adds a option select / picker field to the configuration. See `FieldConfig` below for parameters. | All                 |
| `checkbox_field` | `(key: string, config: FieldConfig) -> void`<br/>Adds a checkbox to the configuration. See `FieldConfig` below for parameters.                     | All                 |
| `create_group`   | `(key: string, config: [key: string, label: string]) -> void`<br/>Adds a new config group to the configuration.                                    | All                 |

### Interface `FieldConfig`

| Field     | Signature                                                                                    | Field type     |
|:----------|----------------------------------------------------------------------------------------------|----------------|
| `label`   | `string`<br/>Configuration field's label visible over the control in the interface.          | All            |
| `hint`    | `string`<br/>Used as a hint in the text field.                                               | `text_field`   |
| `default` | `string`<br/>Field's default value.                                                          | All            |
| `values`  | `list`<br/>All of select field's available values.                                           | `select_field` |
| `group`   | `string`<br/>Group that a given field belongs to, previously registered with `create_group`  | All            |

## `http`
Allows for registering endpoints on the internal HTTP server.

**Implemented by `http.be` and `HTTPModule.cpp`**

### Commands

| Command           | Signature                                                                                                                                                                                                                                | Supported platforms |
|:------------------|------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------|
| `http.handle`     | `(method: string, path: string, response_handler: [(request) -> void]) -> void`<br/>Register a new HTTP endpoint under given `path`. Response handler is a method that takes `HTTPRequest` as a parameter. See examples below for usage. | All                 |
| `http.emit_event` | `(type: string, body: map) -> void`.<br/>Broadcasts a server-side event to all connected devices. `body` will be serialized into json string.                                                                                            | All                 |

### Object `HTTPRequest`

| Command        | Signature                                                                                                                                                                                       | Supported platforms |
|:---------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------|
| `write_json`   | `(body: map, status: int) -> void`.<br/>Writes a response json to given connection. Body is passed as a map of data to be serialized into json. Status is the HTTP status code of the response. | All                 |
| `write_text`   | `(body: string, status: int) -> void`.<br/>Writes the response as `text/plain`.                                                                                                                 | All                 |
| `json_body`    | `() -> map`<br/>Parses given request's body string into json and returns it as map                                                                                                              | All                 |
| `query_params` | `() -> map`<br/>Returns map of parsed query parameters of the given request.                                                                                                                    | All                 |
| `url_params`   | `() -> map`<br/>Returns map of parsed url parameters of the given request.                                                                                                                      | All                 |


### Example

!!! example "HTTP server usage"

    Handle simple GET and return "Hello, world!"

    ```python
    http.handle('GET', '/hello_world', def (request)
        request.write_text("Hello world!", 200)
    end)
    ```
    Handle POST with json body, return a json response

    ```python
    http.handle('POST', '/create_cat', def (request)
        if request.json_body() == nil
            http.write_text("No body", request['connection'], 400)
        else
           # Parse json body
           var parsed_body = request.json_body()

           # Create response
           var response = { 'name': parsed_body['name'], 'age': 3 }
           request.write_json(response, 200)
        end
    end)
    ```

## `playback`

Manages playback state of the system.

### Commands

| Command                  | Signature                                                                                                                                                                        | Supported platforms |
|:-------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------|
| `playback.set_eq`        | `(low: int, mid: int, high: int) -> void`.<br/>If `EqualizerProcessor` is enabled, this changes the eq's settings. 1 point on the scale means 3 db. Defaults to 0, 0, 0 (no eq). | All                 |
| `playback.set_pause`     | `(paused: boolean) -> void`.<br/>Pauses the playback state. This also triggers a pause event.                                                                                    | All                 |
| `playback.empty_buffers` | `() -> void`.<br/>Empties internal audio buffers of the system. Call this during playback changes / stop pause.                                                                  | All                 |
| `playback.soft_volume`   | `(volume: int) -> void`<br/>Changes the system's software volume. Volume is between `0` and `100`.                                                                               | All                 |

## `euphonium`
Global euphonium instance object. Handles main events, and keeps a state of plugin registry.

### Commands

| Command                     | Signature                                                                                                                                     | Supported platforms |
|:----------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------|---------------------|
| `euphonium.register_plugin` | `(plugin: Plugin) -> void`<br/>Registers a new euphonium plugin. `plugin` is a instance of plugin to register in the system.                  | All                 |

## `input`
*ESP32 specific*

Allows registration of callbacks for input events. Useful for adding buttons, encoders and such.
### Commands

| Command                 | Signature                                                                                                                                                                                                                           | Supported platforms |
|:------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------|

## `hooks`

Hooks allow to run different instructions during certain boot stages. Used for example to pull up an IO during boot.

### Commands

| Command             | Signature                                                                                                                | Supported platforms |
|:--------------------|--------------------------------------------------------------------------------------------------------------------------|---------------------|
| `hooks.add_handler` | `(bootstage: int, handler: [() -> void]) -> void`<br/>Register a new hook. Different `bootstage` values described below. | All                 |                                                                                            | esp32               |

### enum `hooks.BOOTSTAGE`

| Command             | Description                                                               | Supported platforms |
|:--------------------|---------------------------------------------------------------------------|---------------------|
| `hooks.ON_INIT`     | Called earliest during boot, after scripting VM init.                     | All                 |
| `hooks.POST_SYSTEM` | Called after all core logic has been initialized, before plugins startup. | All                 |
| `hooks.POST_PLUGIN` | Called after all plugins have been initialized                            | All                 |
| `hooks.AP_INIT`     | Called after AP network has been initialized.                             | esp32               |
| `hooks.WIFI_INIT`   | Called after WiFi has been initialized.                                   | esp32               |


### Example

!!! example "Sample hook that runs after boot"

    Define I2S configuration, output 256 x MCLK clock on GPIO0.
    ```python
    hooks.add_handler(hooks.ON_INIT, def ()
        print("On boot called!")
    end)
    ```

## `i2s`

Controls I2S bus. Mainly used for DAC support.

**Implemented by `I2SDriver.cpp`**

### Commands

| Command              | Signature                                                                                                                       | Supported platforms |
|:---------------------|---------------------------------------------------------------------------------------------------------------------------------|---------------------|
| `i2s.install`        | `(configuration: I2SConfig) -> void`<br/>Installs I2S driver. See below for description of I2SConfig structure.                 | esp32               |
| `i2s.delete`         | `() -> void`.<br/>Uninstalls the current I2S driver.                                                                            | esp32               |
| `i2s.set_expand`     | `(src: int, dst: int) -> void`.<br/>Enables expand from `src` bits to `dst` bits in driver write. Useful for 32bit DAC support. | esp32               |
| `i2s.disable_expand` | `() -> void`.<br/>Disables bits expand.                                                                                         | esp32               |

### Object `I2SConfig`

| Field             | Description                                                                                                                                                 |
|:------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `sample_rate`     | `int`<br/>Defines sample rate for the incoming data signal. _WARNING - Sample rate is only initial, will be further changed in case od dynamic sample rate_ |
| `bits_per_sample` | `int`<br/>Bits per sample for incoming data.                                                                                                                |
| `comm_format`     | one of `I2S_CHANNEL_FMT_RIGHT_LEFT`, `I2S_CHANNEL_FMT_ALL_RIGHT`, `I2S_CHANNEL_FMT_ALL_LEFT` or `I2S_CHANNEL_FMT_ONLY_RIGHT`                                |
| `channel_format`  | one of `I2S_COMM_FORMAT_I2S`, `I2S_COMM_FORMAT_MSB`, `I2S_COMM_FORMAT_PCM_SHORT` or `I2S_COMM_FORMAT_PCM_LONG`                                              |
| `mclk`            | `int` <br/>if defined and larger than 0, outputs given `mclk` on GPIO0.                                                                                     |

### Example

!!! example "Sample driver configuration"

    Define I2S configuration, output 256 x MCLK clock on GPIO0.
    ```python
    var config = I2SConfig()
    config.sample_rate = 44100
    config.bits_per_sample = 16
    config.comm_format = I2S_CHANNEL_FMT_RIGHT_LEFT
    config.channel_format = I2S_COMM_FORMAT_I2S
    config.mclk = 256 * 44100

    i2s.install(config)
    ```
## `i2c`
Controls I2C bus on supported platforms. Mainly used in different drivers.

**Implemented by `I2CDriver.cpp`**

### Commands

| Command           | Signature                                                                                                                                     | Supported platforms |
|:------------------|-----------------------------------------------------------------------------------------------------------------------------------------------|---------------------|
| `i2c.install`     | `(sda: int, scl: int) -> void`<br/>Installs I2C driver under given pins.                                                                      | esp32               |
| `i2c.detect`      | `(addr:int) -> bool`.<br/>Tries to detect device under given addr. Returns true if device found.                                              | esp32               |
| `i2c.read_bytes`  | `(addr:int, reg:int, size:int) -> int or nil`.<br/>Read a value of 1..4 bytes from address addr and register reg. Returns nil if no response. | esp32               |
| `i2c.write_bytes` | `(addr:int, reg:int, val:bytes) -> nil`<br/>Writes the val bytes sequence as bytes() to address addr register reg.                            | esp32               |
| `i2c.read`        | `(addr:int, reg:int, size:int) -> int or nil`.<br/>Reads a singular bytes from a given register.                                              | esp32               |
| `i2c.write`       | `(addr:int, reg:int, val:int) -> nil`<br/>Writes a singular byte to a given register.                                                         | esp32               |
| `i2c.write_raw`   | `(addr:int, val:bytes) -> nil`<br/>Write a raw sequence of bytes to the given device.                                                         | esp32               |
| `i2c.read_raw`    | `(addr:int, val:bytes, size: int) -> int or nil`<br/>Writes the val sequence of bytes on the i2c line, and then reads `size` bytes.           | esp32               |

### Example
!!! example "Write few bytes to I2C device"

    Configure I2C on 21 and 23 pins, then perform two writes.
    ```python
    i2c.install(21, 23)

    var deviceAddr = 0x10

    # Write 0x01 to register 0x00
    i2c.write(deviceAddr, 0x00, 0x01)

    # Write byte sequence to register 0x01
    i2c.write_bytes(deviceAddr, 0x01, bytes('1a01'))
    ```

## `gpio`
Controls GPIO pins on supported platforms. Mainly used in different drivers.

**Implemented by `GPIODriver.cpp`**

### Commands

| Command                | Signature                                                                                                                                                                                                                                                       | Supported platforms |
|:-----------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|---------------------|
| `gpio.digital_write`   | `(gpio: int, state: int) -> void`<br/>Sets GPIO to LOW/HIGH. Needs physical pin number                                                                                                                                                                          | esp32               |
| `gpio.digital_read`    | `(gpio: int) -> int`<br/>Returns digital state of given physical GPIO. Either `gpio.LOW` or `gpio.HIGH`                                                                                                                                                         | esp32               |
| `gpio.pin_mode`        | `(gpio: int, mode: int) -> int`<br/>Changes the GPIO mode. Only use if if you know what you're doing, by default Euphonium handles GPIO mode itself. Mode can have the following values: gpio.INPUT, gpio.OUTPUT, gpio.PULLUP, gpio.INPUT_PULLUP, gpio.PULLDOWN | esp32               |
| `gpio.analog_read`     | `(gpio: int) -> real`.<br/>Returns the voltage on a given pin in mV. **Only used with DAC pins.**                                                                                                                                                               | esp32               |
| `gpio.register_button` | `(gpio: int, event_type: gpio.EVENT_TYPE,  handler: [() -> void], config: map([high_state: bool])) -> void`<br/>Registers a new handler called after interaction with a given button on provided `gpio`. Supports press, double press, and long press.          | esp32               |                                                                                            | esp32               |

### enum `input.EVENT_TYPE`

| Command             | Description                         | Supported platforms  |
|:--------------------|-------------------------------------|----------------------|
| `gpio.PRESS`        | Called on single press of a button. | esp32                |
| `gpio.DOUBLE_PRESS` | Called on double press of a button. | esp32                |
| `gpio.LONG_PRESS`   | Called on long press of a button.   | esp32                |

### Example

!!! example "GPIO Driver usage"

    Sets GPIO 21 as output, writes its state to HIGH.
    ```python
    gpio.pin_mode(21, gpio.OUTPUT)
    gpio.digital_write(21, gpio.HIGH)
    ```

### Example

!!! example "Example button that changes volume when pressed"

    Register a button on gpio 5, and call a function from `playback` when pressed.
    ```python
    gpio.register_button(5, input.PRESS, def ()
        print("Volume up called!")
        playback.set_volume(playback.volume + 5)
    end, { 'high_state': true })
    ```

## `led_strip`
Allows for control of addressable LEDs like the WS28xx and SK6812. Underneath it uses esp32's RMT driver, to drive up to 8 separate strip instances.

**Implemented by `LEDDriver.cpp`**

### Class `LEDStrip`

| Field                | Signature                                                                                                                                                                                                                                                                                                            | Supported platforms  |
|:---------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|----------------------|
| `init / constructor` |`(type: LED_TYPE, pin: int, len: int, channel: RMT_CHANNEL, brightness: int?)`<br/>`LEDStrip` constructor. Allows control of a singular strip, with a driver `type`, connected under a GPIO `pin`. `channel` is the RMT channel to use. If `brightness` is provided, the entire LED chain will be dimmed accordingly. | esp32                |
| `show`               |`() -> void`<br/> Called on existing instance. Will update the LED strip with previously assigned color values                                                                                                                                                                                                        | esp32                |
| `set_item`           | `(index, item: [r: number, g: number, b: number]) -> void`<br/>This implements the API for setting color of individual LEDs via a standard color assign. See example below for usage. `r`, `g` and `b` range from 0 to 255.                                                                                          | esp32                |

### enum `led_strip.LED_TYPE`

| Value               | Description                         |
|:--------------------|-------------------------------------|
| `LED_WS2812`        | Indicates WS2812 LED type.          |
| `LED_WS2812B`       | Indicates WS2812B LED type.         | 
| `LED_SK6812`        | Indicates SK6812 LED type.          |
| `LED_WS2813`        | Indicates WS2813 LED type.          |

### Example

!!! example "LED Driver usage"

    Registers a new LED strip under pin 21, consisting of 12 WS2812 leds, at lower brightness. Then turns the first LED red, the second one green.
    ```python
    volume_strip = LEDStrip(LED_WS2812, 21, 12, RMT_CHANNEL_0, 150)

    # change the first LED to red and second one to green.
    volume_strip[0] = [255, 0, 0] # red in RGB format
    volume_strip[1] = [0, 255, 0] # green in RGB format

    # display the changes on the strip
    volume_strip.show()
    ```


## `wifi`

Controls internal state of the platform's WiFi. Used internally by `wifi.be`.

**Implemented by `WiFiDriver.cpp`**

### Commands

| Command                | Signature                                                                                                                                         | Supported platforms |
|:-----------------------|---------------------------------------------------------------------------------------------------------------------------------------------------|---------------------|
| `wifi.init_stack`      | `() -> void`<br/>Initializes the WiFi stack                                                                                                       | esp32               |
| `wifi.connect`         | `(ssid: string, password: string, fromAP: bool) -> void`<br/>Attempts WiFi connection. `fromAP` should be set according to the current WiFi mode. | esp32               |
| `wifi.start_ap`        | `(ssid: string, password: string) -> void`<br/>Starts an access point with given credentials.                                                     | esp32               |
| `wifi.start_scan`      | `() -> boid`.<br/>Starts scanning of WiFi networks.                                                                                               | esp32               |