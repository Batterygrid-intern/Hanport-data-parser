#include "hpMessageValidator.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <string>
#include <sstream>
#include <ctime>

//TODO

//START OF CONSTRUCTOR METHOD
/*************************************************************************** */
HanportMessageValidator::HanportMessageValidator(std::vector<uint8_t> &raw_hp_message){
    //initialize attribut with 
    this->raw_hp_message=raw_hp_message;
    //split the message from the crc and return the position of the end message indicator
    size_t exclamation_pos = extract_message();
    //extract the src and convert it to unit_16 type
    extract_crc(exclamation_pos);
    //calculate new crc from message extracted
    calculate_crc();
};

//extract the message part from the crc_part to be able to calculate the crc on the messsage part only from / to !
size_t HanportMessageValidator::extract_message(){
    size_t exclamation_pos = 0;
    bool found = false;
    for(size_t i = 0; i < this->raw_hp_message.size(); i++){
        if(this->raw_hp_message[i] == '!'){
            exclamation_pos = i;
            found = true;
            break;
        }
    }
    if(!found){
        throw std::runtime_error("no '!' found invalid dataformat"); 
    }
    //extract message part and assign it to class attribute ...
    this->hp_meter_data.assign(this->raw_hp_message.begin(),this->raw_hp_message.begin() + exclamation_pos +1);
    return exclamation_pos;
}

//extract transmitted crc from hp message to be able to compare with caclulated crc for data validation  
void HanportMessageValidator::extract_crc(size_t exclamation_pos){
    std::string crc_string;
    for(size_t i = exclamation_pos + 1; i < this->raw_hp_message.size();i++){
        if(this->raw_hp_message[i] != '\r' && this->raw_hp_message[i] != '\n'){
            crc_string += (char)this->raw_hp_message[i];
        }
    }
    if(crc_string.empty()){
        throw std::runtime_error("invalid data format, missing CRC value");
    }
    //cast the crc_string type to uint16_t 
    try{
        this->transmitted_crc = static_cast<uint16_t>(std::stoi(crc_string,nullptr,16));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Failed to parse crc value");
    }
}

//calculate the crc of the extracted data
void HanportMessageValidator::calculate_crc(){
    uint16_t crc = 0x0000;
    for (size_t i = 0; i < this->hp_meter_data.size(); i++){
        crc ^= this->hp_meter_data[i];
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

//return an array of strings from the hp_data_message extracted
std::vector<std::string> HanportMessageValidator::message_to_string_arr(){
    std::string dt_buffer(this->hp_meter_data.begin(),this->hp_meter_data.end());
    std::vector<std::string> str_arr;
    std::stringstream ss(dt_buffer);
    std::string line_buf;
    while(getline(ss,line_buf)){
        str_arr.push_back(line_buf);
    }
    if(str_arr.empty()){
        throw std::runtime_error("failed parse hanport_message to string array");
    }
    return str_arr;
}

//GETTERS 
/************************************************************************************/
uint16_t HanportMessageValidator::get_calculated_crc(){
    return this->calculated_crc;
}
uint16_t HanportMessageValidator::get_transmitted_crc(){
    return this->transmitted_crc;
}

