#include "hpModbuss.hpp"
#include <iostream>
#include <thread>
#include <chrono>

#ifdef LIBMODBUS_FOUND
#include <modbus/modbus.h>
#endif

int main(){
    const uint16_t port = 1502;
    try {
        hpModbuss server(port);
        // set register 0 to 0x1234 before start (server will copy on reply)
        server.set_holding_register(0, 0x1234);
        server.start();

        // give server a short moment to start listening
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

#ifdef LIBMODBUS_FOUND
        // create a client and read register 0
        modbus_t* ctx = modbus_new_tcp("127.0.0.1", port);
        if (!ctx) {
            std::cerr << "Failed to create libmodbus client context\n";
            server.stop();
            return 2;
        }
        if (modbus_connect(ctx) == -1) {
            std::cerr << "Failed to connect modbus client\n";
            modbus_free(ctx);
            server.stop();
            return 3;
        }

        uint16_t regs[1] = {0};
        int rc = modbus_read_registers(ctx, 0, 1, regs);
        if (rc == 1) {
            std::cout << "Read register 0 = 0x" << std::hex << regs[0] << std::dec << "\n";
        } else {
            std::cerr << "modbus_read_registers failed rc=" << rc << " err=" << modbus_strerror(errno) << "\n";
        }
        modbus_close(ctx);
        modbus_free(ctx);
#else
        std::cout << "libmodbus not available — client test skipped" << std::endl;
#endif

        server.stop();
    } catch (const std::exception &e){
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
