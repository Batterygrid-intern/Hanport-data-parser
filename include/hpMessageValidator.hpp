#include <vector>
//purpose of this class is to read, validate and parse data to a semistructured format like json csv yaml data from a electricty meater 
class HanportMessageValidator{
    private:
    std::vector<uint8_t> raw_hp_message;
    std::vector<uint8_t> hp_meter_data;
    uint16_t calculated_crc;
    uint16_t transmitted_crc;
       
    public:
    //Constructor methods 
    /***********************************************************************'*/
    HanportMessageValidator(std::vector<uint8_t> raw_hp_message);
    void calculate_crc();
    size_t extract_message();
    void extract_crc(size_t exclamation_pos);
    //Getters
    /************************************************************************ */
    uint16_t get_calculated_crc();
    uint16_t get_transmitted_crc();
    std::vector<std::string>  message_to_string_arr();
};
