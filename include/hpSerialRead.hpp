#include <string>
#include <fstream>
#include <vector>
#include <cstdint>
#include <CppLinuxSerial/SerialPort.hpp>

class hpSerialRead{
    private:
    std::string filepath;
    std::vector<std::uint8_t> data;
    public:
    hpSerialRead(std::string filepath);
    std::ifstream open_fd();
    void read_from_fd(std::ifstream& fd);
    std::vector<uint8_t> getdata(); 
};