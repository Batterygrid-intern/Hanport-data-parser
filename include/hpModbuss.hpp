

// Simple wrapper class for a Modbus TCP server.
// This header declares hpModbuss which will try to use libmodbus when
// built with USE_LIBMODBUS. When libmodbus isn't available a lightweight
// stub implementation in the cpp file provides the same API for testing.

#pragma once

#include <cstdint>
#include <vector>
#include <map>
#include <string>

class hpModbuss {
public:
    // Construct with TCP port (default 1502 for non-privileged testing)
    explicit hpModbuss(uint16_t port = 1502);
    ~hpModbuss();

    // Start the modbus TCP server in a background thread.
    // Throws std::runtime_error on failure.
    void start();

    // Stop the server and join the thread. Safe to call multiple times.
    void stop();

    // Set a single holding register value (address is 0-based)
    void set_holding_register(uint16_t address, uint16_t value);

    // Set multiple registers starting at address
    void set_holding_registers(uint16_t address, const std::vector<uint16_t>& values);

    // Read a single holding register (throws if not present)
    uint16_t get_holding_register(uint16_t address) const;

    // Return a copy of internal registers map (address -> value)
    std::map<uint16_t, uint16_t> snapshot_holding_registers() const;

    // Return human-readable status
    std::string status() const;

private:
    // Pimpl-like private data to keep implementation details out of header
    struct Impl;
    Impl* pimpl_;
};
