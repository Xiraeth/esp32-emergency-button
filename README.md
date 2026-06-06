# ESP32 Emergency Button

ESP-IDF firmware for an ESP32-S3-DevKitC-1 emergency button. The board connects to Wi-Fi, waits for a button press on GPIO4, and sends an HTTP POST request to a local server.

Current request payload:

```json
{ "event": "button_pressed" }
```

## Hardware

- ESP32-S3-DevKitC-1 with ESP32-S3-WROOM-1
- Pull-up button module connected to GPIO4
- Local server reachable from the same Wi-Fi/LAN network

## Wiring

The firmware expects an active-low button:

- Button signal: GPIO4
- Button VCC: 3.3 V
- Button GND: GND

With this setup, GPIO4 reads high when idle and low when pressed. The firmware sends one POST per press and waits for the button to be released before accepting another press.

## Configuration

Set the HTTP endpoint in [main/esp_http_client_example.c](main/esp_http_client_example.c):

```c
#define SERVER_URL "http://YOUR_SERVER_IP:3000/emergency"
```

Use your computer/server LAN IP, not the ESP32 IP. For example, if the server is running on a computer at `192.168.1.100`, use:

```c
#define SERVER_URL "http://192.168.1.100:3000/emergency"
```

Configure Wi-Fi credentials with ESP-IDF menuconfig:

```bash
idf.py menuconfig
```

Then set:

```text
Example Connection Configuration -> WiFi SSID
Example Connection Configuration -> WiFi Password
```

## Build And Flash

Load the ESP-IDF environment first. The exact path depends on your local ESP-IDF installation:

```bash
source ~/.espressif/v6.0.1/esp-idf/export.sh
```

If your ESP-IDF install is somewhere else, source that `export.sh` instead.

Set the target:

```bash
idf.py set-target esp32s3
```

Build:

```bash
idf.py build
```

Flash and monitor:

```bash
idf.py -p [PORT] flash monitor
```

Use the serial device that matches your board.

Exit the monitor with `Ctrl+]`.

## Python Test Dependencies

The pytest file uses Espressif's embedded test packages. Install them into the Python environment used by your editor or terminal:

```bash
python3 -m pip install -r requirements-test.txt
```

If VS Code still reports `Import "pytest_embedded" could not be resolved`, make sure VS Code is using the same Python interpreter where you installed these packages.

Run the test with the ESP-IDF and ESP serial services enabled:

```bash
pytest pytest_esp_http_client.py --embedded-services esp,idf --target esp32s3 --port /dev/ttyUSB0
```

Use the serial port that matches your board.

## Expected Logs

After boot, the firmware connects to Wi-Fi and waits for GPIO4:

```text
Connected to network
Waiting for button press on GPIO4
```

When the button is pressed:

```text
Button pressed
POST status = 200, content_length = 11
Button released
```

## Local Server

The firmware only sends the HTTP request. A separate server must be running at `SERVER_URL`.

For this project, the intended server is the [Discord bot](https://github.com/Xiraeth/discord-narrator) emergency HTTP listener running on port `3000` in development mode. Any server that accepts `POST /emergency` with JSON will work.
