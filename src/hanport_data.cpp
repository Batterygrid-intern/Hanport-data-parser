#include "hanport_data.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>


HanportData::HanportData(std::string filepath){
    this->filepath=filepath;
    std::ifstream file_d;
    std::vector<uint8_t> data_buffer;
    if(open_fd(this->filepath,file_d)) {
        std::cout << "file opened succesfully\n";
        //read data from file store in buffer
        read_from_fd(data_buffer,file_d);
        //extract transmitted crc from data to class attribute
        
    }
    else{
        std::cout << "failed to open file\n";
    }
     //read file into data buffer
    //extract crc
    //calculate crc
};

uint16_t HanportData::calculate_crc(std::vector<uint8_t> data_buffer){
    

}

int HanportData::open_fd(std::string filepath,std::ifstream& fd){
    fd.open(filepath,std::ios::binary);
    if(!fd.is_open()){
        std::perror("failed to open file");
        return 1;
    }
}

void HanportData::read_from_fd(std::vector<uint8_t>& data_buffer,std::ifstream& fd){
    char c;
    size_t i = 0;
    while(fd.get(c)){
        data_buffer.push_back(c);
        i++;
    }
    fd.close();
}
void extract_message_and_crc(std::vector<uint8_t> message_buf,uint16_t transmitted_crc){

}


