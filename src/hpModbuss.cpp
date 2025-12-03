// Implementation of hpModbuss wrapper using libmodbus

#include "hpModbuss.hpp"

#include "hpData.hpp"

#include <thread>
#include <mutex>
#include <atomic>
#include <stdexcept>
#include <chrono>
#include <cstring>
#include <iostream>
#include <ostream>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#ifdef LIBMODBUS_FOUND
#include <modbus/modbus.h>
#endif

#define NB_CONNECTION 10

struct hpModbuss::Impl {
    uint16_t port;
    std::thread server_thread;
    mutable std::mutex mtx;
    std::atomic<bool> running{false};

#ifdef LIBMODBUS_FOUND
    modbus_t* ctx = nullptr;
    modbus_mapping_t* mb_mapping = nullptr;
    int server_socket = -1;
#endif

    // local copy of registers if libmodbus not present or for snapshotting
    std::map<uint16_t, uint16_t> registers;
    Impl(uint16_t p): port(p) {}
};

hpModbuss::hpModbuss(uint16_t port): pimpl_(new Impl(port)) {}

hpModbuss::~hpModbuss(){
    stop();
    delete pimpl_;
}

void hpModbuss::start(){
    std::lock_guard<std::mutex> lk(pimpl_->mtx);
    if (pimpl_->running) return;
    pimpl_->running = true;

#ifdef LIBMODBUS_FOUND
    // create TCP context for all interfaces (NULL) and requested port
    pimpl_->ctx = modbus_new_tcp(NULL, pimpl_->port);
    if (!pimpl_->ctx) {
        pimpl_->running = false;
        throw std::runtime_error("Failed to create libmodbus context");
    }

    // create mapping with room for 1000 registers (adjust as needed)
    const int nb_reg = 1000;
    pimpl_->mb_mapping = modbus_mapping_new(0, 0, nb_reg, 0);
    if (!pimpl_->mb_mapping) {
        modbus_free(pimpl_->ctx);
        pimpl_->ctx = nullptr;
        pimpl_->running = false;
        throw std::runtime_error("Failed to allocate modbus mapping");
    }

    // copy initial registers from map to mapping (all zeros initially)
    // listen for connection
    pimpl_->server_socket = modbus_tcp_listen(pimpl_->ctx, NB_CONNECTION);
    if (pimpl_->server_socket == -1) {
        modbus_mapping_free(pimpl_->mb_mapping);
        modbus_free(pimpl_->ctx);
        pimpl_->mb_mapping = nullptr; pimpl_->ctx = nullptr;
        pimpl_->running = false;
        throw std::runtime_error("Failed to listen on modbus TCP socket");
    }

    // server thread starts server and handles queries
    pimpl_->server_thread = std::thread([this]() {
        //@ refset master of all sockets were monitoring represents all the connected sockets
        fd_set refset; //data structure that works as bitarray each bit represents a socket
        fd_set rdset; // copy of my refset which will be modified by select() to show which sockets has data ready

        int fdmax; //maximum file descriptor number to increment sockets to
        int rc;
        int master_socket;

        //buffer to store the query from the modbus client.
        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];

        FD_ZERO(&refset);//clear the refset array from al sockets
        FD_SET(pimpl_->server_socket, &refset);//add server sockets to refset

        //file descriptors are just integers assigend by the os. stdin = 0 stdout = 1 stderr = 2 first socket could be = 3
        //fdmax will be used for select to know how many sockets to check
        fdmax = pimpl_->server_socket; //assign a variable with the highest value socket so that we know how many connections there are
        while (pimpl_->running) {
            //assigne a copy of the refset to rdset to pass to select
            rdset = refset;
            //timeval struct provides a timout for select()
            //allows the thread to wake up periodically to check if pimple_>running is false and terminate gracefully
            //check for socket activity if there is new connections or data
            struct timeval tv;
            tv.tv_sec = 1;
            tv.tv_usec = 0;
            //select() system call to monitor multiple sockets. which has data ready to read, accept connections
            int ret = select(fdmax +1, &rdset, nullptr, nullptr, &tv);
            if (ret == -1) {
                if (!pimpl_->running) break;
                perror("Server select() failure.");
                break;
            }
            if (ret == 0) {
                continue;
            }
            for (master_socket = 0; master_socket <= fdmax; master_socket++) {
                if (!FD_ISSET(master_socket, &rdset)) {
                    continue;
                }
                if (master_socket == pimpl_->server_socket) {
                    socklen_t addrlen;
                    struct sockaddr_in clientaddr;
                    addrlen = sizeof(clientaddr);
                    memset(&clientaddr, 0, sizeof(clientaddr));
                    int newfd = accept(pimpl_->server_socket, reinterpret_cast<struct sockaddr *>(&clientaddr), &addrlen);
                    if (newfd == -1) {
                        perror("Server accept() failure.");
                    }
                    else {
                        FD_SET(newfd, &refset);
                        if (newfd > fdmax) {
                            fdmax = newfd;
                        }
                        char client_ip[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip, INET_ADDRSTRLEN);
                        std::cout << "New connection from " << client_ip << ":" << ntohs(clientaddr.sin_port) << std::endl;
                    }
                }
                else {
                    modbus_set_socket(pimpl_->ctx, master_socket);
                    rc = modbus_receive(pimpl_->ctx,query);
                    if (rc > 0) {
                        modbus_reply(pimpl_->ctx,query,rc,pimpl_->mb_mapping);
                    }
                    else if (rc == -1) {
                        std::cout << "Connection closed on socket " << master_socket << std::endl;
                        close(master_socket);
                        FD_CLR(master_socket, &refset);
                        if (master_socket == fdmax) {
                            fdmax--;
                        }
                    }
                }
            }
        }
    });

