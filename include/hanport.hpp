#include <vector>
//skapa en class med olika interface.?





//purpose of this class is to read, validate and parse data to a semistructured format like json csv yaml data from a electricty meater 
class Hanport{
    private:
    //data that we want to live inside the class.
    std::vector<uint8_t> data_buffer;
    //throw 

    //catch

    public:
    //skapa ett object
    //När objektet skapas vill vi konstruera den med datan.
    //Konstruktorn ska skapa ett objekt läsa data från hanporten validera crcn.
    //Stämmer datan skapas ett objekt med meddelandet
    //Stämmer inte crcn förstörs objektet och vi får försöka läsa ut ny data. 
    Hanport(std::vector<uint8_t> data_buffer);

    //Tolka datan formatera den som en text med olika nyckelvärden. 
    void parse_data();

    void data_to_json();

};