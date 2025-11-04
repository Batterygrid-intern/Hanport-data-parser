#!/usr/bin/env python3
"""
Simple Modbus TCP client for testing the hpModbuss server.

Requires: pip install pymodbus

Usage:
  python3 scripts/modbus_client.py --host 192.168.1.10 --port 1502 --start 2 --count 2

This will read `count` registers starting at `start` and print them. If `count` is even
the client will print pairs as 32-bit IEEE floats (high-word then low-word).
"""
import argparse
from pymodbus.client.sync import ModbusTcpClient
import struct


def regs_to_float(high, low):
    u = (high << 16) | low
    return struct.unpack('!f', struct.pack('!I', u))[0]


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--host', required=True)
    p.add_argument('--port', type=int, default=1502)
    p.add_argument('--start', type=int, default=0)
    p.add_argument('--count', type=int, default=2)
    args = p.parse_args()

    client = ModbusTcpClient(args.host, port=args.port)
    if not client.connect():
        print('Failed to connect to {}:{}'.format(args.host, args.port))
        return 2

    rr = client.read_holding_registers(args.start, args.count, unit=1)
    if rr.isError():
        print('Modbus read error:', rr)
        return 3
    regs = rr.registers
    print('Read {} registers starting at {}'.format(len(regs), args.start))
    for i, v in enumerate(regs):
        print('R[{}] = {}'.format(args.start + i, v))

    # if even count, print floats from pairs
    if len(regs) >= 2:
        for i in range(0, len(regs) - 1, 2):
            f = regs_to_float(regs[i], regs[i+1])
            print('Float @ {}..{} = {}'.format(args.start + i, args.start + i + 1, f))

    client.close()


if __name__ == '__main__':
    main()
