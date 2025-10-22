#include "hanport_data.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
//START OF CONSTRUCTOR METHODS
/*************************************************************************** */
HanportData::HanportData(std::string filepath){
    this->filepath=filepath;
    std::ifstream file_d;
    std::vector<uint8_t> data_buffer;
    //following methods will be used to instantiate the rest of the privat attributes
    if(open_fd(this->filepath,file_d)) {
        std::cout << "file opened succesfully\n";
        //read data from file store in buffer
        read_from_fd(data_buffer,file_d);
        //extract transmitted crc from data to class attributes
        extract_message_and_crc(data_buffer);
        //calculate new crc from message
        calculate_crc(this->hanportmessage);
    }

    else{
        std::cout << "failed to open file\n";
    }
     //read file into data buffer
    //extract crc
    //calculate crc
};

void HanportData::calculate_crc(std::vector<uint8_t>& hanport_message){
   uint16_t crc = 0x0000;
   for (size_t i = 0; i < hanport_message.size(); i++){
       crc ^= hanport_message[i];
       for (int j = 0; j < 8; j++){
           if (crc & 0x0001){
               crc = (crc >> 1) ^ 0xA001;
           }
           else{
               crc >>= 1;
           }
       }
   }
   this->calculated_crc = crc;
}

int HanportData::open_fd(std::string filepath,std::ifstream& fd){
    fd.open(filepath,std::ios::binary);
    if(!fd.is_open()){
        std::perror("failed to open file");
        return 1;
    }
}
//read the file and close fd afterwards
void HanportData::read_from_fd(std::vector<uint8_t>& data_buffer,std::ifstream& fd){
    
    char c;
    size_t i = 0;
    while(fd.get(c)){
        data_buffer.push_back(c);
        i++;
    }
    fd.close();
}
//initialize the hanportmessage attribute and transmitted crc attribute
void HanportData::extract_message_and_crc(std::vector<uint8_t>& data_buffer){
    size_t exclamation_pos = 0;
    for(size_t i = 0; i < data_buffer.size(); i++){
        if(data_buffer[i] == '!'){
            exclamation_pos = i;
            break;
        }
    }
    //extract message part and assign it to class attribute
    this->hanportmessage.assign(data_buffer.begin(),data_buffer.begin() + exclamation_pos +1);
    //extract crc from data_buffer
    std::string crc_string;
    for(size_t i = exclamation_pos + 1; i < data_buffer.size();i++){
        if(data_buffer[i] != '\r' && data_buffer[i] != '\n'){
            crc_string += (char)data_buffer[i];
        }
    }
    //cast the crc_string type to uint16_t type and instat
    this->transmitted_crc = std::stoi(crc_string,nullptr,16);
}
/*********************************************************************************' */
//GETTERS 
//write getters do extract value of crcs



