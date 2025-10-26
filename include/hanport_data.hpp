#include <vector>
#include <map>
//skapa en class med olika interface.?





//purpose of this class is to read, validate and parse data to a semistructured format like json csv yaml data from a electricty meater 
class HanportData{
    private:
    //data that we want to live inside the class.
    std::vector<uint8_t> hanport_message;
    std::string filepath;
    uint16_t calculated_crc;
    uint16_t transmitted_crc;
    
    public: 
    /***********************************************************************'*/
    HanportData(std::string filepath);
    void calculate_crc(std::vector<uint8_t>& hanport_message);
    void open_fd(std::string& filepath,std::ifstream& fd);
    void read_from_fd(std::vector<uint8_t>& data_buffer,std::ifstream& fd);
    void extract_message_and_crc(std::vector<uint8_t> &data_buffer);
    /************************************************************************ */
    uint16_t get_calculated_crc();
    uint16_t get_transmitted_crc();
    std::vector<uint8_t> get_hanport_message();
    /************************************************************************** */
    std::map<std::string, std::pair<std::string,std::string>> hp_data_parser(std::vector<uint8_t>& hanport_message);

};
