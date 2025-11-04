#include <iostream>
#include <vector>
#include <cstdint>
#include <cstring>
#include <cstdlib>
#include <modbus/modbus.h>

static float regs_to_float(uint16_t high, uint16_t low){
    uint32_t u = (static_cast<uint32_t>(high) << 16) | static_cast<uint32_t>(low);
    float f;
    std::memcpy(&f, &u, sizeof(u));
    return f;
}

int main(int argc, char** argv){
    if(argc < 5){
        std::cerr << "Usage: "<< argv[0] << " <host> <port> <start> <count>\n";
        return 1;
    }
    const char* host = argv[1];
    int port = std::atoi(argv[2]);
    int start = std::atoi(argv[3]);
    int count = std::atoi(argv[4]);

    modbus_t* ctx = modbus_new_tcp(host, port);
    if(!ctx){ std::cerr << "modbus_new_tcp failed\n"; return 2; }
    if(modbus_connect(ctx) == -1){ std::cerr << "modbus_connect failed\n"; modbus_free(ctx); return 3; }

    std::vector<uint16_t> regs(count);
    int rc = modbus_read_registers(ctx, start, count, regs.data());
    if(rc != count){ std::cerr << "modbus_read_registers failed rc="<<rc<<"\n"; modbus_close(ctx); modbus_free(ctx); return 4; }
    for(int i=0;i<count;++i){ std::cout<<"R["<<(start+i)<<"]="<<regs[i]<<"\n"; }
    for(int i=0;i+1<count;i+=2){
        float f = regs_to_float(regs[i], regs[i+1]);
        std::cout<<"Float @ "<<(start+i)<<".."<<(start+i+1)<<" = "<<f<<"\n";
    }

    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}
