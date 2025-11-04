# Swedish P1(rj12) Hanport data.

This is a library for \*data extraction(propably not extraction), data validation, and data parsing for a swedish electricity meter.



# Product design

1. Library for reading data and using the gpio interface on the raspberry pi
2. Hanport library for validating,parsing, and structuring(to desired format)the data collected by the raspberry pi
3. CRC library for the alrogirthms? or part of the hanport library?
4. MQTT library for data transmittion the data
5. Modbus library for data transmittion

# hanport class
after reading the data from the hanport this class will be used to validate and parse the data.

# mqtt class
used to transmitt data.

# Things to look at
- interfacing
# Hanport data parser + Modbus bridge

This project reads Hanport/P1 style meter messages from a serial device, validates and parses the data into a structured `hpData` object and exposes selected fields over a Modbus TCP server.

The repository now contains:

- reader and validator: `hpMessageValidator`
- parser: `hpDataParser` -> fills `hpData` (floats for energy, power, voltage, current, ...)
- serial helper: `hpSerialRead` (wraps CppLinuxSerial)
- Modbus TCP server wrapper: `hpModbuss` (uses libmodbus)
- an example CLI: `bin/modbus_example`
- the main program: `bin/test` (starts modbus server and updates registers)
- integration test: `bin/modbus_integration_test` (gtest)

## Prerequisites (Raspberry Pi)

- Raspberry Pi OS (or Debian-based)
- Build tools and headers: `build-essential`, `cmake`, `pkg-config`, `libmodbus-dev`
- Optional test client: `mbpoll` (or use the bundled `modbus_example`)
- Access to the serial device (add the running user to `dialout` group)

Install example:

```bash
sudo apt update
sudo apt install -y build-essential cmake pkg-config libmodbus-dev
sudo usermod -aG dialout $USER
```

If you use the Pi UART (ttyAMA0 / serial0) ensure the serial console is disabled (`raspi-config`) and `enable_uart=1` in `/boot/config.txt`.

## Build

From the project root:

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

Artifacts are placed under `bin/`:

- `bin/test` — the main program (starts Modbus server and reads serial)
- `bin/modbus_example` — minimal example that demonstrates server+client on the Pi
- `bin/modbus_integration_test` — gtest integration test

## Run (manual)

Start the program (use a non‑privileged port like 1502):

```bash
./bin/test
```

Read registers using a Modbus TCP client (for example `mbpoll`) — registers contain 32‑bit IEEE floats split into two 16‑bit registers (high-word first):

```bash
# read 2 registers starting at address 2 (one float)
mbpoll -m tcp -a 1 -r 2 -c 2 127.0.0.1:1502
```

## Register mapping (holding registers, base = 0)

Each `float` in `hpData` is placed as two registers (big-endian word order). Current layout (starting at register 0):

- 0..1: time_stamp
- 2..3: active_enery_import_total
- 4..5: active_energy_export_total
- 6..7: reactive_energy_import_total
- 8..9: reactive_energy_export_total
- 10..11: active_power_import
- 12..13: active_power_export
- 14..15: reactive_power_import
- 16..17: reactive_power_export
- 18..19: l1_active_power_import
- 20..21: l1_active_power_export
- 22..23: l2_active_power_import
- 24..25: l2_active_power_export
- 26..27: l3_active_power_import
- 28..29: l3_active_power_export
- 30..31: l1_reactive_power_import
- 32..33: l1_reactive_power_export
- 34..35: l2_reactive_power_import
- 36..37: l2_reactive_power_export
- 38..39: l3_reactive_power_import
- 40..41: l3_reactive_power_export
- 42..43: l1_voltage_rms
- 44..45: l2_voltage_rms
- 46..47: l3_voltage_rms
- 48..49: l1_current_rms
- 50..51: l2_current_rms
- 52..53: l3_current_rms

Adjust clients to interpret two consecutive 16‑bit registers as one 32‑bit IEEE float in the same word order.

## Tests

The project includes a gtest integration test `modbus_integration_test` that parses a sample message, starts the server on a test port, writes registers and reads them back with a libmodbus client.

Run the test binary directly:

```bash
./bin/modbus_integration_test
```

If you prefer `ctest`, add `enable_testing()` to the top-level `CMakeLists.txt` (I can add this for you) then run:

```bash
ctest --output-on-failure
```

## Run as a service (systemd)

Create a unit file `/etc/systemd/system/hanport.service` and adjust paths/user:

```
[Unit]
Description=Hanport Modbus/Serial bridge
After=network.target

[Service]
Type=simple
User=pi
Group=pi
WorkingDirectory=/home/pi/ws/bgs-ws/Hanport-data-parser
ExecStart=/home/pi/ws/bgs-ws/Hanport-data-parser/bin/test
Restart=on-failure
RestartSec=5

[Install]
WantedBy=multi-user.target
```

Enable and start with:

```bash
sudo systemctl daemon-reload
sudo systemctl enable --now hanport.service
sudo journalctl -fu hanport.service
```

## Notes and next steps

- Word/byte ordering: the code writes floats as (high‑word, low‑word). If your clients expect a different order (low‑high or byte swapped) we can add runtime configuration to choose the ordering.
- If you prefer scaled integers (for better PLC compatibility) we can expose values multiplied by a scale factor instead of IEEE floats.
- The libmodbus mapping currently reserves 1000 registers — if you need more or different spaces (coils, discrete inputs) we can add them.

If you want I can add `enable_testing()` to CMake, include a systemd unit file in the repo under `extras/`, or make the register layout configurable.

## Client testing (from your PC)

You can test the Modbus server running on the Raspberry Pi from your PC using either the provided Python client or the C++ client binary.

Python client (recommended quick test)

- Requirements: Python 3 and `pymodbus`.

```bash
pip3 install pymodbus
# Example: read 2 registers starting at address 2 from a Pi at 192.168.1.10
python3 scripts/modbus_client.py --host 192.168.1.10 --port 1502 --start 2 --count 2
```

Output will list each register and, for register pairs, decode them as 32-bit IEEE floats (high-word then low-word). Example:

```
Read 2 registers starting at 2
R[2] = 16452
R[3] = 49396
Float @ 2..3 = 6678.39
```

C++ client (no Python dependency)

Build step (already part of normal project build):

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

Run the C++ client binary on any machine with libmodbus (or copy the binary from the Pi):

```bash
./bin/modbus_client 192.168.1.10 1502 2 2
```

This prints the raw 16-bit register values and decodes pairs as floats similarly to the Python client.

Notes
- The clients assume the server uses Modbus unit id 1. If you change the unit id in the server, update the clients accordingly.
- The float decoding uses high‑word then low‑word order (the same order used by `hpModbuss`). If your Modbus consumer expects a different word order, we can add a command-line option to the clients to swap words/bytes.
- If you cannot install `pymodbus` on your PC, I can add a tiny pure-socket Python client to the repo that performs the same read (no external dependency).

## Firewall & network guidance

If your Raspberry Pi is connected to a local network and you want to allow remote clients (PCs, HMIs, SCADA) to connect to the Modbus TCP server, you may need to open the Modbus port in the Pi's firewall. Below are example commands for common setups.

Prefer using a non‑privileged port (default in this project: 1502). If you bind to port 502 (the Modbus standard), you'll need root or CAP_NET_BIND_SERVICE.

UFW (Ubuntu / Raspberry Pi OS with ufw)

```bash
sudo ufw allow 1502/tcp        # allow Modbus TCP on port 1502
sudo ufw status verbose
```

iptables (legacy)

```bash
sudo iptables -A INPUT -p tcp --dport 1502 -m conntrack --ctstate NEW,ESTABLISHED -j ACCEPT
# Save rules persistently (Debian/Ubuntu)
sudo apt install -y iptables-persistent
sudo netfilter-persistent save
```

nftables (modern Linux)

```bash
# add a rule to the inet filter table (adjust to your nftables config)
sudo nft add rule inet filter input tcp dport 1502 ct state { new, established } accept
```

Router / NAT

- If you want to access the Pi from outside your LAN you will need to add a port-forward on your router from the WAN to the Pi's LAN IP. Be careful exposing Modbus to the public internet — prefer a VPN.

Quick connectivity tests

- Check port reachability from your PC:

```bash
nc -vz 192.168.1.10 1502
# or use telnet/nc to test TCP connect
```

- Use the Python client or C++ client to perform an actual Modbus read (examples above). If the connect fails, verify firewall rules, Pi IP, and that the server is running.

Security notes

- Do not expose Modbus directly to the public internet without a VPN, firewall rules, and proper authentication layers — Modbus itself does not provide robust authentication or encryption.
- If you must expose the service, consider tunneling over SSH or using a VPN (WireGuard/OpenVPN).