#else
    // If libmodbus isn't found, throw — top-level CMake currently requires libmodbus.
    pimpl_->running = false;
    throw std::runtime_error("libmodbus not available at runtime — build must find libmodbus");
#endif
}

void hpModbuss::stop(){
    {
        std::lock_guard<std::mutex> lk(pimpl_->mtx);
        if (!pimpl_->running) return;
        pimpl_->running = false;
    }
#ifdef LIBMODBUS_FOUND
    // closing server socket by freeing context will unblock accept/receive
    if (pimpl_->ctx) {
        // close the listening socket if open
        if (pimpl_->server_socket != -1) {
            // shutdown the socket to break out of accept
            ::shutdown(pimpl_->server_socket, SHUT_RDWR);
            ::close(pimpl_->server_socket);
            pimpl_->server_socket = -1;
        }
        // modbus_close will close client sockets
        modbus_close(pimpl_->ctx);
    }

    if (pimpl_->server_thread.joinable()) pimpl_->server_thread.join();

    if (pimpl_->mb_mapping) {
        modbus_mapping_free(pimpl_->mb_mapping);
        pimpl_->mb_mapping = nullptr;
    }
    if (pimpl_->ctx) {
        modbus_free(pimpl_->ctx);
        pimpl_->ctx = nullptr;
    }
#endif
}

void hpModbuss::set_holding_register(uint16_t address, uint16_t value){
    std::lock_guard<std::mutex> lk(pimpl_->mtx);
    pimpl_->registers[address] = value;
#ifdef LIBMODBUS_FOUND
    // If mapping exists, write into mapping too
    if (pimpl_->mb_mapping && address < 1000) {
        pimpl_->mb_mapping->tab_registers[address] = value;
    }
#endif
}

void hpModbuss::set_holding_registers(uint16_t address, const std::vector<uint16_t>& values){
    std::lock_guard<std::mutex> lk(pimpl_->mtx);
    for (size_t i=0;i<values.size();++i){
        pimpl_->registers[address + i] = values[i];
        #ifdef LIBMODBUS_FOUND
        if (pimpl_->mb_mapping && (address + i) < 1000) pimpl_->mb_mapping->tab_registers[address + i] = values[i];
        #endif
    }
}

