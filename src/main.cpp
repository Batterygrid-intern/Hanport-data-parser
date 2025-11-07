#include "hpMqttPub.hpp"
#include "hpMessageValidator.hpp"
#include "hpDataParser.hpp"
#include "hpData.hpp"
#include "hpSerialRead.hpp"
#include "hpModbuss.hpp"
// #include "Logger.hpp"   // removed: using std::cout / std::cerr instead
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <thread>
#include <chrono>
#include <cstdint>
#include <cstring>


int main(int argc, char** argv)
{
  // parse command line options
  int port = 1502; // default non-privileged port
  for (int i = 1; i < argc; ++i) {
    std::string a(argv[i]);
    if (a == "--port" || a == "-p") {
      if (i + 1 >= argc) {
        std::cerr << "Missing value for --port\n";
        return 1;
      }
      try {
        port = std::stoi(argv[++i]);
      } catch (...) {
        std::cerr << "Invalid port value\n";
        return 1;
      }
    } else if (a == "--help" || a == "-h") {
      std::cout << "Usage: " << argv[0] << " [--port <port>]\n";
      return 0;
    }
  }

  //initialize data object to store parsed data
  hpData data_obj;
  std::vector<uint8_t> raw_hp_message;
  // open serial port

  const char *SERIAL_PORT = "/dev/ttyAMA0";
  // construct serial_reader with default attributes.
  hpSerialRead serial_reader;

  // heartbeat counter (ticks every loop cycle) - will be stored in time_stamp
  float heartbeat = 0.0f;
  // register conversion and packing are provided by hpModbuss (see set_from_hpData)
  // start modbus server to expose hpData on registers

  // initialize mqtt client using hardcoded values
  const std::string brokerAddress = "localhost:1883";
  const std::string clientId = "hanport_client";
  const std::string willTopic = "hanport_client/status";
  const std::string willMessage = "offline";
  const std::string site = "bgs-office";
  const std::string deviceId = "hanport_meter_01";
  const std::string userName = "mqtt_user";
  const std::string password = "mqtt_password";
  const std::string measurement_topic = "hanport_data";
  
  HpMqttPub mqtt_publisher(brokerAddress, clientId, site, deviceId);
  mqtt_publisher.buildTopic(measurement_topic);
  mqtt_publisher.setCrendetials(userName, password);
  mqtt_publisher.setLastWill(willTopic, willMessage);
  if (mqtt_publisher.connect()) {
    std::cout << "Connected to MQTT broker at " << brokerAddress << std::endl;
  } else {
    std::cerr << "MQTT: Failed to connect to MQTT broker at " << brokerAddress << std::endl;
  }

  // initialize modbus server
  hpModbuss modbus_server(static_cast<uint16_t>(port));
  try {
    modbus_server.start();
    std::cout << "Modbus server started on port " << std::to_string(port) << std::endl;
  } catch (const std::exception &e) {
    std::cerr << "Modbus: Failed to start modbus server: " << e.what() << std::endl;
    // continue without modbus, main functionality still runs
  }
  
  try {
    serial_reader.openPort(SERIAL_PORT);
  } catch (std::exception &e) {
    std::cerr << "SerialPort: Failed to open serial port: " << e.what() << std::endl;
    throw; // Re-throw as this is critical
  }

  // initialize registers once so clients can read initial values immediately
  try {
    modbus_server.set_from_hpData(data_obj, 0);
  } catch (const std::exception &e) {
    std::cerr << "Initial modbus register write failed: " << e.what() << "\n";
  }

  while (true)
  {
    // tick heartbeat once per loop iteration
    heartbeat += 1.0f;
    // always expose heartbeat even if no serial message arrives
    data_obj.time_stamp = heartbeat;
    raw_hp_message = serial_reader.hpRead();
    for (uint8_t &i : raw_hp_message)
    {
      std::cout << i;
    }
    std::cout << std::endl;

    std::vector<std::string> message_array;
    // just for use when testing
    // try to validate the data if failed catch exceptions thrown inside HanportMessageValidator class.
    if (!raw_hp_message.empty()) // find '!'?{
    {
      try { 
        // instantiate message_validator object with transmitted raw data from serial port
        HanportMessageValidator message_validator(raw_hp_message);
        
        // compare the crc calculated inside the constructor with the transmitted crc
        if (message_validator.get_calculated_crc() != message_validator.get_transmitted_crc()) {
          std::string crcError = "CRC mismatch - Calculated: " + 
                                std::to_string(message_validator.get_calculated_crc()) +
                                " Transmitted: " + 
                                std::to_string(message_validator.get_transmitted_crc());
          std::cerr << "MessageValidator: " << crcError << std::endl;
          throw std::runtime_error("Data invalid: " + crcError);
        }
        
        std::cout << "Message validated successfully - CRC check passed" << std::endl;
        message_array = message_validator.message_to_string_arr();
      } catch (const std::exception &e) {
        std::cerr << "MessageValidator: " << e.what() << std::endl;
        continue;
      }
      // initalise message_parser
      // save parsed data in data object.
      try {
        hpDataParser message_parser(message_array);
        message_parser.parse_message(data_obj);
        std::cout << "Successfully parsed message data" << std::endl;
      } catch (const std::exception &e) {
        std::cerr << "DataParser: Failed to parse message: " << e.what() << std::endl;
      }
      raw_hp_message.clear();
    }
    
    try {
      mqtt_publisher.publishAllData(data_obj, site, deviceId);
      std::cout << "Data published successfully to MQTT" << std::endl;
    } catch (const std::exception &e) {
      std::cerr << "MQTT: Failed to publish data: " << e.what() << std::endl;
    }
    // write registers every loop so heartbeat and last-known values are always exposed
    try {
      modbus_server.set_from_hpData(data_obj, 0);
    } catch (const std::exception &e) {
      std::cerr << "Failed to update modbus registers: " << e.what() << "\n";
    }
  }
  // stop modbus server cleanly before exiting
  try {
    modbus_server.stop();
    mqtt_publisher.disconnect();
  } catch (...) {
    // ignore
  }
  return 0;
}
