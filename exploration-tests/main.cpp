#include <iostream> //std I/O
#include <fstream> //read/write from file stream
#include <iomanip> //input output manipulator
#include <string>

uint16_t crc16_ibm(const uint8_t *data, size_t length); 
void read_hanport_message(std::ifstream &file,uint8_t* data_buffer,size_t max_size);
void read_hanport_message_v(std::ifstream &file, std::vector<uint8_t> &data_buffer);
uint16_t crc16_ibm_v(std::vector<uint8_t> message, size_t length);
int main(int argc, char** argv){

    //std::ifstream file("exampledata/meter_data.bin", std::ios::binary | std::ios::ate);
    std::ifstream file("exampledata/meter_data.bin", std::ios::binary);
    if (!file.is_open())
    {
        std::perror("failed to open file");
        return 1;
    }

    //get the size of the file 
    //const size_t FILE_SIZE = file.tellg();
    //allocate memory to the databuffer to store the message from the file
    //uint8_t* data_buffer = new uint8_t[FILE_SIZE];
    //move the cursor to the first line of the file to start reading
    //file.seekg(0);
    std::vector<uint8_t> data_buffer;

    //read the whole file and store in data_buffer
     
    read_hanport_message_v(file,data_buffer);
    file.close();
    size_t exclamation_pos = 0;
    for(size_t i = 0 ; i < data_buffer.size(); i++){
         if(data_buffer[i] == '!'){
            exclamation_pos = i;
            break;
         } 
    }
    //add crc sep code
    std::string crc_string;
    for(size_t i = exclamation_pos + 1; i < data_buffer.size() ;i++){
        if(data_buffer[i] != '\r' && data_buffer[i] != '\n'){
            crc_string += (char)data_buffer[i];
        }
    }
    std::cout << crc_string << '\n';
    //int recieved_crc = std::stoi(crc_string, nullptr,16); 

    std::vector<uint8_t> hanport_message(data_buffer.begin(),data_buffer.begin() + exclamation_pos +1);


    for(size_t i : hanport_message){
        std::cout << (char)i ;
    }
    std::cout << '\n';

    std::cout << exclamation_pos << "\n";
    std::cout << "\n";
    std::cout << data_buffer.size() << std::endl;
    //read_hanport_message(file,data_buffer,FILE_SIZE);

    //separate the message and the crc from the databuffer, message from / to ! crc the remainder 
    //separet the 2 by finding position
    //uint16_t recivedCRC = 0x7945;
    //std::cout << "data_buffer: "<< FILE_SIZE << std::endl;
    // calculate the crc from the message store in the buffer / to ! everything included
    uint16_t calculatedCRC = crc16_ibm_v(hanport_message, hanport_message.size());
   // uint16_t calculatedCRC = crc16_ibm(data_buffer, FILE_SIZE);

    //printout result
    std::cout << "Result of CRC calculation: " << std::hex << calculatedCRC << "\n";
   // std::cout << "Recieved CRC from message: " << std::hex <<recieved_crc << "\n";
    //validate crc, see if the crc calculated from your message is the same as the one extracted from the databuffer

    //free memory of the databuffer
    //delete[] data_buffer; 

    return 0;
}
//read data message
void read_hanport_message(std::ifstream &file, uint8_t* data_buffer,size_t max_size){
    char c;
    for(size_t i = 0; i < max_size; i++){
        file.get(c);
        data_buffer[i] = c;
    }
}

void read_hanport_message_v(std::ifstream &file, std::vector<uint8_t> &data_buffer){
    char c;
    size_t i = 0;
    while(file.get(c)){
        data_buffer.push_back(c);
        i++;
    }
    /*for(size_t i = 0; i < max_size; i++){
        file.get(c);
        data_buffer.push_back(c);
    }*/
}


uint16_t crc16_ibm(const uint8_t *data, size_t length) {
    uint16_t crc = 0x0000;  // IBM startar med 0x0000
    
    for (size_t i = 0; i < length; i++) {
        crc ^= data[i];

        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}


uint16_t crc16_ibm_v(std::vector<uint8_t> message, size_t length) {
    uint16_t crc = 0x0000;  // IBM startar med 0x0000
    
    for (size_t i = 0; i < length; i++) {
        crc ^= message[i];

        for (int j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}
