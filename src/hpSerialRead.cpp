#include "hpSerialRead.hpp"
#include <iostream>
#include <cstdint>
#include <fstream>

hpSerialRead::~hpSerialRead(){
    closePort();
} 
void hpSerialRead::openPort(const char* serial_port_path){
    this->serial_fd = open(serial_port_path,O_RDONLY);
    sleep(2);
    tcflush(this->serial_fd, TCIOFLUSH);

    if(this->serial_fd < 0){
        std::string error = "Failed to open serial port: " + std::string(serial_port_path);
        std::cerr << "SerialRead: " << error << std::endl;
        throw std::runtime_error(error);
    }
    std::cout << "Successfully opened serial port: " << serial_port_path << std::endl;
    setupPort();
}
void hpSerialRead::setupPort(){
    if(tcgetattr(this->serial_fd,&this->tty) != 0){
        std::string error = "Failed to get attributes for serial port";
        std::cerr << "SerialRead: " << error << std::endl;
        throw std::runtime_error(error);
    }
    hpSetupCflag(&this->tty);
    hpSetupLflag(&this->tty);
    hpSetupIflag(&this->tty);
    hpSetupCc(&this->tty);

    cfsetispeed(&this->tty,B115200);

    if(tcsetattr(this->serial_fd,TCSANOW,&this->tty) != 0){
        std::string error = "Failed to set attributes for serial port";
        std::cerr << "SerialRead: " << error << std::endl;
        throw std::runtime_error(error);
    }
    std::cout << "Successfully configured serial port" << std::endl;
}
void hpSerialRead::closePort(){
    close(serial_fd);
}
void hpSerialRead::hpSetupCflag(struct termios *tty){
    tty->c_cflag &= ~PARENB; //disable parity bit 
    tty->c_cflag &= ~CSTOPB; //set stop bits to 1
    tty->c_cflag &= ~CSIZE; //clear the size bitmask so you can set the right value for hanport we want 8 bits for the message here
    tty->c_cflag |= CS8;  //set data bits per message to 8
    tty->c_cflag &= ~CRTSCTS; //Disable RTC/CTS hardware flow control
    tty->c_cflag |= CREAD | CLOCAL; // enabeling CREAD makes the port able to read serial data? CLOCAL turn of controllines for modem translations?

}
void hpSerialRead::hpSetupLflag(struct termios *tty){
    tty->c_lflag &= ~ICANON; //disable icanonical mode so that we dont process data line på line and so that we gather everything sent and do not treat special characters differently, in canonical inpus is prossesed when a new line is recieved
    tty->c_lflag &= ~ISIG; //Disable interupts quit and suspends like ctrl-C
}
void hpSerialRead::hpSetupIflag(struct termios *tty){
    tty->c_iflag &= ~(IXON | IXOFF | IXANY);                                      // disable flow controls allow for raw serial input to be read.
    tty->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); //Disable any special handling of recived bytes    
}
void hpSerialRead::hpSetupCc(struct termios *tty){
    tty->c_cc[VTIME] = 10;
    tty->c_cc[VMIN] = 1;
}
std::vector<uint8_t> hpSerialRead::hpRead()
{
    //buffer to read chunks of bytes into.
    uint8_t read_buff[256];
    //condition if we are reading a message or not
    bool reading_message = false;
    bool end_found = false;
    int crc_bytes_read = 0;
    //protocol sas 2 bytes but im not getting the full message when just extracting 4 bytes
    const int CRC_SIZE = 4;
    std::vector<uint8_t> message;
    //continously read from the serial buffer
    while(true)
    {   
        //read chunks from the serial buffer into read buffer
        int num_of_bytes = read(this->serial_fd, read_buff, sizeof(read_buff));
        //if no bytes are read continue reading
        if(num_of_bytes <= 0 ) 
        {
           continue; 
        }
        //for each chunk read, iterate through and look for start and end positions and push back all chunks read into message buffer if its reading hpmessge
        for (int i = 0; i < num_of_bytes; i++) 
        {   //extract each byte read
            uint8_t byte = read_buff[i];
	    std::cout << byte;
            //compare to look for starting delimiter
            if(byte == '/')
            {  
                message.clear();
                message.push_back(byte);
                reading_message = true;
                end_found = false;
                crc_bytes_read = 0;
                continue;
            }
            //identify end of message marker and tell to start reading crc part
            if(byte == '!' && reading_message && !end_found)
            {   message.push_back(byte);
                end_found = true;
                crc_bytes_read = 0;
                continue;
            }
            //read crcpart and return the vector array
            if(end_found)
            {
                message.push_back(byte);
                crc_bytes_read++;
                if(crc_bytes_read == CRC_SIZE)
                {   std::cout << "\n";
                    return message;
                }
                continue;

            } 
            //add bytes within message to message array
            if(reading_message)
            {
                message.push_back(byte);
            }
        }
    }
}
