#include "hpData.hpp"
#include <iostream>
#include <string>


hpData::hpData(std::vector<std::string> message_array){
    this->message_array = message_array;
    if(this->message_array.empty()){
        throw std::runtime_error("object initialisation failed message_array is empty");
    }
}

void hpData::parse_message(){
    //buffer to hold each line extracted from the string array
    std::string line;
    //iterate through each line of the string array
    for(size_t i; i < this->message_array.size();i++){
        line = this->message_array[i];
        if(line.find('-') == std::string::npos){
            continue;
        }
        //obis for time_stamp
        if(line.find("0-0:1.0.0") != std::string::npos){
            parse_time();
        }
        else{
            parse_electricity_data(line);
        }
    }
}
void hpData::parse_time(){
}
//find electricity obis
void hpData::parse_electricity_data(std::string& line){
    //ACTIVE ENERGY
    //active energy import total
    if(line.find("1-0:1.8.0")){
        this->active_enery_import_total = set_value(line);
    }
    //active energy export total
    if(line.find("1-0:2.8.0")){
        this->active_energy_export_total = set_value(line);
    } 

    //REACTIVE ENERGY
    //reactive energy import total
    if(line.find("1-0:3.8.0")){
        this->reactive_energy_import_total = set_value(line);
    }
    //reactive energy export total
    if(line.find("1-0:4.8.0")){
        this->reactive_energy_export_total = set_value(line);
    }

    //INSTANTANEOUS(MOMENTANT) ACTIVE EFFECT 
    //active power import
    if(line.find("1-0:1.7.0")){
        this->active_power_import = set_value(line);
    }
    //active power export
    if(line.find("1-0:2.7.0")){
        this->active_power_export = set_value(line);
    }

    //INSTANTANEOUS(MOMENTANT) REACTIVE EFFECT TOTAL
    //reactive power import
    if(line.find("1-0:3.7.0")){
        this->reactive_power_import = set_value(line);
    }
    //reactive power export
    if(line.find("1-0:4.7.0")){
        this->reactive_power_export = set_value(line);
    }

    //ACTIVE EFFECT FOR EACH PHASE 
    //l1 active power import
    if(line.find("1-0:21.7.0")){
        this->l1_active_power_import = set_value(line);
    }
    //l1 active power export
    if(line.find("1-0:22.7.0")){
        this->l1_active_power_export = set_value(line);
    }
    //l2 active_power_import
    if(line.find("1-0:41.7.0")){
        this->l2_active_power_import = set_value(line);
    }
    //l2 active power export
    if(line.find("1-0:42.7.0")){
        this->l2_active_power_export = set_value(line);
    }
    //l3 active power import
    if(line.find("1-0:61.7.0")){
        this->l3_active_power_import = set_value(line);
    }
    //l3 active power export
    if(line.find("1-0:62.7.0")){
        this->l3_active_power_export = set_value(line);
    }

    //REACTIVE EFFEKT FOR EACH PHASE
    //l1 reactive power import
    if(line.find("1-0:23.7.0")){
        this->l1_reactive_power_import = set_value(line);
    }
    //l1 reactive power export
    if(line.find("1-0:24.1.7.0")){
        this->l1_reactive_power_export = set_value(line);
    }
    //l2 reactive power import
    if(line.find("1-0:43.7.0")){
        this->l2_reactive_power_import = set_value(line);
    }
    //l2 reactive power export
    if(line.find("1-0:44.7.0")){
        this->l2_reactive_power_export = set_value(line);
    }
    //l3 reactive power import
    if(line.find("1-0:63.7.0")){
        this->l3_reactive_power_export = set_value(line);
    }
    //l3 reactive power export
    if(line.find("1-0:64.7.0")){
        this->l3_reactive_power_export = set_value(line);
    }
    
    //VOLTAGE LEVEL FOR EACH PHASE
    //l1 voltage rms
    if(line.find("1-0:32.7.0")){
        this->l1_voltage_rms = set_value(line);
    }
    //l2 voltage rms
    if(line.find("1-0:52.7.0")){
        this->l2_voltage_rms = set_value(line);
    }
    //l3 voltage rms
    if(line.find("1-0:72.7.0")){
        this->l3_voltage_rms = set_value(line);
    }

    //AMPHERE FOR EACH PHASE
    //l1 current rms
    if(line.find("1-0:32.7.0")){
        this->l1_current_rms = set_value(line);
    }
    //l2 current rms
    if(line.find("1-0:51.7.0")){
        this->l2_current_rms = set_value(line);
    }
    //l3 current rms
    if(line.find("1-0:71.7.0")){
        this->l3_current_rms = set_value(line);
    }
}

float hpData::set_value(std::string& line){
    char obis_del =  '(';
    char unit_del = '*';
    float value = 0;
    int start_pos = line.find(obis_del) + 1;
    int end_pos = line.find(unit_del) - 1;
    int num_of_chars = end_pos - start_pos;
    try{
        float value = std::stof(line.substr(start_pos,num_of_chars));
    }
    catch(std::exception& e){
        throw std::runtime_error("failed to convert string to float for value");
    }
}