#include "hanport_data.hpp"
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
    //call constructor
    //const char* path_to_testfile = getenv("example_data");
    //read from file as long as.... change state so that you can press a button on the pi or command on computer to restart program?


    int failed_readings = 0;
    std::map<std::string,std::pair<std::string,std::string>> hp_data;
    while(failed_readings < MAX_TRIES){
         
        std::this_thread::sleep_for(std::chrono::seconds(10));
        try{
            HanportData data_obj(EX_DATA_PATH);
            //compare transmitted crc with calculated crc
            
            if(data_obj.get_calculated_crc() != data_obj.get_transmitted_crc()){
                std::cout << "Calucalted CRC = " << std::hex << std::showbase <<  data_obj.get_calculated_crc() << "\nTransmitted CRC = " << data_obj.get_transmitted_crc() << std::dec << std::noshowbase << "\n";
                throw std::runtime_error("Data invalid: calculated crc  not equal to transmitted crc");
            }
            else{
                std::cout << "Calucalted CRC = " << std::hex << std::setiosflags(std::ios::showbase) << data_obj.get_calculated_crc() << "\nTransmitted CRC = " << data_obj.get_transmitted_crc() << std::dec << std::noshowbase << "\n";
                std::cout << "Data is valid" << std::endl;
                //not so efficient?????? tänker något fel här. använder inte klassen rätt?
                std::vector<uint8_t> data_buffer = data_obj.get_hanport_message();
                hp_data = data_obj.hp_data_parser(data_buffer);
                //skriv ut hp_data kolla så att det stämmer
            }
            //reset cout manipulation for std::cout ??
        }
        catch (const std::exception& e){
            //how to make it try again?
            //loop to make the program live?
            //try number of times before shutting down and give a warning?
            //call for destructor?? it passes out of scope so no?
            std::cerr << "\nError " << e.what() << "\n";
            failed_readings ++;
            //return 1;  //change state?
        }
    }
return 0;
}
