# ESP32-CAM web server camera

## Project goals

- ✅ provide custom environment for development on ESP32-CAM with Docker and CLion
- ✅ implement ESP32 camera stream over websocket
- ✅ implement ESP32 control stream over websocket
- ✅ provide simple js client served by ESP32
- ✅ provide sample of python client with opencv

## Running project

Use environment defined in [`env`](./env) directory. It contains script for starting and stopping
docker environment with ESP IDF with camera installed.

You can easily connect to this environment from CLion. Just use toolchains configuration
and select a remote toolchains option to connect to image over ssh.

By default, script shares with docker image `/dev/ttyUSB0` device which is usually an ESP32
module connected to USB port. You should adjust this configuration to your needs and rerun
[`start.sh`](./env/start.sh) script.

## Flashing project

To flash project to ESP32, open terminal on remote container with action
`Open Remote Host Terminal` (you can find it with `Ctrl+Shift+A`) and use

```bash
idf.py flash monitor
```

After flashing compiled program to ESP32, serial monitor will be opened in console,
so you can easily debug log data in terminal. You can stop it with `Ctrl+]`

## Build configuration

You can configure your build with

```bash
idf.py menuconfig
```

executed in remote container. It allows to change compiler options and log level
as well as manage all modules from ESP IDF.

