import serial
import argparse
import os
import sys
import binascii

parser = argparse.ArgumentParser(description='Encrypt/decrypt using the PSoC 5LP')
parser.add_argument('filepath', action="store")
parser.add_argument('destination', action="store")
args = parser.parse_args()

assert len(sys.argv) == 3

if (not os.path.isfile(args.filepath)):
    print("Enter a valid filepath")
    exit()

ser = serial.Serial('/dev/ttyUSB0', timeout=1)

fp = binascii.hexlify(open(args.filepath,'rb').read())

ret = []

i = 0
while (i < round(len(fp)/16)):
    ser.write(fp[i*16:i*16 + 16])
    i += 1

remainder = len(fp) - i*16
ser.write(fp[-remainder:] + b'\x00'*(16 - remainder))

lines = ser.readlines()
import ipdb; ipdb.set_trace()
ser.close()

output = open(args.destination,"wb")
for line in lines:
    output.write(binascii.unhexlify(line))

output.close()

