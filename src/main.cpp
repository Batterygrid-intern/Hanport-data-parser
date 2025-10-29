#include "hpMessageValidator.hpp"
#include "hpDataParser.hpp"
#include "hpData.hpp"
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <thread>
#include <chrono>
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
int main(/*int argc, char** argv*/){
    //variable to calculate how many failed validations that is made
    int failed_readings = 0;
    hpData data_obj;
    while(failed_readings < MAX_TRIES){
        //array for raw message read from serial port && array of strings to store the data line by line for data_parsing
        std::vector<uint8_t> raw_hp_message;
        std::vector<std::string> message_array; 
        //just for use when testing
        std::this_thread::sleep_for(std::chrono::seconds(2));
        //try to validate the data if failed catch exceptions thrown inside HanportMessageValidator class.
        try{
            //instantiate message_validator object with transmitted raw data from serial port exceptions will be thrown if construction fails 
            HanportMessageValidator message_validator(raw_hp_message); 
            //compare the crc calculated inside the constructor with the transmitted crc extracted from the message 
            if(message_validator.get_calculated_crc() != message_validator.get_transmitted_crc()){
                std::cout << "Calucalted CRC = " << std::hex << std::showbase <<  message_validator.get_calculated_crc() << "\nTransmitted CRC = " << message_validator.get_transmitted_crc() << std::dec << std::noshowbase << "\n";
                throw std::runtime_error("Data invalid: calculated crc  not equal to transmitted crc");
            }
            std::cout << "Calucalted CRC = " << std::hex << std::setiosflags(std::ios::showbase) << message_validator.get_calculated_crc() << "\nTransmitted CRC = " << message_validator.get_transmitted_crc() << std::dec << std::noshowbase << "\n";
            std::cout << "Data is valid" << std::endl;
            //from message_validator return a string array for the data to be parsed
            message_array = message_validator.message_to_string_arr();
        }
        //catch exceptions thrown inside message_validator
        catch (const std::exception& e){
            std::cerr << "\nError " << e.what() << "\n";
            failed_readings ++;
            continue;
        }
        //initalise message_parser 
        //save parsed data in data object. 
        try{
          hpDataParser message_parser(message_array);
          message_parser.parse_message(data_obj);
        }
        catch(const std::exception& e){
          std::cerr << "\nError " << e.what() << "\n";
          failed_readings ++;
        }
    }
    return 0;
}
