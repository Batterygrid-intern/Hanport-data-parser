#include <iostream>
#include <CppLinuxSerial/SerialPort.hpp>
#include <cstdint>
#include <vector>
#include <thread>
#include <chrono>

using namespace mn::CppLinuxSerial;
int main(){
while(true){
    SerialPort serialPort("/dev/ttyAMA0", BaudRate::B_115200, NumDataBits::EIGHT, Parity::NONE, NumStopBits::ONE);
serialPort.SetTimeout(1000); // Kortare timeout: 1 sekund
serialPort.Open();

std::vector<uint8_t> buffer;
std::vector<uint8_t> chunk;

bool found_end = false;
int empty_reads = 0;

while(!found_end && empty_reads < 3) {
    chunk.clear();
    serialPort.ReadBinary(chunk);
    
    if(chunk.empty()) {
        empty_reads++;
        continue;
    }
    
    empty_reads = 0;
    buffer.insert(buffer.end(), chunk.begin(), chunk.end());
    
    // Leta efter '!' slutmarkör
    for(size_t i = 0; i < buffer.size(); i++) {
        if(buffer[i] == '!') {
            found_end = true;
            break;
        }
    }
}

for(auto &p : buffer) {
    std::cout << (char)p;
}
std::cout << std::endl;
std::this_thread::sleep_for(std::chrono::seconds(10));
}
return 0;
}
