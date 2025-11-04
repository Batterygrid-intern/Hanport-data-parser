#include "hpMessageValidator.hpp"
#include "hpDataParser.hpp"
#include "hpData.hpp"
#include "hpSerialRead.hpp"
#include "filereader.hpp"
#include "hpModbuss.hpp"
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

//debugging GDB 
//

//use 2 threads one for reading data one for calculating data or just put everything in the same try block?
//sleep validating tread until reading is completed?
//build binary file that you can flash on any raspberry pi.
//build libraries static or dynamic? static because im either way going to need all the methods i create.?
//states error state ->(reset)-> running state?


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

  // variable to calculate how many failed validations that has occured
  int failed_readings = 0;
  hpData data_obj;
  std::vector<uint8_t> raw_hp_message;
  // open serial port
  const char *SERIAL_PORT = "/dev/ttyAMA0";
  // construct serial_reader with default attributes.
  hpSerialRead serial_reader;
  // heartbeat counter (ticks every loop cycle) - will be stored in time_stamp
  float heartbeat = 0.0f;
  // start modbus server to expose hpData on registers
  hpModbuss modbus_server(static_cast<uint16_t>(port));
  try {
    modbus_server.start();
    std::cout << "Modbus server started on port " << port << "\n";
  } catch (const std::exception &e) {
    std::cerr << "Failed to start modbus server: " << e.what() << "\n";
    // continue without modbus, main functionality still runs
  }
  try
  {
    serial_reader.openPort(SERIAL_PORT);
  }
  catch (std::exception &e)
  {
    std::cerr << "\nError " << e.what() << "\n";
  }

  while (true)
  {
    // tick heartbeat once per loop iteration
    heartbeat += 1.0f;
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
      try
      { // instantiate message_validator object with transmitted raw data from serial port exceptions will be thrown if construction fails
        HanportMessageValidator message_validator(raw_hp_message);
        // compare the crc calculated inside the constructor with the transmitted crc extracted from the message
        if (message_validator.get_calculated_crc() != message_validator.get_transmitted_crc())
        {
          std::cout << "Calucalted CRC = " << std::hex << std::showbase << message_validator.get_calculated_crc() << "\nTransmitted CRC = " << message_validator.get_transmitted_crc() << std::dec << std::noshowbase << "\n";
          throw std::runtime_error("Data invalid: calculated crc  not equal to transmitted crc");
        }
        std::cout << "Calucalted CRC = " << std::hex << std::setiosflags(std::ios::showbase) << message_validator.get_calculated_crc() << "\nTransmitted CRC = " << message_validator.get_transmitted_crc() << std::dec << std::noshowbase << "\n";
        std::cout << "Data is valid" << std::endl;
        // from message_validator return a string array for the data to be parsed
        message_array = message_validator.message_to_string_arr();
      }
      // catch exceptions thrown inside message_validator
      catch (const std::exception &e)
      {
        std::cerr << "\nError " << e.what() << "\n";
        failed_readings++;
        continue;
      }
      // initalise message_parser
      // save parsed data in data object.
      try
      {
        hpDataParser message_parser(message_array);
        message_parser.parse_message(data_obj);
        // store heartbeat in time_stamp so it is exposed via Modbus registers
        data_obj.time_stamp = heartbeat;
        std::cout << "active energy export: " << data_obj.active_enery_import_total << " kw" << std::endl;
        // map important hpData floats into modbus holding registers as 32-bit floats
        // helper: convert float to two uint16_t (big-endian register order)
        auto float_to_regs = [](float f)
        {
          uint32_t u = 0;
          static_assert(sizeof(float) == 4, "float must be 32-bit");
          std::memcpy(&u, &f, sizeof(u));
          uint16_t high = static_cast<uint16_t>((u >> 16) & 0xFFFF);
          uint16_t low = static_cast<uint16_t>(u & 0xFFFF);
          return std::vector<uint16_t>{high, low};
        };

        std::vector<uint16_t> regs;
        // order: time_stamp, active_enery_import_total, active_energy_export_total,
        // reactive_import, reactive_export, active_power_import, active_power_export
        auto append_float = [&](float v)
        {
          auto r = float_to_regs(v);
          regs.insert(regs.end(), r.begin(), r.end());
        };

        append_float(data_obj.time_stamp);
        append_float(data_obj.active_enery_import_total);
        append_float(data_obj.active_energy_export_total);
        append_float(data_obj.reactive_energy_import_total);
        append_float(data_obj.reactive_energy_export_total);
        append_float(data_obj.active_power_import);
        append_float(data_obj.active_power_export);
        append_float(data_obj.reactive_power_import);
        append_float(data_obj.reactive_power_export);
        // per-phase active power
        append_float(data_obj.l1_active_power_import);
        append_float(data_obj.l1_active_power_export);
        append_float(data_obj.l2_active_power_import);
        append_float(data_obj.l2_active_power_export);
        append_float(data_obj.l3_active_power_import);
        append_float(data_obj.l3_active_power_export);
        // per-phase reactive power
        append_float(data_obj.l1_reactive_power_import);
        append_float(data_obj.l1_reactive_power_export);
        append_float(data_obj.l2_reactive_power_import);
        append_float(data_obj.l2_reactive_power_export);
        append_float(data_obj.l3_reactive_power_import);
        append_float(data_obj.l3_reactive_power_export);
        // voltages
        append_float(data_obj.l1_voltage_rms);
        append_float(data_obj.l2_voltage_rms);
        append_float(data_obj.l3_voltage_rms);
        // currents
        append_float(data_obj.l1_current_rms);
        append_float(data_obj.l2_current_rms);
        append_float(data_obj.l3_current_rms);

        // write into modbus server holding registers starting at address 0
        try
        {
          modbus_server.set_holding_registers(0, regs);
        }
        catch (const std::exception &e)
        {
          std::cerr << "Failed to update modbus registers: " << e.what() << "\n";
        }
      }
      catch (const std::exception &e)
      {
        std::cerr << "\nError " << e.what() << "\n";
        failed_readings++;
      }
      raw_hp_message.clear();
    }
  }
  // stop modbus server cleanly before exiting
  try {
    modbus_server.stop();
  } catch (...) {
    // ignore
  }
  return 0;
}
