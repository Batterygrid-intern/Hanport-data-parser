#include "filereader.hpp"
#include <iostream>



fileReader::fileReader(std::string filepath){
    this->filepath=filepath;
}

void fileReader::open_f(){
    this->fd.open(this->filepath,std::ios::binary);
    if(!this->fd.is_open()){
        throw std::runtime_error("Failed to open file: " + this->filepath);
    }
}
//read the file and close fd afterwards
void fileReader::read_from_file(){
    char c;
    while(fd.get(c)){
        this->data.push_back(c);
    }
    this->fd.close();
    if(this->data.empty()){
        throw std::runtime_error("File is empty or could not be read");
    }
    std::cout << "Read " << this->data.size() << " bytes from file\n";
}

std::vector<uint8_t> fileReader::getdata(){
    return this->data;
}