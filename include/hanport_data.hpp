#include <vector>
//skapa en class med olika interface.?





//purpose of this class is to read, validate and parse data to a semistructured format like json csv yaml data from a electricty meater 
class HanportData{
    private:
    //data that we want to live inside the class.
    std::vector<uint8_t> hanportmessage;
    std::string filepath;
    uint16_t calculated_crc;
    uint16_t transmitted_crc;
    
    public:
    //skapa ett object
    //När objektet skapas vill vi konstruera den med datan.
    //Konstruktorn ska skapa ett objekt läsa data från hanporten validera crcn.
    //Stämmer datan skapas ett objekt med meddelandet
    //Stämmer inte crcn förstörs objektet och vi får försöka läsa ut ny data. 
    HanportData(std::string filepath);
    //kalkylera crcn
    void calculate_crc(std::vector<uint8_t>& hanportmessage);
    void open_fd(std::string& filepath,std::ifstream& fd);
    void read_from_fd(std::vector<uint8_t>& data_buffer,std::ifstream& fd);
    void extract_message_and_crc(std::vector<uint8_t> &data_buffer);

};
