#include "hpSerialRead.hpp"
#include <iostream>
#include <cstdint>

/*READ FROM FILE*/
hpSerialRead::hpSerialRead(std::string filepath){
    this->filepath=filepath;
}
std::ifstream hpSerialRead::open_fd(){
    std::ifstream fd; 
    fd.open(this->filepath,std::ios::binary);
    if(!fd.is_open()){
        throw std::runtime_error("Failed to open file: " + filepath);
    }
    return fd;
}
//read the file and close fd afterwards
void hpSerialRead::read_from_fd(std::ifstream& fd){
    char c;
    while(fd.get(c)){
        this->data.push_back(c);
    }
    fd.close();
    if(this->data.empty()){
        throw std::runtime_error("File is empty or could not be read");
    }
    std::cout << "Read " << this->data.size() << " bytes from file\n";
}

std::vector<uint8_t> hpSerialRead::getdata(){
    return this->data;
}