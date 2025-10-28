void HanportMessageValidator::parse_time_message(std::string& str_buf){
    char obis_del = '(';
    char last_del = ')';
    int obis_pos = str_buf.find(obis_del);
    int last_pos = str_buf.find(last_del);
    std::tm time_obj = {};

    std::string time_value = str_buf.substr(obis_pos + 1, last_pos - obis_pos -1);
    if(time_value.size() > 12){
        time_value = time_value.substr(0,12);
    }
    time_obj.tm_year= std::stof(time_value.substr(0,2)) + 100;
    time_obj.tm_mon = std::stof(time_value.substr(2,2)) - 1;
    time_obj.tm_mday = std::stof(time_value.substr(4,2));
    time_obj.tm_hour = std::stof(time_value.substr(6,2));
    time_obj.tm_min = std::stof(time_value.substr(8,2));
    time_obj.tm_sec = std::stof(time_value.substr(10,2));

    std::time_t unix_time = std::mktime(&time_obj);
    std::cout << std::put_time(&time_obj, "%Y-%m-%d %H:%M:%S")<< std::endl;
    std::cout << "Unix timestamp: " << unix_time << std::endl;
    ////this->parsed_data["Time_stamp"] = std::stof(time_value);
}


void HanportMessageValidator::parse_meter_message(std::string& str_buf){
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



void HanportMessageValidator::parse_message(){
    //gå igenom hela listan med strängar
    //plocka ut obis värdet lägg som nyckel, float som tar emot det andra värdet
    std::string buffer;
    for(size_t i = 0; i < this->str_ar.size(); i++){
        buffer = this->str_ar[i];
        if (buffer.find(this->time_stamp)!= std::string::npos){
            //std::cout << "time stamp obis found: " << buffer << std::endl;
            parse_time_message(buffer);
        }
    }
    for(const auto&  p : this->parsed_data){
    std::cout << p.first << " "  << p.second<< std::endl; 
    }
}
