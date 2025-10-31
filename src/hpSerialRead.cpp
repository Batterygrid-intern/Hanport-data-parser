#include "hpSerialRead.hpp"
#include <iostream>
#include <cstdint>


hpSerialRead::hpSerialRead(char* serial_port_path){
    this->serial_port_path = serial_port_path;
}
hpSerialRead::~hpSerialRead(){
    closePort();
} 
void hpSerialRead::openPort(){
    this->serial_fd = open(this->serial_port_path,O_RDONLY);
    if(this->serial_fd < 0){
        throw std::runtime_error("failed to open file descriptor");
    }
    setupPort();
}
void hpSerialRead::setupPort(){
    if(tcgetattr(this->serial_fd,&this->tty) != 0){
        throw std::runtime_error("failed to get attributes for serial port");
    }
    hpSetupCflag(&this->tty);
    hpSetupLflag(&this->tty);
    hpSetupIflag(&this->tty);
    hpSetupCc(&this->tty);

    cfsetispeed(&this->tty,BAUD_RATE);

    if(tcsetattr(this->serial_fd,TCSANOW,&this->tty) != 0){
        throw std::runtime_error("Failed to set attribute to serial port");
    }
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
    tty->c_cc[VTIME] = 255;
    tty->c_cc[VMIN] = 0;
}
std::vector<uint8_t> hpSerialRead::hpRead(){
char read_buff[1000];
   // num_of_bytes will be how many bytes that is read, 0 if no bytes recived and negativ if error occured
   int num_of_bytes = read(this->serial_fd,&read_buff,sizeof(read_buff));
   printf("%s",read_buff);
}

