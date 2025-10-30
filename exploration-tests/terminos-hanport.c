#include <stdio.h> //standard input output
#include <string.h> //string functionalities

#include <termios.h> //Configuration for serial asyncronous port:
#include <unistd.h> //for open write read functions and other defines
#include <fcntl.h> // to control file descriptors for reading or writing mode etc when creating a descriptor with open
#include <errno.h> // handle errors and indicate on what when wrong
#include <sys/ioctl.h>


//set baud rate to B115200 (defined in terminos.h)
#define BAUD_RATE B115200
//define path to serial port
#define SERIAL_PORT_PATH "/dev/ttyAMA0"
//need to set partity to NONE
//set stopbit to one
//bytesize eightbits
//timout 30?
//rtscts=False
//dsrdtr=False
//xonxoff=False

//need to read everything raw need to get the whole message
void hanport_setup_cflag(struct termios *tty);
void hanport_setup_lflag(struct termios *tty);
void hanport_setup_iflag(struct termios *tty);
void hanport_setup_cc(struct termios *tty);
int main(){
    //need to open serial port for reading only :W
    int serial_port = open(SERIAL_PORT_PATH,O_RDONLY);
    //check that the file opened correctly
    if(serial_port < 0 ){
        //use errno to get the error code and get the corresponding error message printed to that code.
        printf("Error %i from trying to open serial port: %s\n",errno, strerror(errno));
        return 1;
    }

    //create termios struct to configure settings of the serial port
    struct termios tty;
    //read existing settings of the serial_port file descriptor and saves the min the tty structure need to do this before we can set new values 
    if(tcgetattr(serial_port,&tty) != 0){
        printf("Error %i from tcgetattr: %s\n",errno,strerror(errno));
        return 1;
    }
    //setup hardware and sowftare configurations for the hanport how to decode , transform and handle the data sent from the hanport
    hanport_setup_cflag(&tty);
    hanport_setup_lflag(&tty);
    hanport_setup_iflag(&tty);
    hanport_setup_cc(&tty);
    //set the baudrate used by the hanport to interperate the data
    cfsetispeed(&tty,BAUD_RATE);
    //save the changes made above to the struct (TCSANOW saves the changes imidietly) and changet the terminal port that is reading serial data
    if(tcsetattr(serial_port,TCSANOW,&tty)!= 0){
        printf("Error %i from tcsetattr: %s\n",errno,strerror(errno));
    }
   //buffer to read into  
   char read_buff[1000];
   // num_of_bytes will be how many bytes that is read, 0 if no bytes recived and negativ if error occured
   int num_of_bytes = read(serial_port,&read_buff,sizeof(read_buff));
   printf("%s",read_buff);
   close(serial_port);
   return 0;
}
// Configure hardware-level UART settings for the serial port.
// Sets how the UART interprets incoming electrical signals and frames them into bytes.
// Controls: number of data bits, parity, stop bits, receiver enable, and ignoring modem control lines.
// The UART decodes the electrical signals into bytes and stores them in the OS input buffer.
void hanport_setup_cflag(struct termios *tty){
    //disable parity bit 
    tty->c_cflag &= ~PARENB; //disable parity bit 
    tty->c_cflag &= ~CSTOPB; //set stop bits to 1
    tty->c_cflag &= ~CSIZE; //clear the size bitmask so you can set the right value for hanport we want 8 bits for the message here
    tty->c_cflag |= CS8;  //set data bits per message to 8
    tty->c_cflag &= ~CRTSCTS; //Disable RTC/CTS hardware flow control
    tty->c_cflag |= CREAD | CLOCAL; // enabeling CREAD makes the port able to read serial data? CLOCAL turn of controllines for modem translations?

}
//local modes, software level terminal behavior how does the software handle the input data as raw bytes or as characters with line feeds and controll characters
//electrical signals -> decoded by uart(cflag iflag?) -> sent to os buffer and with lflag we decide how the kernel should interpret the bytes 
void hanport_setup_lflag(struct termios *tty){
    tty->c_lflag &= ~ICANON; //disable icanonical mode so that we dont process data line på line and so that we gather everything sent and do not treat special characters differently, in canonical inpus is prossesed when a new line is recieved
    tty->c_lflag &= ~ISIG; //Disable interupts quit and suspends like ctrl-C
}
//how incoming data is handeled
void hanport_setup_iflag(struct termios *tty){
    tty->c_iflag &= ~(IXON | IXOFF | IXANY); //disable flow controls allow for raw serial input to be read.
    tty->c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL); //Disable any special handling of recived bytes
}
//set max time for message recival?
//wait max for 30 sec before for returning data to buffer and atleast 0 bits befor returning
void hanport_setup_cc(struct termios *tty){
    tty->c_cc[VTIME] = 255;
    tty->c_cc[VMIN] = 0;
}

