#include <vector>
#include <fstream>
#include <cstdint>


class fileReader{
    private:
    std::string filepath;
    std::vector<uint8_t> data;
    std::ifstream fd; 

    public:

    fileReader(std::string filepath);
    void open_f();
    void read_from_file();
    std::vector<uint8_t> getdata();
};