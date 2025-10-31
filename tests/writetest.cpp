#include <iostream>
#include <vector>
#include <cstdint>
#include "hpSerialRead.hpp"
#include "CppLinuxSerial/SerialPort.hpp"
#include "filereader.hpp"
#include <thread>  
#include <chrono>
  int main(){
       fileReader data_reader("./bin/../exampledata/meter_data.bin");
       data_reader.open_f();
       /*TEST code
       ************************************************************************************/
       data_reader.read_from_file();
       std::vector<uint8_t> datatest = data_reader.getdata();
       // initialize serialport object to read and write data to serial port
       mn::CppLinuxSerial::SerialPort serialport("/dev/ttyAMA0",
                                                 mn::CppLinuxSerial::BaudRate::B_115200,
                                                 mn::CppLinuxSerial::NumDataBits::EIGHT,
                                                 mn::CppLinuxSerial::Parity::NONE,
                                                 mn::CppLinuxSerial::NumStopBits::ONE);
       // open serial port and write to serial port
       serialport.SetTimeout(25000);
       serialport.Open();
       while(true){
         std::this_thread::sleep_for(std::chrono::seconds(10));
         serialport.WriteBinary(datatest);
       }

   } 