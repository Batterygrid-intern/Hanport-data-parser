#include <vector>
#include <string>
//publik för att enkelt komma åt utanför klassen eller privat med getters
//metoder för att transformera eller transformera på en gång och till vad?
//brukar man använda andra klasser eller structs som attribut i en klass?
//bygga läsning från seriell porten sen modbus kommunikationen.
#pragma once
class hpData{
    public:
    //time_stamp on the message;
    int heart_beat = 0;
    //active energy(energy used to light up a lightbulb run a motor)
    //(energy that is converted to run devices and is consumed to heat movement or such)
    float active_energy_import_total = 0;
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
    private:

};
