#include "hpMqttPub.hpp"
#include "hpMessageValidator.hpp"
#include "hpDataParser.hpp"
#include "hpData.hpp"
#include "hpSerialRead.hpp"
#include "hpModbuss.hpp"
#include "config.hpp"
#include "spdlog/sinks/daily_file_sink.h"
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <thread>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <vector>

int main(int argc, char** argv)
{
  //parse command line options
  //build class for this
  std::string configFilePath = "configs/app.ini";
  for (int i = 1; i < argc; ++i)
  {
    std::string a(argv[i]);
    if (a == "--config" || a == "-c")
    {
      if (i + 1 >= argc)
      {
        std::cerr << "Missing value for --config\n";
        return 1;
      }
      configFilePath = argv[++i];
    }
    else if (a == "--help" || a == "-h")
    {
      std::cout << "Usage: " << argv[0] << " [--config <config_file>]\n";
      return 0;
    }
  }

  //initialize config file and read from file.
  Config cfg;
  std::string err;
  try {
    if (!cfg.loadFromFile(configFilePath, err)) {
      std::cerr << "Config: Failed to load config file: " << err << std::endl;
      return 1;
    }
  } catch (const std::exception &e) {
    std::cerr << "Config: unexcpected error while loading config file: " << e.what() << std::endl;
    return 1;
  }
  // create logger from config so log path can be configured per-site
  auto logger = spdlog::daily_logger_st("daily_logger", cfg.get("LOGGING", "PATH", "var/log/hanport/hanport.log"), 23, 59);
  logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
  logger->info("Logger initialized");
  //initialize data object to store parsed data
  hpData data_obj;
  std::vector<uint8_t> raw_hp_message;


  // heartbeat counter (ticks every loop cycle) - will be stored in time_stamp
  float heartbeat = 0.0f;
  // register conversion and packing are provided by hpModbuss (see set_from_hpData)
  // start modbus server to expose hpData on registers

  // initialize mqtt client using hardcoded values
  const std::string brokerAddress = cfg.get("MQTT", "BROKER", "localhost:1883");
  const std::string clientId = cfg.get("MQTT", "CLIENT_ID", "hanport_client");
  const std::string willTopic = cfg.get("MQTT", "WILL_TOPIC", "hanport_client/status");
  const std::string willMessage = cfg.get("MQTT", "WILL_MESSAGE", "offline");
  const std::string site = cfg.get("MQTT", "SITE_ID", "bgs-office");
  const std::string deviceId = cfg.get("MQTT", "DEVICE_ID", "hanport_meter_01");
  const std::string userName = cfg.get("MQTT", "USERNAME", "mqtt_user");
  const std::string password = cfg.get("MQTT", "PASSWORD", "mqtt_password");
  const std::string measurement_topic = cfg.get("MQTT", "MEASUREMENT_TOPIC", "hanport_data");

  HpMqttPub mqtt_publisher(brokerAddress, clientId, site, deviceId);
  mqtt_publisher.buildTopic(measurement_topic);
  mqtt_publisher.setCrendetials(userName, password);
  mqtt_publisher.setLastWill(willTopic, willMessage);
  if (mqtt_publisher.connect()) {
    logger->info("Connected to MQTT broker at {}", brokerAddress);
  } else {
    logger->error("MQTT: Failed to connect to MQTT broker at {}", brokerAddress);
  }
  
  // initialize modbus server
  const int port = std::stoi(cfg.get("MODBUS", "PORT", "502"));
  hpModbuss modbus_server(static_cast<uint16_t>(port));
  try {
    modbus_server.start();
    logger->info("Modbus server started on port {}", port);
  } catch (const std::exception &e) {
    logger->error("Modbus: Failed to start modbus server: {}", e.what());
    // continue without modbus, main functionality still runs
  }

  const char *SERIAL_PORT = cfg.get("SERIALPORT", "PATH", "/dev/ttyAMA0").c_str(); 
  // construct serial_reader with default attributes.
  hpSerialRead serial_reader;
  try {
    serial_reader.openPort(SERIAL_PORT);
  } catch (std::exception &e) {
    logger->error("SerialPort: Failed to open serial port: {}", e.what());
    throw; // Re-throw as this is critical
  }

  // initialize registers once so clients can read initial values immediately
  try {
    modbus_server.set_from_hpData(data_obj, 0);
  } catch (const std::exception &e) {
    logger->warn("Initial modbus register write failed: {}", e.what());
  }

  while (true)
  {
    // tick heartbeat once per loop iteration
    heartbeat += 1.0f;
    // always expose heartbeat even if no serial message arrives
    data_obj.time_stamp = heartbeat;
    raw_hp_message = serial_reader.hpRead();
    /*if (!raw_hp_message.empty()) {
      std::ostringstream oss;
      for (uint8_t &b : raw_hp_message) {
        oss << b;
      }
      logger->debug("Raw serial bytes: {}", oss.str());
    }*/

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
          logger->error("MessageValidator: {}", crcError);
          throw std::runtime_error("Data invalid: " + crcError);
        }
        
        logger->info("Message validated successfully - CRC check passed");
        message_array = message_validator.message_to_string_arr();
      } catch (const std::exception &e) {
        logger->warn("MessageValidator: {}", e.what());
        continue;
      }
      // initalise message_parser
      // save parsed data in data object.
      try {
        hpDataParser message_parser(message_array);
        message_parser.parse_message(data_obj);
        logger->info("Successfully parsed message data");
      } catch (const std::exception &e) {
        logger->error("DataParser: Failed to parse message: {}", e.what());
      }
      raw_hp_message.clear();
    }
    
    try {
      mqtt_publisher.publishAllData(data_obj, site, deviceId);
      logger->info("Data published successfully to MQTT");
    } catch (const std::exception &e) {
      logger->error("MQTT: Failed to publish data: {}", e.what());
    }
    // write registers every loop so heartbeat and last-known values are always exposed
    try {
      modbus_server.set_from_hpData(data_obj, 0);
    } catch (const std::exception &e) {
      logger->warn("Failed to update modbus registers: {}", e.what());
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
