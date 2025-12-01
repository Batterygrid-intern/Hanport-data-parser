# Hanport Data Parser - Utvecklarguide

## Innehåll
- [Projektöversikt](#projektöversikt)
- [Arkitektur](#arkitektur)
- [Utvecklingsmiljö](#utvecklingsmiljö)
- [Bygga projektet](#bygga-projektet)
- [Kodstruktur](#kodstruktur)
- [Protokoll](#protokoll)
- [Bidra till projektet](#bidra-till-projektet)

## Projektöversikt

Hanport Data Parser är en C++-applikation som läser energimätardata från Hanport-enheter via seriell kommunikation, validerar och parsar data, och exponerar den via MQTT och Modbus TCP.

### Funktioner
- Seriell kommunikation (115200 baud, 8N1)
- CRC16-validering av inkommande meddelanden
- MQTT-publicering av mätdata
- Modbus TCP-server för realtidsdata
- INI-baserad konfiguration
- Daglig roterande loggning

### Teknisk stack
- **Språk**: C++17
- **Kompilator**: GCC/G++ 7.0 eller högre
- **Byggverktyg**: CMake 3.10+
- **Plattform**: Linux (primärt Raspberry Pi OS)
- **Externa bibliotek**:
  - **spdlog** - Loggning med daglig filrotation
  - **Eclipse Paho MQTT C++** - MQTT-klient
  - **libmodbus** - Modbus TCP-server

## Arkitektur

### Systemdiagram
```
┌─────────────────┐
│ Hanport Mätare  │
└────────┬────────┘
         │ Serial RS-232/485 (115200 baud, 8N1)
         │
┌────────▼────────────────────┐
│   hpSerialRead              │
│  - Läser rådata byte-för-byte│
│  - Bufferthantering          │
│  - Start/end delimiter sökning│
└────────┬────────────────────┘
         │ std::vector<uint8_t>
         │
┌────────▼────────────────────┐
│ HanportMessageValidator     │
│  - CRC16-beräkning          │
│  - Validering mot transmitted│
│  - Exception vid fel        │
└────────┬────────────────────┘
         │ std::vector<std::string>
         │
┌────────▼────────────────────┐
│   hpDataParser              │
│  - Parsar textfält          │
│  - String→float konvertering│
│  - Fyller hpData struktur   │
└────────┬────────────────────┘
         │
┌────────▼────────────────────┐
│      hpData                 │
│  - POD struktur (floats)    │
│  - Spänning, ström, effekt  │
│  - Heartbeat counter        │
└────┬─────────────────┬──────┘
     │                 │
┌────▼──────┐    ┌─────▼──────┐
│ HpMqttPub │    │ hpModbuss  │
│ (Paho)    │    │ (libmodbus)│
└───────────┘    └────────────┘
```

### Huvudkomponenter

| Komponent | Ansvar | Filer | Beroenden |
|-----------|--------|-------|-----------|
| `main.cpp` | Event loop, koordinering | src/main.cpp | spdlog, Config |
| `Config` | INI-parser | src/config.cpp, include/Config.hpp | - |
| `hpSerialRead` | POSIX serial I/O | src/hpSerialRead.cpp, include/hpSerialRead.hpp | termios |
| `HanportMessageValidator` | CRC16 validering | src/hpMessageValidator.cpp, include/hpMessageValidator.hpp | - |
| `hpDataParser` | Text→Data parsing | src/hpDataParser.cpp, include/hpDataParser.hpp | - |
| `hpData` | POD datamodell | include/hpData.hpp | - |
| `HpMqttPub` | MQTT publisher | src/hpMqttPub.cpp, include/hpMqttPub.hpp | Paho MQTT C++ |
| `hpModbuss` | Modbus TCP server | src/hpModbuss.cpp, include/hpModbuss.hpp | libmodbus |

## Utvecklingsmiljö

### Förutsättningar

**Raspberry Pi OS / Debian / Ubuntu:**
```bash
# Grundläggande verktyg
sudo apt update
sudo apt install -y build-essential git cmake pkg-config

# Externa bibliotek för utveckling
sudo apt install -y \
    libspdlog-dev \
    libpaho-mqtt-dev \
    libpaho-mqttpp-dev \
    libmodbus-dev
```

**Versioner som krävs:**
- CMake ≥ 3.10
- GCC/G++ ≥ 7.0 (för C++17 stöd)
- spdlog (header-only eller kompilerad)
- Paho MQTT C++ ≥ 1.0
- libmodbus ≥ 3.1

### Verifiera installation

```bash
# Kontrollera CMake version
cmake --version

# Kontrollera GCC version
g++ --version

# Kontrollera att bibliotek är installerade
pkg-config --modversion libmodbus
pkg-config --libs paho-mqttpp3
ldconfig -p | grep spdlog
```

### Klona repository

```bash
git clone https://github.com/your-repo/hanport-data-parser.git
cd hanport-data-parser
```

## Bygga projektet

### Build-typer

Projektet stöder två build-typer:

1. **Debug** - För utveckling
   - Debug-symboler aktiverade
   - Inga optimeringar
   - Dynamisk länkning till bibliotek

2. **Release** - För distribution
   - Optimeringar aktiverade (O2)
   - Statisk länkning av bibliotek
   - Strip:ad binär (mindre storlek)

### Debug Build (för utveckling)

```bash
# Skapa build-katalog
mkdir build
cd build

# Konfigurera för debug
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Kompilera
make -j$(nproc)

# Binären skapas som: build/hanport-data-parser
# Kör från build-katalogen:
./hanport-data-parser --config ../configs/app.ini
```

### Release Build (för produktion)

```bash
# Skapa separat release-katalog
mkdir build-release
cd build-release

# Konfigurera för release
cmake -DCMAKE_BUILD_TYPE=Release ..

# Kompilera
make -j$(nproc)

# Binären är nu statiskt länkad och optimerad
# Verifiera:
file hanport-data-parser
ldd hanport-data-parser  # Ska visa minimala dynamiska beroenden
```

**CMake konfigurerar automatiskt:**
- **Debug**: Dynamisk länkning (`-lpaho-mqttpp3`, etc.)
- **Release**: Statisk länkning (`libpaho-mqttpp3.a`, etc.)
- **Release**: Optimering (`-O2`)
- **Debug**: Debug-symboler (`-g`)

### Build-options

```bash
# Ändra installationssökväg
cmake -DCMAKE_INSTALL_PREFIX=/opt/hanport ..

# Verbose build (visa exakta kommandon)
make VERBOSE=1

# Parallell kompilering (4 trådar)
make -j4

# Rensa build
make clean

# Ombyggnad från scratch
rm -rf build && mkdir build && cd build && cmake .. && make
```

### Installera lokalt

```bash
cd build-release
sudo make install

# Installerar till /usr/local/bin som standard
# Eller till CMAKE_INSTALL_PREFIX om specificerat
```

### Cross-compilation för Raspberry Pi (från x86)

```bash
# Installera ARM cross-compiler
sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

# Skapa toolchain-fil
cat > toolchain-rpi.cmake << 'EOF'
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)
set(CMAKE_C_COMPILER aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER aarch64-linux-gnu-g++)
set(CMAKE_FIND_ROOT_PATH /usr/aarch64-linux-gnu)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
EOF

# Bygg för ARM
mkdir build-arm && cd build-arm
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-rpi.cmake \
      -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

## Kodstruktur

### Katalogstruktur

```
Hanport-data-parser/
├── src/                      # Källkodsfiler
│   ├── main.cpp              # Huvudprogram & event loop
│   ├── config.cpp            # INI-parser implementation
│   ├── hpSerialRead.cpp      # POSIX serial I/O
│   ├── hpMessageValidator.cpp # CRC16 validering
│   ├── hpDataParser.cpp      # Data parsing
│   ├── hpMqttPub.cpp         # MQTT client wrapper
│   └── hpModbuss.cpp         # Modbus TCP server wrapper
├── include/                  # Header-filer
│   ├── Config.hpp            # Config class
│   ├── hpSerialRead.hpp      # Serial reader class
│   ├── hpMessageValidator.hpp # Message validator class
│   ├── hpDataParser.hpp      # Data parser class
│   ├── hpData.hpp            # POD data structure
│   ├── hpMqttPub.hpp         # MQTT publisher class
│   └── hpModbuss.hpp         # Modbus server class
├── configs/                  # Konfiguration
│   └── app.ini.example       # Exempel-config
├── docs/                     # Dokumentation
│   ├── USER_GUIDE.md
│   ├── DEVELOPER_GUIDE.md
│   └── RELEASE_GUIDE.md
├── scripts/                  # Hjälpskript
│   ├── create_release.sh     # Skapa release-paket
│   └── install.sh            # Installationsskript
├── CMakeLists.txt            # CMake build-konfiguration
├── README.md
├── CHANGELOG.md
└── LICENSE
```

### Dataflöde i detalj

#### 1. Initialisering (main.cpp)

```cpp
// Läs konfiguration
Config cfg;
std::string err;
if (!cfg.loadFromFile(configFilePath, err)) {
    // Hantera fel
}

// Skapa daily rotating logger
auto logger = spdlog::daily_logger_mt(
    "hanport_logger",
    cfg.get("LOGGER", "PATH", "/var/log/hanport/hanport.log"),
    0, 0  // Rotera vid midnatt
);
logger->set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
logger->set_level(spdlog::level::debug);

// Initiera komponenter
hpSerialRead serial_reader;
serial_reader.openPort(SERIAL_PORT);

HpMqttPub mqtt_publisher(brokerAddress, clientId, site, deviceId);
mqtt_publisher.connect();

hpModbuss modbus_server(port);
modbus_server.start();
```

#### 2. Huvudloop

```cpp
hpData data_obj;
float heartbeat = 0.0f;

while (true) {
    heartbeat += 1.0f;
    data_obj.time_stamp = heartbeat;
    
    // Läs från serial
    std::vector<uint8_t> raw_message = serial_reader.hpRead();
    
    if (!raw_message.empty()) {
        try {
            // Validera
            HanportMessageValidator validator(raw_message);
            if (validator.get_calculated_crc() != validator.get_transmitted_crc()) {
                logger->error("CRC mismatch");
                continue;
            }
            
            // Parsa
            auto message_array = validator.message_to_string_arr();
            hpDataParser parser(message_array);
            parser.parse_message(data_obj);
            
            // Publicera
            mqtt_publisher.publishAllData(data_obj, site, deviceId);
            modbus_server.set_from_hpData(data_obj, 0);
            
        } catch (const std::exception &e) {
            logger->error("Processing failed: {}", e.what());
        }
    }
}
```

#### 3. Config-parsning (config.cpp)

```cpp
bool Config::loadFromFile(const std::string &path, std::string &err) {
    std::ifstream ifs(path);
    if (!ifs) {
        err = "Could not open config file: " + path;
        return false;
    }
    
    std::string current_section;
    std::string line;
    
    while (std::getline(ifs, line)) {
        line = trim(line);
        
        // Skippa tomma rader och kommentarer
        if (line.empty() || line[0] == ';' || line[0] == '#') 
            continue;
        
        // Section header [SECTION]
        if (line.front() == '[' && line.back() == ']') {
            current_section = trim(line.substr(1, line.size() - 2));
            continue;
        }
        
        // Key=value par
        auto eq = line.find('=');
        if (eq != std::string::npos) {
            std::string key = trim(line.substr(0, eq));
            std::string val = trim(line.substr(eq + 1));
            
            // Strip inline comments
            size_t comment = val.find_first_of(";#");
            if (comment != std::string::npos) {
                val = trim(val.substr(0, comment));
            }
            
            data_[current_section][key] = val;
        }
    }
    return true;
}
```

#### 4. Seriell läsning (hpSerialRead.cpp)

```cpp
std::vector<uint8_t> hpSerialRead::hpRead() {
    uint8_t read_buff[256];
    bool reading_message = false;
    bool end_found = false;
    int crc_bytes_read = 0;
    const int CRC_SIZE = 4;
    std::vector<uint8_t> message;
    
    while(true) {
        int num_bytes = read(serial_fd, read_buff, sizeof(read_buff));
        
        if (num_bytes <= 0) continue;
        
        for (int i = 0; i < num_bytes; i++) {
            uint8_t byte = read_buff[i];
            
            // Start delimiter
            if (byte == '/') {
                message.clear();
                message.push_back(byte);
                reading_message = true;
                end_found = false;
                crc_bytes_read = 0;
                continue;
            }
            
            // End delimiter
            if (byte == '!' && reading_message && !end_found) {
                message.push_back(byte);
                end_found = true;
                crc_bytes_read = 0;
                continue;
            }
            
            // Läs CRC bytes
            if (end_found) {
                message.push_back(byte);
                crc_bytes_read++;
                if (crc_bytes_read == CRC_SIZE) {
                    return message;
                }
                continue;
            }
            
            // Lägg till databytes
            if (reading_message) {
                message.push_back(byte);
            }
        }
    }
}
```

**Serial port konfiguration:**
```cpp
void hpSerialRead::setupPort() {
    tcgetattr(serial_fd, &tty);
    
    // Control flags
    tty.c_cflag &= ~PARENB;   // No parity
    tty.c_cflag &= ~CSTOPB;   // 1 stop bit
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;        // 8 data bits
    tty.c_cflag &= ~CRTSCTS;   // No flow control
    tty.c_cflag |= CREAD | CLOCAL;
    
    // Local flags
    tty.c_lflag &= ~ICANON;    // Non-canonical mode
    tty.c_lflag &= ~ISIG;      // No signals
    
    // Input flags
    tty.c_iflag &= ~(IXON | IXOFF | IXANY);
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL);
    
    // Timeouts
    tty.c_cc[VTIME] = 255;     // Non-blocking with timeout
    tty.c_cc[VMIN] = 0;        // Return immediately
    
    // Baudrate
    cfsetispeed(&tty, B115200);
    
    tcsetattr(serial_fd, TCSANOW, &tty);
}
```

#### 5. CRC-validering (hpMessageValidator.cpp)

```cpp
void HanportMessageValidator::calculate_crc() {
    uint16_t crc = 0x0000;
    
    for (uint8_t byte : hp_meter_data) {
        crc ^= byte;
        for (int i = 0; i < 8; i++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    
    calculated_crc = crc;
}
```

#### 6. MQTT-publicering (hpMqttPub.cpp)

```cpp
void HpMqttPub::publishAllData(const hpData &data, 
                                const std::string &site,
                                const std::string &device) {
    // Skapa JSON payload
    std::ostringstream json;
    json << "{";
    json << "\"voltage_L1\":" << data.voltage_L1 << ",";
    json << "\"voltage_L2\":" << data.voltage_L2 << ",";
    json << "\"voltage_L3\":" << data.voltage_L3 << ",";
    json << "\"current_L1\":" << data.current_L1 << ",";
    json << "\"current_L2\":" << data.current_L2 << ",";
    json << "\"current_L3\":" << data.current_L3 << ",";
    json << "\"timestamp\":" << data.time_stamp;
    json << "}";
    
    // Publicera med QoS 1
    mqtt::message_ptr pubmsg = mqtt::make_message(topic_, json.str());
    pubmsg->set_qos(1);
    client_->publish(pubmsg);
}
```

#### 7. Modbus register-mappning (hpModbuss.cpp)

```cpp
void hpModbuss::set_from_hpData(const hpData &data, int offset) {
    // Varje float = 2 register (IEEE 754, 32-bit)
    int reg = offset;
    
    setFloatRegister(reg, data.voltage_L1);     reg += 2;
    setFloatRegister(reg, data.voltage_L2);     reg += 2;
    setFloatRegister(reg, data.voltage_L3);     reg += 2;
    setFloatRegister(reg, data.current_L1);     reg += 2;
    setFloatRegister(reg, data.current_L2);     reg += 2;
    setFloatRegister(reg, data.current_L3);     reg += 2;
    setFloatRegister(reg, data.active_power_pos); reg += 2;
    // ... etc
}

void hpModbuss::setFloatRegister(int reg, float value) {
    uint32_t bits;
    memcpy(&bits, &value, sizeof(float));
    
    registers_[reg] = (bits >> 16) & 0xFFFF;      // High word
    registers_[reg + 1] = bits & 0xFFFF;          // Low word
}
```

### Viktiga datastrukturer

#### hpData (include/hpData.hpp)
```cpp
struct hpData {
    // Spänningar (V)
    float voltage_L1;
    float voltage_L2;
    float voltage_L3;
    
    // Strömmar (A)
    float current_L1;
    float current_L2;
    float current_L3;
    
    // Effekter (kW / kVAr)
    float active_power_pos;
    float active_power_neg;
    float reactive_power_pos;
    float reactive_power_neg;
    
    // Energi (kWh / kVArh)
    float active_energy_import;
    float active_energy_export;
    float reactive_energy_import;
    float reactive_energy_export;
    
    // Metadata
    float time_stamp;  // Heartbeat counter
};
```

## Protokoll

### Hanport Serial Protocol

**Meddelandeformat:**
```
/<data_fields>!<CRC>
```

- **Start**: `/` (0x2F)
- **Data**: ASCII-siffror och mellanslag/kommatecken
- **End**: `!` (0x21)
- **CRC**: 4 ASCII hex-tecken (CRC16, little-endian)

**Exempel:**
```
/230.5 5.2 1.15 229.8 4.8 1.10!A5F3
```

**CRC-beräkning:**
- CRC16 (polynomial 0xA001)
- Beräknas på bytes mellan `/` och `!` (exklusive delimiters)
- Resultat konverteras till 4 hex-tecken

### MQTT Protocol

**Topic-struktur:**
```
<measurement_topic>/<site_id>/<device_id>
```

**Exempel:**
```
hanport_data/bgs-office/hanport_meter_01
```

**Payload (JSON):**
```json
{
  "voltage_L1": 230.5,
  "voltage_L2": 229.8,
  "voltage_L3": 231.2,
  "current_L1": 5.2,
  "current_L2": 4.8,
  "current_L3": 5.5,
  "active_power_pos": 3.6,
  "active_power_neg": 0.0,
  "reactive_power_pos": 0.5,
  "reactive_power_neg": 0.0,
  "timestamp": 12345.0
}
```

**QoS**: 1 (At least once delivery)
**Retained**: No
**Last Will**: `<will_topic>` med meddelande "offline"

### Modbus TCP Protocol

**Registertyp**: Holding Registers (Function Code 0x03/0x04)
**Data format**: IEEE 754 32-bit floats

**Registermappning:**

| Register | Fält | Datatyp | Enhet |
|----------|------|---------|-------|
| 0-1 | voltage_L1 | float32 | V |
| 2-3 | voltage_L2 | float32 | V |
| 4-5 | voltage_L3 | float32 | V |
| 6-7 | current_L1 | float32 | A |
| 8-9 | current_L2 | float32 | A |
| 10-11 | current_L3 | float32 | A |
| 12-13 | active_power_pos | float32 | kW |
| 14-15 | active_power_neg | float32 | kW |
| 16-17 | reactive_power_pos | float32 | kVAr |
| 18-19 | reactive_power_neg | float32 | kVAr |
| ... | ... | ... | ... |

**Läsa register:**
```bash
# Med mbpoll
mbpoll -a 1 -r 0 -c 2 -t 4:float localhost 1502

# Med Python pymodbus
from pymodbus.client import ModbusTcpClient
client = ModbusTcpClient('localhost', port=1502)
result = client.read_holding_registers(0, 2)  # Läs voltage_L1
```

## Kodstil och best practices

### Namnkonventioner
- **Klasser**: `PascalCase` (`HanportMessageValidator`)
- **Funktioner**: `camelCase` (`calculateCrc`)
- **Variabler**: `snake_case` (`voltage_L1`)
- **Privata medlemmar**: trailing `_` (`data_`)
- **Konstanter**: `UPPER_SNAKE_CASE` (`CRC_SIZE`)

### Felhantering

**Kritiska fel (kasta exception):**
```cpp
if (serial_fd < 0) {
    std::string error = "Failed to open serial port: " + 
                       std::string(serial_port_path);
    logger->error("SerialRead: {}", error);
    throw std::runtime_error(error);
}
```

**Icke-kritiska fel (logga och fortsätt):**
```cpp
try {
    mqtt_publisher.publishAllData(data_obj, site, deviceId);
    logger->info("Data published successfully to MQTT");
} catch (const std::exception &e) {
    logger->error("MQTT: Failed to publish data: {}", e.what());
    // Fortsätt utan MQTT
}
```

### Loggning med spdlog

**Loggnivåer:**
```cpp
logger->trace("Detailed trace information");
logger->debug("Debug information: value={}", value);
logger->info("Normal operation: {}", message);
logger->warn("Warning: {}", warning);
logger->error("Error occurred: {}", error);
logger->critical("Critical error: {}", critical);
```

**Best practices:**
- Använd `info` för normal drift
- Använd `error` för fel som bör åtgärdas
- Använd `debug` för utvecklingsinformation
- Använd `{}` för formattering (spdlog formattering)

### Minneshantering

**RAII-principer:**
```cpp
// Bra: Automatisk resurshantering
class hpSerialRead {
public:
    ~hpSerialRead() { closePort(); }  // Cleanup i destruktor
private:
    int serial_fd;
};

// Undvik: Manuell minneshantering med new/delete
```

**Smart pointers (vid behov):**
```cpp
#include <memory>

std::unique_ptr<HpMqttPub> mqtt_pub = 
    std::make_unique<HpMqttPub>(broker, client_id, site, device);
```

## Tester

### Enhetstester (TODO)

Skapa `tests/` katalog och använd ett testramverk som Google Test:

```cpp
// tests/test_config.cpp
#include <gtest/gtest.h>
#include "Config.hpp"

TEST(ConfigTest, LoadValidFile) {
    Config cfg;
    std::string err;
    ASSERT_TRUE(cfg.loadFromFile("test.ini", err));
    EXPECT_EQ(cfg.get("MQTT", "BROKER", ""), "localhost:1883");
}

TEST(ConfigTest, MissingFile) {
    Config cfg;
    std::string err;
    ASSERT_FALSE(cfg.loadFromFile("nonexistent.ini", err));
    EXPECT_FALSE(err.empty());
}
```

### Integrationstester

```bash
# Skapa test-script
#!/bin/bash

# 1. Starta mock MQTT broker
mosquitto -p 1883 &
MQTT_PID=$!

# 2. Skapa mock serial data
mkfifo /tmp/mock_serial
echo "test data" > /tmp/mock_serial &

# 3. Kör programmet
./hanport-data-parser --config test.ini &
APP_PID=$!

# 4. Verifiera MQTT meddelanden
timeout 10 mosquitto_sub -h localhost -t "hanport_data/#" -C 1

# 5. Cleanup
kill $APP_PID $MQTT_PID
rm /tmp/mock_serial
```

### Manuell testning

**Serial port test:**
```bash
# Skapa virtual serial ports
socat -d -d pty,raw,echo=0 pty,raw,echo=0

# Skriv testdata till ena porten
echo "/230.5 5.2!ABCD" > /dev/pts/X

# Kör programmet mot andra porten
./hanport-data-parser --serial-port /dev/pts/Y
```

**MQTT test:**
```bash
# Lyssna på alla topics
mosquitto_sub -h localhost -v -t "#"

# Publicera testmeddelande
mosquitto_pub -h localhost -t "test" -m "hello"
```

**Modbus test:**
```bash
# Installera mbpoll
sudo apt install mbpoll

# Läs register 0-1 (voltage_L1)
mbpoll -a 1 -r 0 -c 2 -t 4:float localhost 1502

# Kontinuerlig polling
mbpoll -a 1 -r 0 -c 10 -t 4 -1 localhost 1502
```

## Debugging

### Kompilera med debug-symboler

```bash
mkdir build-debug && cd build-debug
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### GDB (GNU Debugger)

```bash
# Starta med gdb
gdb ./hanport-data-parser

# I gdb:
(gdb) run --config ../configs/app.ini
(gdb) break main.cpp:100        # Sätt breakpoint
(gdb) continue                  # Fortsätt körning
(gdb) print variable_name       # Visa variabelvärde
(gdb) bt                        # Backtrace vid crash
(gdb) info locals               # Visa lokala variabler
```

### Valgrind (minnesläckor)

```bash
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --verbose \
         ./hanport-data-parser --config configs/app.ini
```

### Loggning för debugging

```cpp
// Sätt debug-nivå
logger->set_level(spdlog::level::debug);

// Logga viktiga händelser
logger->debug("Raw message size: {}", raw_message.size());
logger->debug("CRC calculated: {:04X}, transmitted: {:04X}", 
              calculated_crc, transmitted_crc);
```

### Strace (systemanrop)

```bash
# Visa alla systemanrop
strace -o trace.log ./hanport-data-parser

# Filtrera endast file operations
strace -e trace=file ./hanport-data-parser

# Filtrera serial port operations
strace -e trace=open,read,write,ioctl ./hanport-data-parser
```

## Performance optimization

### Profiling med gprof

```bash
# Kompilera med profiling
g++ -pg -O2 src/*.cpp -o hanport-data-parser

# Kör programmet
./hanport-data-parser

# Analysera
gprof hanport-data-parser gmon.out > analysis.txt
```

### Optimeringstips

1. **Undvik onödiga kopior:**
```cpp
// Bra: Reference
void processData(const std::vector<uint8_t>& data);

// Undvik: Kopia
void processData(std::vector<uint8_t> data);
```

2. **Reservera vektorstorlek:**
```cpp
std::vector<uint8_t> message;
message.reserve(256);  // Undvik reallokering
```

3. **Cacha vanliga värden:**
```cpp
// Cache site + device för MQTT topic
std::string cached_topic = measurement_topic + "/" + site + "/" + device;
```

## Bidra till projektet

### Workflow

1. Forka repository på GitHub
2. Klona din fork: `git clone https://github.com/your-username/hanport-data-parser.git`
3. Skapa feature branch: `git checkout -b feature/ny-funktion`
4. Gör ändringar och testa
5. Commit: `git commit -m "Add: beskrivning av ändring"`
6. Push: `git push origin feature/ny-funktion`
7. Skapa Pull Request på GitHub

### Commit-meddelanden

Använd Conventional Commits format:

```
Add: ny funktion för X
Fix: buggfix i CRC-beräkning
Update: förbättra MQTT reconnect-logik
Docs: uppdatera användarguide
Refactor: omstrukturera config-parser
Test: lägg till enhetstester för validator
```

### Pull Request checklist

- [ ] Kod kompilerar utan warnings (`-Wall -Wextra`)
- [ ] Debug build fungerar
- [ ] Release build fungerar
- [ ] Testad på Raspberry Pi (om möjligt)
- [ ] Dokumentation uppdaterad
- [ ] CHANGELOG.md uppdaterad
- [ ] Code review genomförd
- [ ] Commit messages följer standard

### Code review guidelines

- Håll PR:s fokuserade och små
- Beskriv vad och varför, inte hur
- Inkludera tester om möjligt
- Följ befintlig kodstil
- Kommentera komplexa delar

## Vanliga problem

### Build-fel

**"spdlog not found":**
```bash
sudo apt install libspdlog-dev
# eller
cmake -Dspdlog_DIR=/path/to/spdlog ..
```

**"paho-mqtt not found":**
```bash
sudo apt install libpaho-mqttpp-dev libpaho-mqtt-dev
```

**"undefined reference to...":**
```bash
# Kontrollera länkningsordning i CMakeLists.txt
# Bibliotek måste komma efter källfiler
```

### Runtime-fel

**"Permission denied" på serial port:**
```bash
sudo usermod -a -G dialout $USER
# Logga ut och in igen
```

**"Address already in use" (Modbus):**
```bash
# Port redan använd
sudo netstat -tulpn | grep 1502
# Ändra port i config eller döda process
```

## Resurser

### Dokumentation
- [CMake Documentation](https://cmake.org/documentation/)
- [spdlog GitHub](https://github.com/gabime/spdlog)
- [Eclipse Paho MQTT C++](https://github.com/eclipse/paho.mqtt.cpp)
- [libmodbus Documentation](https://libmodbus.org/)

### Verktyg
- [GDB Tutorial](https://www.sourceware.org/gdb/documentation/)
- [Valgrind User Manual](https://valgrind.org/docs/manual/)
- [Git Best Practices](https://git-scm.com/doc)

### Community
- GitHub Issues för buggrapporter
- GitHub Discussions för frågor
- Pull Requests för bidrag