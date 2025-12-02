#include "hpDataParser.hpp"
#include "hpData.hpp"
#include <iostream>
#include <string>


hpDataParser::hpDataParser(std::vector<std::string> message_array){
    this->message_array = message_array;
    if(this->message_array.empty()){
        throw std::runtime_error("object initialisation failed: message_array is empty");
    }
}
void hpDataParser::parse_message(hpData& dt_obj){
    //buffer to hold each line extracted from the string array
    std::string line;
    //iterate through each line of the string array
    for(size_t i = 0; i < this->message_array.size();i++){
        line = this->message_array[i];
        if(line.find('-') == std::string::npos){
            continue;
        }
        //obis for time_stamp
        else if(line.find("0-0:1.0.0") != std::string::npos){
            continue;
        }
        else{
            char obis_del = ('(');
            std::string obis_code = line.substr(0,line.find(obis_del));
            parse_electricity_data(line,obis_code, dt_obj);
        }
    }
}
//do we want this?
float hpDataParser::parse_time(std::string& line){
    float time_value = 0; 
    char obis_del('(');
    char time_del(')');
    int start_pos = static_cast<int>(line.find(obis_del));
    if (start_pos == (int)std::string::npos) throw std::runtime_error("failed to locate time '(' in line");
    start_pos += 1;
    int last_pos = static_cast<int>(line.find(time_del));
    if (last_pos == (int)std::string::npos) throw std::runtime_error("failed to locate time ')' in line");
    int read_x_chars = last_pos - start_pos; // number of characters between '(' and ')'
    try{
        time_value = std::stof(line.substr(start_pos, read_x_chars));
    }
    catch(std::exception& e){
        throw std::runtime_error("failed to convert time_value to float");
    }
    return time_value;  
}
float hpDataParser::set_value(std::string& line){
    float value = 0;
    char obis_del =  '(';
    char value_del = '*';
    int start_pos = static_cast<int>(line.find(obis_del));
    if (start_pos == (int)std::string::npos) throw std::runtime_error("failed to locate '(' in value line");
    start_pos += 1;
    int end_pos = static_cast<int>(line.find(value_del));
    if (end_pos == (int)std::string::npos) throw std::runtime_error("failed to locate '*' in value line");
    int read_x_chars = end_pos - start_pos; // characters between '(' and '*'
    try{
        value = std::stof(line.substr(start_pos, read_x_chars));
    }
    catch(std::exception& e){
        throw std::runtime_error("failed to convert string to float for value");
    }
    return value;
}
//find electricity obis
void hpDataParser::parse_electricity_data(std::string& line,std::string& obis_code,hpData& dt_obj){
    //ACTIVE ENERGY
    //active energy import total
    if(obis_code == "1-0:1.8.0"){
        dt_obj.active_energy_import_total = set_value(line);
    }
    //active energy export total
    else if(obis_code == "1-0:2.8.0"){
        dt_obj.active_energy_export_total = set_value(line);
    } 

    //REACTIVE ENERGY
    //reactive energy import total
    else if(obis_code == "1-0:3.8.0"){
       dt_obj.reactive_energy_import_total = set_value(line);
    }
    //reactive energy export total
    else if(obis_code == "1-0:4.8.0"){
        dt_obj.reactive_energy_export_total = set_value(line);
    }

    //INSTANTANEOUS(MOMENTANT) ACTIVE EFFECT 
    //active power import
    else if(obis_code == "1-0:1.7.0"){
        dt_obj.active_power_import = set_value(line);
    }
    //active power export
    else if(obis_code == "1-0:2.7.0"){
        dt_obj.active_power_export = set_value(line);
    }

    //INSTANTANEOUS(MOMENTANT) REACTIVE EFFECT TOTAL
    //reactive power import
    else if(obis_code == "1-0:3.7.0"){
        dt_obj.reactive_power_import = set_value(line);
    }
    //reactive power export
    else if(obis_code == "1-0:4.7.0"){
        dt_obj.reactive_power_export = set_value(line);
    }

    //ACTIVE EFFECT FOR EACH PHASE 
    //l1 active power import
    else if(obis_code == "1-0:21.7.0"){
        dt_obj.l1_active_power_import = set_value(line);
    }
    //l1 active power export
    else if(obis_code == "1-0:22.7.0"){
        dt_obj.l1_active_power_export = set_value(line);
    }
    //l2 active_power_import
    else if(obis_code == "1-0:41.7.0"){
        dt_obj.l2_active_power_import = set_value(line);
    }
    //l2 active power export
    else if(obis_code == "1-0:42.7.0"){
        dt_obj.l2_active_power_export = set_value(line);
    }
    //l3 active power import
    else if(obis_code == "1-0:61.7.0"){
        dt_obj.l3_active_power_import = set_value(line);
    }
    //l3 active power export
    else if(obis_code == "1-0:62.7.0"){
        dt_obj.l3_active_power_export = set_value(line);
    }

    //REACTIVE EFFEKT FOR EACH PHASE
    //l1 reactive power import
    else if(obis_code == "1-0:23.7.0"){
        dt_obj.l1_reactive_power_import = set_value(line);
    }
    //l1 reactive power export
    else if(obis_code == "1-0:24.7.0"){
        dt_obj.l1_reactive_power_export = set_value(line);
    }
    //l2 reactive power import
    else if(obis_code == "1-0:43.7.0"){
        dt_obj.l2_reactive_power_import = set_value(line);
    }
    //l2 reactive power export
    else if(obis_code == "1-0:44.7.0"){
        dt_obj.l2_reactive_power_export = set_value(line);
    }
    //l3 reactive power import
    else if(obis_code == "1-0:63.7.0"){
        dt_obj.l3_reactive_power_export = set_value(line);
    }
    //l3 reactive power export
    else if(obis_code == "1-0:64.7.0"){
        dt_obj.l3_reactive_power_export = set_value(line);
    }
    
    //VOLTAGE LEVEL FOR EACH PHASE
    //l1 voltage rms
    else if(obis_code == "1-0:32.7.0"){
        dt_obj.l1_voltage_rms = set_value(line);
    }
    //l2 voltage rms
    else if(obis_code == "1-0:52.7.0"){
        dt_obj.l2_voltage_rms = set_value(line);
    }
    //l3 voltage rms
    else if(obis_code == "1-0:72.7.0"){
        dt_obj.l3_voltage_rms = set_value(line);
    }

    //AMPHERE FOR EACH PHASE
    //l1 current rms
    else if(obis_code == "1-0:31.7.0"){
        dt_obj.l1_current_rms = set_value(line);
    }
    //l2 current rms
    else if(obis_code == "1-0:51.7.0"){
        dt_obj.l2_current_rms = set_value(line);
    }
    //l3 current rms
    else if (obis_code == "1-0:71.7.0"){
        dt_obj.l3_current_rms = set_value(line);
    }
    else{
        std::cout << "Obis code: "<< obis_code << " did not find a match\n";
    }
}

