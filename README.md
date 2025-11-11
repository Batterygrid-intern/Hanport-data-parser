# rpi-hanport
This is a project built on the raspberry pi 4 to read data from a swedish electricity meter using the hanport interface and to
transmitt the data collected using modbus and mqtt protocol.



# Configure (UART) serial on the rpi4 (RX pin)

# Installations
* automake
* autoconf
* libtool
* openssl
* build-essential
* libssl-dev
  
## build tools 
* cmake version 3.16
* make
* build-essential

## Compilers
* Fully compliant c++17 compiler.
* C v11 compiler

## Install external libraries
### Libmodbus
* [Libmodbus](https://github.com/stephane/libmodbus) follow the installation instructions.

### PahoMqttCpp
* [PahoMqttCpp](https://github.com/eclipse-paho/paho.mqtt.cpp) follow the installation instructions for version 1.5