// static helper: float -> two registers (small endian word order)
std::vector<uint16_t> hpModbuss::float_to_regs(float f){
    uint32_t u = 0;
    static_assert(sizeof(float) == 4, "float must be 32-bit");
    std::memcpy(&u, &f, sizeof(u));
    uint16_t high = static_cast<uint16_t>((u >> 16) & 0xFFFF);
    uint16_t low = static_cast<uint16_t>(u & 0xFFFF);
    return std::vector<uint16_t>{low, high};
}
//static helper
std::vector<uint16_t> hpModbuss::int_to_regs(int i){
    uint16_t high = static_cast<uint16_t>((i >> 16) & 0xFFFF);
    uint16_t low = static_cast<uint16_t>(i & 0xFFFF);
    return std::vector<uint16_t>{low, high};
}

// Build register vector from hpData
std::vector<uint16_t> hpModbuss::make_regs(const hpData &d) const {
    std::vector<uint16_t> registers;
    auto append_float = [&](const float v){
        auto parts = float_to_regs(v);
        registers.insert(registers.end(), parts.begin(), parts.end());
    };
    auto append_int = [&](const int v) {
        auto parts = int_to_regs(v);
        registers.insert(registers.end(), parts.begin(), parts.end());
    };
    append_int(d.heartbeat);
    append_float(d.active_energy_import_total);
    append_float(d.active_energy_export_total);
    append_float(d.reactive_energy_import_total);
    append_float(d.reactive_energy_export_total);
    append_int(static_cast<int>(d.active_power_import * 1000));
    append_int(static_cast<int>(d.active_power_export * 1000));
    append_int(static_cast<int>(d.reactive_power_import * 1000));
    append_int(static_cast<int>(d.reactive_power_export * 1000));
    // per-phase active power
    append_int(static_cast<int>(d.l1_active_power_import * 1000));
    append_int(static_cast<int>(d.l1_active_power_export * 1000));
    append_int(static_cast<int>(d.l2_active_power_import * 1000));
    append_int(static_cast<int>(d.l2_active_power_export * 1000));
    append_int(static_cast<int>(d.l3_active_power_import * 1000));
    append_int(static_cast<int>(d.l3_active_power_export * 1000));
    // per-phase reactive power
    append_int(static_cast<int>(d.l1_reactive_power_import * 1000));
    append_int(static_cast<int>(d.l1_reactive_power_export * 1000));
    append_int(static_cast<int>(d.l2_reactive_power_import * 1000));
    append_int(static_cast<int>(d.l2_reactive_power_export * 1000));
    append_int(static_cast<int>(d.l3_reactive_power_import * 1000));
    append_int(static_cast<int>(d.l3_reactive_power_export * 1000));
    // voltages
    append_float(d.l1_voltage_rms);
    append_float(d.l2_voltage_rms);
    append_float(d.l3_voltage_rms);
    // currents
    append_float(d.l1_current_rms);
    append_float(d.l2_current_rms);
    append_float(d.l3_current_rms);
    return registers;
}

// Convenience: set registers directly from hpData
void hpModbuss::set_from_hpData(const hpData &d, uint16_t start_address){
    auto regs = make_regs(d);
    set_holding_registers(start_address, regs);
}

uint16_t hpModbuss::get_holding_register(uint16_t address) const{
    std::lock_guard<std::mutex> lk(pimpl_->mtx);
    auto it = pimpl_->registers.find(address);
    if (it == pimpl_->registers.end()) throw std::out_of_range("register not set");
    return it->second;
}

std::map<uint16_t, uint16_t> hpModbuss::snapshot_holding_registers() const{
    std::lock_guard<std::mutex> lk(pimpl_->mtx);
    return pimpl_->registers;
}

std::string hpModbuss::status() const{
    std::lock_guard<std::mutex> lk(pimpl_->mtx);
    return pimpl_->running ? "running" : "stopped";
}



