#include <iostream>
#include <vector>
#include "hpData.hpp"
#pragma once
class hpDataParser{
    private:
    std::vector<std::string>  message_array;
    public:

    hpDataParser(std::vector<std::string> message__array);
   
    //method to parse_message from string array
    void parse_message(hpData& obj);
    
    //method to parse the electricity data
    void parse_electricity_data(std::string& line,std::string& obis_code,hpData& obj);
    //method to set the value of time attribute 
    float parse_time(std::string &line);
    //method do set the value to value attribute
    float set_value(std::string& line);

    //getters to get data and transform how you want?
    //transformera värde attribut till dem värden vi vill skicka kw /mw olika metoder för att transformera olika datavärden

    //getters
};
