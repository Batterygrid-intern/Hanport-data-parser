#include "hpMqttPub.hpp"
#include "hpMessageValidator.hpp"
#include "hpDataParser.hpp"
#include "hpData.hpp"
#include "hpSerialRead.hpp"
#include "hpModbuss.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <thread>
#include <chrono>
#include <cstdint>
#include <cstring>
#ifndef EX_DATA_PATH
#define EX_DATA_PATH "ex_data"
#endif
#ifndef MAX_TRIES
#define MAX_TRIES 5
#endif

#ifndef LOG_FILE_PATH
#define LOG_FILE_PATH "/var/log/hanport/hanport.log"
#endif

// Initialize the system and set up error handling
void initializeSystem() {
    try {
        Logger::init(LOG_FILE_PATH);
        Logger::logLatestMessage("System initialization started");
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize logging system: " << e.what() << std::endl;
        throw;
    }
}


//SETUP TTYAMA0 DEVICE TO INTERPRATE MESSAGE CORRECTLY ACCODRING TO THE HANPORT PROTOCOL (1 time thing at start-up)
/***********************************************************************/
/*1. READ DATA FROM SERIAL PORT TTYAMA0
  2. WRITE DATA TO FILE
  ? do we want to read every 10 seconds or activate dr pin every... */

//HANPORTDATA PARSER
/**********************************************************************/
/*3. READ DATA FROM FILE, VALIDATE AND PARSE IT TO JSON (formatted string?) save in std::map?
  4. Parse to string attribute inside class? 
  ? what do we want to do with the data?*/

//MQTT 
/***********************************************************************/
/*5.BROKER RUNNING IN DOCKER CONTAINER ON SAME DEVICE
  6.PUBLISH TO TOPIC..... to broker broker inserts in database?*/
 
//MODBUS
/***********************************************************************/

//DOCUMENTATION
/***********************************************************************/
/*1. write comments and document application
 */
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
  // Determine config path: allow override with HANPORT_CONF env var
  Config cfg;
  std::string cfgPath = std::getenv("HANPORT_CONF") ? std::string(std::getenv("HANPORT_CONF")) : std::string("app.ini");
  std::string cfgErr;
  if (!cfg.loadFromFile(cfgPath, cfgErr)) {
    // If config not found, log and continue with defaults
    Logger::logError("Init", "Failed to load config '" + cfgPath + "': " + cfgErr);
  }

  // Serial path: can be set in config under [serial] path=...
  std::string serialPath = cfg.get("serial", "path", "/dev/ttyAMA0");
  const char *SERIAL_PORT = serialPath.c_str();
  // construct serial_reader with default attributes.
  hpSerialRead serial_reader;

  // heartbeat counter (ticks every loop cycle) - will be stored in time_stamp
  float heartbeat = 0.0f;
  // register conversion and packing are provided by hpModbuss (see set_from_hpData)
  // start modbus server to expose hpData on registers

  // initialize mqtt client using values from config (falls back to sensible defaults)
  // TODO: consider env overrides for production (e.g. HANPORT_MQTT_BROKER)
  const std::string brokerAddress = cfg.get("mqtt", "broker", "localhost:1883");
  const std::string clientId = cfg.get("mqtt", "client_id", "1");
  const std::string willTopic = cfg.get("mqtt", "will_topic", "hanport_client/status");
  const std::string willMessage = cfg.get("mqtt", "will_message", "offline");
  const std::string site = cfg.get("mqtt", "site", "bgs-office");
  const std::string deviceId = cfg.get("mqtt", "device_id", "hanport_meter_01");
  const std::string userName = cfg.get("mqtt", "username", "mqtt_user");
  const std::string password = cfg.get("mqtt", "password", "mqtt_password");
  const std::string measurement_topic = cfg.get("mqtt", "measurement_topic", "hanport_data");

  HpMqttPub mqtt_publisher(brokerAddress, clientId, site, deviceId);
  mqtt_publisher.buildTopic(measurement_topic);
  mqtt_publisher.setCrendetials(userName, password);
  mqtt_publisher.setLastWill(willTopic, willMessage);
  if (mqtt_publisher.connect()) {
    Logger::logLatestMessage("Connected to MQTT broker at " + brokerAddress);
  } else {
    Logger::logError("MQTT", "Failed to connect to MQTT broker at " + brokerAddress);
  }

  // initialize modbus server
  hpModbuss modbus_server(static_cast<uint16_t>(port));
  try {
    modbus_server.start();
    Logger::logLatestMessage("Modbus server started on port " + std::to_string(port));
  } catch (const std::exception &e) {
    Logger::logError("Modbus", std::string("Failed to start modbus server: ") + e.what());
    // continue without modbus, main functionality still runs
  }
  
  try {
    serial_reader.openPort(SERIAL_PORT);
  } catch (std::exception &e) {
    Logger::logError("SerialPort", std::string("Failed to open serial port: ") + e.what());
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
          Logger::logError("MessageValidator", crcError);
          throw std::runtime_error("Data invalid: " + crcError);
        }
        
        Logger::logLatestMessage("Message validated successfully - CRC check passed");
        message_array = message_validator.message_to_string_arr();
      } catch (const std::exception &e) {
        Logger::logError("MessageValidator", e.what());
        continue;
      }
      // initalise message_parser
      // save parsed data in data object.
      try {
        hpDataParser message_parser(message_array);
        message_parser.parse_message(data_obj);
        Logger::logLatestMessage("Successfully parsed message data");
      } catch (const std::exception &e) {
        Logger::logError("DataParser", std::string("Failed to parse message: ") + e.what());
      }
      raw_hp_message.clear();
    }
    
    try {
      mqtt_publisher.publishAllData(data_obj, site, deviceId);
      Logger::logLatestMessage("Data published successfully to MQTT");
    } catch (const std::exception &e) {
      Logger::logError("MQTT", std::string("Failed to publish data: ") + e.what());
      std::cerr << "Failed to publish MQTT data: " << e.what() << "\n";
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
