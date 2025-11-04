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
from pymodbus.client import ModbusTcpClient
import struct
import socket


def regs_to_float(high, low):
    u = (high << 16) | low
    return struct.unpack('!f', struct.pack('!I', u))[0]


def raw_modbus_read(host, port, start, count, unit=1, timeout=3.0):
    """Read holding registers via raw Modbus TCP (no pymodbus). Returns list of uint16 registers."""
    trans_id = 1
    proto_id = 0
    length = 6
    mbap = struct.pack('>HHH', trans_id, proto_id, length)
    unit_id = struct.pack('B', unit)
    pdu = struct.pack('>BHH', 3, start, count)
    adu = mbap + unit_id + pdu
    with socket.create_connection((host, port), timeout=timeout) as s:
        s.sendall(adu)
        header = s.recv(7)
        if len(header) < 7:
            raise RuntimeError('Incomplete MBAP header')
        _, _, length = struct.unpack('>HHH', header[0:6])
        # unit = header[6]
        to_read = length - 1
        body = b''
        while len(body) < to_read:
            chunk = s.recv(to_read - len(body))
            if not chunk:
                break
            body += chunk
        if len(body) < to_read:
            raise RuntimeError('Incomplete PDU body')
        func = body[0]
        if func & 0x80:
            exc = body[1]
            raise RuntimeError(f'Modbus exception code {exc}')
        bytecount = body[1]
        data = body[2:2+bytecount]
        if len(data) != bytecount:
            raise RuntimeError('Mismatch bytecount')
        regs = []
        for i in range(0, len(data), 2):
            regs.append(struct.unpack('>H', data[i:i+2])[0])
        return regs

