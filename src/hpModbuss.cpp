// Implementation of hpModbuss wrapper using libmodbus

#include "hpModbuss.hpp"

#include "hpData.hpp"

#include <thread>
#include <mutex>
#include <atomic>
#include <stdexcept>
#include <chrono>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>

#ifdef LIBMODBUS_FOUND
#include <modbus/modbus.h>
#endif

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
    // listen
    pimpl_->server_socket = modbus_tcp_listen(pimpl_->ctx, 1);
    if (pimpl_->server_socket == -1) {
        modbus_mapping_free(pimpl_->mb_mapping);
        modbus_free(pimpl_->ctx);
        pimpl_->mb_mapping = nullptr; pimpl_->ctx = nullptr;
        pimpl_->running = false;
        throw std::runtime_error("Failed to listen on modbus TCP socket");
    }

    // server thread
    pimpl_->server_thread = std::thread([this](){
        // Accept and serve loop
        try {
            while (pimpl_->running) {
                int rc;
                // Wait for client
                int client_sock = modbus_tcp_accept(pimpl_->ctx, &pimpl_->server_socket);
                if (client_sock == -1) {
                    if (!pimpl_->running) break;
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }

                // Serve requests on this context until error or stop
                while (pimpl_->running) {
                    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
                    rc = modbus_receive(pimpl_->ctx, query);
                    if (rc > 0) {
                        // Before replying, copy protected registers from our map into mapping
                        {
                            std::lock_guard<std::mutex> lk(pimpl_->mtx);
                            for (const auto &kv : pimpl_->registers) {
                                uint16_t addr = kv.first;
                                if (addr < (uint16_t)1000) {
                                    pimpl_->mb_mapping->tab_registers[addr] = kv.second;
                                }
                            }
                        }
                        modbus_reply(pimpl_->ctx, query, rc, pimpl_->mb_mapping);
                    } else if (rc == -1) {
                        // client closed or error
                        break;
                    }
                }
                // close client socket and continue
            }
        } catch (...) {
            // swallow to ensure thread exits cleanly
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

// static helper: float -> two registers (big-endian word order)
std::vector<uint16_t> hpModbuss::float_to_regs(float f){
    uint32_t u = 0;
    static_assert(sizeof(float) == 4, "float must be 32-bit");
    std::memcpy(&u, &f, sizeof(u));
    uint16_t high = static_cast<uint16_t>((u >> 16) & 0xFFFF);
    uint16_t low = static_cast<uint16_t>(u & 0xFFFF);
    return std::vector<uint16_t>{high, low};
}

// Build register vector from hpData
std::vector<uint16_t> hpModbuss::make_regs(const hpData &d) const {
    std::vector<uint16_t> r;
    auto append_float = [&](float v){
        auto parts = float_to_regs(v);
        r.insert(r.end(), parts.begin(), parts.end());
    };
    append_float(d.heartbeat);
    append_float(d.active_energy_import_total);
    append_float(d.active_energy_export_total);
    append_float(d.reactive_energy_import_total);
    append_float(d.reactive_energy_export_total);
    append_float(d.active_power_import * 1000);
    append_float(d.active_power_export * 1000);
    append_float(d.reactive_power_import * 1000);
    append_float(d.reactive_power_export * 1000);
    // per-phase active power
    append_float(d.l1_active_power_import * 1000);
    append_float(d.l1_active_power_export * 1000);
    append_float(d.l2_active_power_import * 1000);
    append_float(d.l2_active_power_export * 1000);
    append_float(d.l3_active_power_import * 1000);
    append_float(d.l3_active_power_export * 1000);
    // per-phase reactive power
    append_float(d.l1_reactive_power_import * 1000);
    append_float(d.l1_reactive_power_export * 1000);
    append_float(d.l2_reactive_power_import * 1000);
    append_float(d.l2_reactive_power_export * 1000);
    append_float(d.l3_reactive_power_import * 1000);
    append_float(d.l3_reactive_power_export * 1000);
    // voltages
    append_float(d.l1_voltage_rms);
    append_float(d.l2_voltage_rms);
    append_float(d.l3_voltage_rms);
    // currents
    append_float(d.l1_current_rms);
    append_float(d.l2_current_rms);
    append_float(d.l3_current_rms);
    return r;
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
#include "hpModbuss.hpp"



