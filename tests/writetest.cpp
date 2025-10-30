#include <iostream>
#include <vector>
#include <cstdint>
#include "hpSerialRead.hpp"
#include "CppLinuxSerial/SerialPort.hpp"

  
  
  int main(){
       hpSerialRead data_reader("./bin/../exampledata/meter_data.bin");
       std::ifstream fd = data_reader.open_fd();
       /*TEST code
       ************************************************************************************/
       data_reader.read_from_fd(fd);
       // initialize serialport object to read and write data to serial port
       mn::CppLinuxSerial::SerialPort serialport("/dev/ttyAMA0",
                                                 mn::CppLinuxSerial::BaudRate::B_115200,
                                                 mn::CppLinuxSerial::NumDataBits::EIGHT,
                                                 mn::CppLinuxSerial::Parity::NONE,
                                                 mn::CppLinuxSerial::NumStopBits::ONE);
       // open serial port and write to serial port
       std::vector<uint8_t> datatest = data_reader.getdata();

       serialport.SetTimeout(25000);
       serialport.Open();
       serialport.WriteBinary(datatest);

   } 