def main():
    p = argparse.ArgumentParser()
    p.add_argument('--host', required=True)
    p.add_argument('--port', type=int, default=1502)
    p.add_argument('--start', type=int, default=0)
    p.add_argument('--count', type=int, default=2)
    p.add_argument('--all', action='store_true', help='read the full mapped range (0..53)')
    p.add_argument('--pretty', action='store_true', help='pretty-print labelled fields')
    args = p.parse_args()

    # if --all requested, override start/count to read the full mapped set
    if args.all:
        args.start = 0
        args.count = 54

    # Try using pymodbus first; if it fails or API mismatch, fall back to raw-socket
    try:
        client = ModbusTcpClient(args.host, port=args.port)
        if client.connect():
            try:
                # try common signatures
                rr = None
                try:
                    rr = client.read_holding_registers(args.start, args.count, unit=1)
                except TypeError:
                    try:
                        rr = client.read_holding_registers(args.start, args.count, slave=1)
                    except TypeError:
                        rr = client.read_holding_registers(args.start, args.count, 1)
                # interpret response
                if rr is None:
                    raise RuntimeError('pymodbus returned no result')
                if hasattr(rr, 'isError') and rr.isError():
                    raise RuntimeError(f'pymodbus error: {rr}')
                if hasattr(rr, 'registers'):
                    regs = rr.registers
                else:
                    regs = list(rr)
                print('Read {} registers starting at {}'.format(len(regs), args.start))
                if args.pretty and args.start == 0 and len(regs) >= 54:
                    # pretty-print the full mapped register set
                    labels = [
                        ('time_stamp','ticks'),
                        ('active_energy_import_total','kWh'),
                        ('active_energy_export_total','kWh'),
                        ('reactive_energy_import_total','kvarh'),
                        ('reactive_energy_export_total','kvarh'),
                        ('active_power_import','kW'),
                        ('active_power_export','kW'),
                        ('reactive_power_import','kvar'),
                        ('reactive_power_export','kvar'),
                        ('l1_active_power_import','kW'),
                        ('l1_active_power_export','kW'),
                        ('l2_active_power_import','kW'),
                        ('l2_active_power_export','kW'),
                        ('l3_active_power_import','kW'),
                        ('l3_active_power_export','kW'),
                        ('l1_reactive_power_import','kvar'),
                        ('l1_reactive_power_export','kvar'),
                        ('l2_reactive_power_import','kvar'),
                        ('l2_reactive_power_export','kvar'),
                        ('l3_reactive_power_import','kvar'),
                        ('l3_reactive_power_export','kvar'),
                        ('l1_voltage_rms','V'),
                        ('l2_voltage_rms','V'),
                        ('l3_voltage_rms','V'),
                        ('l1_current_rms','A'),
                        ('l2_current_rms','A'),
                        ('l3_current_rms','A')
                    ]
                    for idx, (name, unit) in enumerate(labels):
                        hi = regs[2*idx]
                        lo = regs[2*idx + 1]
                        f = regs_to_float(hi, lo)
                        print(f"{name:30s} @ {2*idx:2d}..{2*idx+1:2d} = {f} {unit}  (0x{hi:04X} 0x{lo:04X})")
                else:
                    for i, v in enumerate(regs):
                        print('R[{}] = {}'.format(args.start + i, v))
                    if len(regs) >= 2:
                        for i in range(0, len(regs) - 1, 2):
                            f = regs_to_float(regs[i], regs[i+1])
                            print('Float @ {}..{} = {}'.format(args.start + i, args.start + i + 1, f))
                client.close()
                raise SystemExit(0)
            finally:
                try:
                    client.close()
                except Exception:
                    pass
        else:
            print(f'Failed to connect to {args.host}:{args.port} using pymodbus')
    except Exception as e:
        print('pymodbus path failed, falling back to raw socket:', e)

    # raw socket fallback
    try:
        regs = raw_modbus_read(args.host, args.port, args.start, args.count)
    except Exception as e:
        print('Raw socket modbus read failed:', e)
        raise SystemExit(4)
    print('Read {} registers starting at {}'.format(len(regs), args.start))
    if args.pretty and args.start == 0 and len(regs) >= 54:
        labels = [
            ('time_stamp','ticks'),
            ('active_energy_import_total','kWh'),
            ('active_energy_export_total','kWh'),
            ('reactive_energy_import_total','kvarh'),
            ('reactive_energy_export_total','kvarh'),
            ('active_power_import','kW'),
            ('active_power_export','kW'),
            ('reactive_power_import','kvar'),
            ('reactive_power_export','kvar'),
            ('l1_active_power_import','kW'),
            ('l1_active_power_export','kW'),
            ('l2_active_power_import','kW'),
            ('l2_active_power_export','kW'),
            ('l3_active_power_import','kW'),
            ('l3_active_power_export','kW'),
            ('l1_reactive_power_import','kvar'),
            ('l1_reactive_power_export','kvar'),
            ('l2_reactive_power_import','kvar'),
            ('l2_reactive_power_export','kvar'),
            ('l3_reactive_power_import','kvar'),
            ('l3_reactive_power_export','kvar'),
            ('l1_voltage_rms','V'),
            ('l2_voltage_rms','V'),
            ('l3_voltage_rms','V'),
            ('l1_current_rms','A'),
            ('l2_current_rms','A'),
            ('l3_current_rms','A')
        ]
        for idx, (name, unit) in enumerate(labels):
            hi = regs[2*idx]
            lo = regs[2*idx + 1]
            f = regs_to_float(hi, lo)
            print(f"{name:30s} @ {2*idx:2d}..{2*idx+1:2d} = {f} {unit}  (0x{hi:04X} 0x{lo:04X})")
    else:
        for i, v in enumerate(regs):
            print('R[{}] = {}'.format(args.start + i, v))
        if len(regs) >= 2:
            for i in range(0, len(regs) - 1, 2):
                f = regs_to_float(regs[i], regs[i+1])
                print('Float @ {}..{} = {}'.format(args.start + i, args.start + i + 1, f))


if __name__ == '__main__':
    main()
