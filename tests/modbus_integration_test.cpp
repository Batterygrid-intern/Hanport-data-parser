#include <gtest/gtest.h>
#include "hpModbuss.hpp"
#include "hpMessageValidator.hpp"
#include "hpDataParser.hpp"
#include "hpData.hpp"
#include <vector>
#include <string>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <cmath>

#ifdef LIBMODBUS_FOUND
#include <modbus/modbus.h>
#endif

// CRC calculation copied from hpMessageValidator::calculate_crc
static uint16_t calculate_crc16(const std::vector<uint8_t>& data){
    uint16_t crc = 0x0000;
    for (size_t i = 0; i < data.size(); i++){
        crc ^= data[i];
        for (int j = 0; j < 8; j++){
            if (crc & 0x0001){
                crc = (crc >> 1) ^ 0xA001;
            }
            else{
                crc >>= 1;
            }
        }
    }
    return crc;
}

// helper convert float -> two registers (big-endian words)
static std::vector<uint16_t> float_to_regs(float f){
    uint32_t u = 0;
    std::memcpy(&u, &f, sizeof(u));
    uint16_t high = static_cast<uint16_t>((u >> 16) & 0xFFFF);
    uint16_t low  = static_cast<uint16_t>(u & 0xFFFF);
    return {high, low};
}

// helper reconstruct float from two registers
static float regs_to_float(uint16_t high, uint16_t low){
    uint32_t u = (static_cast<uint32_t>(high) << 16) | static_cast<uint32_t>(low);
    float f;
    std::memcpy(&f, &u, sizeof(u));
    return f;
}

TEST(ModbusIntegration, ParseExampleAndExposeRegisters){
    // sample message (based on exampledata/example1.txt) with CRLF line endings
    const std::string sample =
        "/ELL5\\253833635_A\r\n\r\n"
        "0-0:1.0.0(210217184019W)\r\n"
        "1-0:1.8.0(00006678.394*kWh)\r\n"
        "1-0:2.8.0(00000000.000*kWh)\r\n"
        "1-0:3.8.0(00000021.988*kvarh)\r\n"
        "1-0:4.8.0(00001020.971*kvarh)\r\n"
        "1-0:1.7.0(0001.727*kW)\r\n"
        "1-0:2.7.0(0000.000*kW)\r\n"
        "1-0:3.7.0(0000.000*kvar)\r\n"
        "1-0:4.7.0(0000.309*kvar)\r\n"
        "1-0:21.7.0(0001.023*kW)\r\n"
        "1-0:41.7.0(0000.350*kW)\r\n"
        "1-0:61.7.0(0000.353*kW)\r\n"
        "1-0:22.7.0(0000.000*kW)\r\n"
        "1-0:42.7.0(0000.000*kW)\r\n"
        "1-0:62.7.0(0000.000*kW)\r\n"
        "1-0:23.7.0(0000.000*kvar)\r\n"
        "1-0:43.7.0(0000.000*kvar)\r\n"
        "1-0:63.7.0(0000.000*kvar)\r\n"
        "1-0:24.7.0(0000.009*kvar)\r\n"
        "1-0:44.7.0(0000.161*kvar)\r\n"
        "1-0:64.7.0(0000.138*kvar)\r\n"
        "1-0:32.7.0(240.3*V)\r\n"
        "1-0:52.7.0(240.1*V)\r\n"
        "1-0:72.7.0(241.3*V)\r\n"
        "1-0:31.7.0(004.2*A)\r\n"
        "1-0:51.7.0(001.6*A)\r\n"
        "1-0:71.7.0(001.7*A)\r\n"
        "!\r\n";

    std::vector<uint8_t> raw(sample.begin(), sample.end());
    // compute CRC over message part (up to and including '!')
    // find position of '!'
    size_t excl = 0;
    for(size_t i=0;i<raw.size();++i) if(raw[i] == '!'){ excl = i; break; }
    std::vector<uint8_t> msg_part(raw.begin(), raw.begin() + excl + 1);
    uint16_t crc = calculate_crc16(msg_part);
    // append CRC as hex ascii (no 0x), uppercase
    char buf[8];
    snprintf(buf, sizeof(buf), "%04X", crc);
    std::string crcstr(buf);
    // append crc and CRLF to raw
    raw.insert(raw.end(), crcstr.begin(), crcstr.end());
    raw.push_back('\r'); raw.push_back('\n');

    // validate/parse
    HanportMessageValidator validator(raw);
    ASSERT_EQ(validator.get_calculated_crc(), validator.get_transmitted_crc());
    auto arr = validator.message_to_string_arr();
    hpDataParser parser(arr);
    hpData data;
    parser.parse_message(data);

    // quick sanity check on a parsed value
    EXPECT_NEAR(data.active_enery_import_total, 6678.394f, 0.01f);

    // start modbus server and write a selection of registers as main does
    const uint16_t port = 1810; // test port
    hpModbuss server(port);
    server.start();

    // build regs vector: we'll write time_stamp (0) then active_enery_import_total at regs 2..3
    std::vector<uint16_t> regs;
    // time_stamp 0
    auto r0 = float_to_regs(data.time_stamp);
    regs.insert(regs.end(), r0.begin(), r0.end());
    // active_enery_import_total
    auto r1 = float_to_regs(data.active_enery_import_total);
    regs.insert(regs.end(), r1.begin(), r1.end());

    server.set_holding_registers(0, regs);

#ifdef LIBMODBUS_FOUND
    // client: read registers 2 and 3 and check float
    modbus_t* ctx = modbus_new_tcp("127.0.0.1", port);
    ASSERT_NE(ctx, nullptr);
    ASSERT_EQ(0, modbus_connect(ctx));
    uint16_t out[2] = {0,0};
    int rc = modbus_read_registers(ctx, 2, 2, out);
    ASSERT_EQ(rc, 2);
    float readval = regs_to_float(out[0], out[1]);
    EXPECT_NEAR(readval, data.active_enery_import_total, 0.001f);
    modbus_close(ctx);
    modbus_free(ctx);
#else
    GTEST_SKIP() << "libmodbus not available";
#endif

    server.stop();
}
