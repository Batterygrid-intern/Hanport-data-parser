#include "hanport_data.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <string>
#include <sstream>

//TODO
//bygg om klassen så den tar in listor som objekt
//START OF CONSTRUCTOR METHODS
/*************************************************************************** */
HanportData::HanportData(std::string filepath){
    //bygg om 
    this->filepath=filepath;
    std::ifstream file_d;
    std::vector<uint8_t> data_buffer;
    //following methods will be used to instantiate the rest of the privat attributes
    open_fd(this->filepath,file_d); 
    // read data from file store in buffer
    read_from_fd(data_buffer,file_d);
    //extract transmitted crc from data to class attributes
    extract_message_and_crc(data_buffer);
    //calculate new crc from message
    calculate_crc(this->hanport_message);

    message_to_string_ar();
};
//method to calculate the crc and initialize calculated_crc attribute
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

void HanportData::open_fd(std::string& filepath,std::ifstream& fd){
    fd.open(filepath,std::ios::binary);
    if(!fd.is_open()){
        throw std::runtime_error("Failed to open file: " + filepath);
    }
}
//read the file and close fd afterwards
void HanportData::read_from_fd(std::vector<uint8_t>& data_buffer,std::ifstream& fd){
    char c;
    while(fd.get(c)){
        data_buffer.push_back(c);
    }
    fd.close();
    if(data_buffer.empty()){
        throw std::runtime_error("File is empty or could not be read");
    }
    std::cout << "Read " << data_buffer.size() << " bytes from file\n";
}
//initialize the hanport_message attribute and transmitted crc attribute
void HanportData::extract_message_and_crc(std::vector<uint8_t>& data_buffer){
    size_t exclamation_pos = 0;
    bool found = false;
    for(size_t i = 0; i < data_buffer.size(); i++){
        if(data_buffer[i] == '!'){
            exclamation_pos = i;
            found = true;
            break;
        }
    }

    if(!found){
        throw std::runtime_error("no '!' found invalid dataformat"); 
    }
    //extract message part and assign it to class attribute ...
    this->hanport_message.assign(data_buffer.begin(),data_buffer.begin() + exclamation_pos +1);
    //extract crc from data_buffer
    std::string crc_string;
    for(size_t i = exclamation_pos + 1; i < data_buffer.size();i++){
        if(data_buffer[i] != '\r' && data_buffer[i] != '\n'){
            crc_string += (char)data_buffer[i];
        }
    }

    if(crc_string.empty()){
        throw std::runtime_error("invalid data format, missing CRC value");
    }
    //cast the crc_string type to uint16_t type and instat
    try{
        this->transmitted_crc = static_cast<uint16_t>(std::stoi(crc_string,nullptr,16));
    }
    catch(const std::exception& e){
        throw std::runtime_error("Failed to parse crc value");
    }
}
void HanportData::message_to_string_ar(){
    std::string dt_buffer(this->hanport_message.begin(),this->hanport_message.end());
    std::stringstream ss(dt_buffer);
    std::string line_buf;
    while(getline(ss,line_buf)){
        this->str_ar.push_back(line_buf);
    }
    if(this->str_ar.empty()){
        throw std::runtime_error("failed parse hanport_message to string array");
    }
}
//PARSE /************************************************************************************ */
void HanportData::parse_message(){
    //gå igenom hela listan med strängar
    //plocka ut obis värdet lägg som nyckel, float som tar emot det andra värdet
    //
    std::string buffer;
    for(size_t i = 0; i < this->str_ar.size(); i++){
        buffer = this->str_ar[i];
        if(buffer.find('(') && buffer.find('*')){
           parse_meter_message(buffer);
        }
        else if (buffer.find('(')){
            parse_time_message(buffer);
        }
        else {
            continue;
        }
    }
    for(const auto&  p : this->parsed_data){
    std::cout << p.first << " "  << p.second<< std::endl; 
    }
}


