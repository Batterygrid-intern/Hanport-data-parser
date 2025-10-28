#include <vector>
#include <string>

class hpData{
    private:
    std::vector<std::string>  message_array;
    //time_stamp on the message;
    float time_stamp = 0;
    //active energy(energy used to light up a lightbulb run a motor)
    //(energy that is converted to run devices and is consumed to heat movement or such)
    float active_enery_import_total = 0;
    float active_energy_export_total = 0;
    //reactive energy, energy that is stored in capacitors or not directly consumed by a device) energy that returns in the circut 
    float reactive_energy_import_total = 0; //kvarh
    float reactive_energy_export_total = 0; //kvarh
    //instantaneous(momentan) active effect (total)
    float active_power_import = 0; //kw
    float active_power_export = 0; //kw
    //instantaneous(momentan) reactive effect (total)
    float reactive_power_import = 0;
    float reactive_power_export = 0;
    //active effect per phase(fas) -> (kw)
    float l1_active_power_import = 0;
    float l1_active_power_export = 0;
    float l2_active_power_import = 0;
    float l2_active_power_export = 0;
    float l3_active_power_import = 0;
    float l3_active_power_export = 0;
    //reactive effect per phase(fas) -> (kw)
    float l1_reactive_power_import = 0;
    float l1_reactive_power_export = 0;
    float l2_reactive_power_import = 0;
    float l2_reactive_power_export = 0;
    float l3_reactive_power_import = 0;
    float l3_reactive_power_export = 0;
    //phase(fas) voltage(spänning) -> (volt)
    float l1_voltage_rms = 0;
    float l2_voltage_rms = 0;
    float l3_voltage_rms = 0;
    //current(ampere) ->(ampere - a)
    float l1_current_rms = 0;
    float l2_current_rms = 0;
    float l3_current_rms = 0;


    public:

    hpData(std::vector<std::string> message__array);
   
    //method to parse_message from string array
    void parse_message();
    //method to parse the time object
    void parse_time();
    //method to parse the electricity data
    void parse_electricity_data(std::string& line);
    //method do set the value to value attribute
    float set_value(std::string& line);

    //transformera värde attribut till dem värden vi vill skicka kw /mw

    //getters



};
