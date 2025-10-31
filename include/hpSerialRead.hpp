#include <string>
#include <fstream>
#include <vector>
#include <cstdint>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#define BAUD_RATE B1152000

class hpSerialRead{
    private:
    struct termios tty;
    int serial_fd = -1;
    std::vector<uint8_t> complete_data_message;
    std::vector<uint8_t> chunk_messages;

    void hpSetupCflag(struct termios *tty);
    void hpSetupLflag(struct termios *tty);
    void hpSetupIflag(struct termios *tty);
    void hpSetupCc(struct termios *tty);

    public:
    hpSerialRead();
    ~hpSerialRead();
    void openPort(const char* serial_port_path);
    void setupPort();
    void closePort();
    std::vector<uint8_t> hpRead();
   



    /**************************** */
    hpSerialRead(char* filepath);
    std::ifstream open_fd();
    void read_from_fd(std::ifstream& fd);
    std::vector<uint8_t> getdata(); 
};