void HanportData::parse_meter_message(std::string& str_buf){
    char obis_del = '(';
    char last_del = '*';
    int start_pos = 0; 
    int obis_pos = str_buf.find(obis_del);
    int last_pos = str_buf.find(last_del);
    std::string obis_code = str_buf.substr(start_pos,obis_pos-1);
    std::string value = str_buf.substr(obis_pos + 1, last_pos - obis_pos -1);
    std::cout << value << std::endl;
    //this->parsed_data[obis_code] = std::stof(value);
    //omvandla direkt till rätt format / kw 
    //vinter / sommar tid  
    //spara bara konverterade eller aggergerade värden.
}
void HanportData::parse_time_message(std::string& str_buf){
    char obis_del = '(';
    char last_del = ')';
    int obis_pos = str_buf.find(obis_del);
    int last_pos = str_buf.find(last_del);
    std::string time_value = str_buf.substr(obis_pos + 1, last_pos - obis_pos -1);
    std::cout << time_value << std::endl;
    //this->parsed_data["Time_stamp"] = std::stof(time_value);
}

//GETTERS 
/************************************************************************************/
uint16_t HanportData::get_calculated_crc(){
    return this->calculated_crc;
}
uint16_t HanportData::get_transmitted_crc(){
    return this->transmitted_crc;
}
std::vector<uint8_t> HanportData::get_hanport_message(){
    return this->hanport_message;
}

//DATA PARSING
/*******************************************************************/
//string array seperate by each line

//iterate through string

//switch case for each obis 


//method for extracting time

//method for extracting values












/*std::map<std::string,std::pair<std::string,std::string>> HanportData::hp_data_parser(std::vector<uint8_t>&hanport_message){
    std::string dt_buffer(hanport_message.begin(),hanport_message.end());
    //define map to store all key values pairs with obis id and data related to it
    std::map<std::string,std::pair<std::string,std::string>> hp_data;
    //std::cout << dt_buffer;
    //alla delimeters att leta efter 
    std::string start_del = "-";
    std::string obis_del = "(";
    std::string value_del = "*";
    std::string unit_del = ")";
    //definiera positionerna för första läsningen
    auto start_pos = dt_buffer.find(start_del) - 1;
    auto unit_pos = 0;
    auto value_pos = 0;

    std::string obis_code;
    std::string value;
    std::string unit;
    //auto next_start_pos = dt_buffer.find(unit_del) + 3;
    //för att flytta fram till nästa rad och börja läsa men medelandena är olika långa.
    std::cout << "\n" << dt_buffer[start_pos] << std::endl;
    auto obis_pos = dt_buffer.find(obis_del);
    while(obis_pos != std::string::npos){

        // hitta positionen för value delaren
        value_pos = dt_buffer.find(value_del,obis_pos);
        // hitta positionen mät enheten
        unit_pos = dt_buffer.find(unit_del, value_pos);
        //plocka ut obis del
        obis_code = dt_buffer.substr(start_pos,obis_pos-start_pos -1);
        std::cout << obis_code << "\n";
        // Plocka ut värde delen
        value = dt_buffer.substr(obis_pos + 1, value_pos - obis_pos -1);
        std::cout << value  << "\n";
        // Plocka ut mätvärde enheten
        unit = dt_buffer.substr(value_pos + 1, unit_pos - value_pos -1);
        std::cout << unit << "\n";
        //lägg alla 3 behållare i en map med obis_code som nyckel
        hp_data[obis_code] = std::make_pair(value,unit);

        start_pos = dt_buffer.find(unit_del,start_pos) + 3;
        if(start_pos != 1){
            break;
        }
        obis_pos = dt_buffer.find(obis_del,start_pos);
        // flytta fram från s till nästa rad? behöver jag göra det hur gör jag det?
        // start + unit del + 1
    }
    return hp_data;
}*/


//
/*auto value_pos = dt_buffer.find(value_del);
        auto unit_pos = dt_buffer.find(unit_del);
        std::string obis_code = dt_buffer.substr(start_pos,obis_pos);
        std::string value = dt_buffer.substr(obis_pos + 1, value_pos - obis_pos -1);
        std::string unit = dt_buffer.substr(value_pos + 1, unit_pos - value_pos -1);
        hp_data[obis_code] = std::make_pair(value,unit);
        obis_pos = dt_buffer.find(obis_del);
        break;
*/
