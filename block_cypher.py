import serial
import argparse
import math
import os
import sys
import binascii
import time

# two args: filepath and destination
parser = argparse.ArgumentParser(description='Encrypt/decrypt using the PSoC 5LP')
parser.add_argument('filepath', action="store")
parser.add_argument('destination', action="store")
args = parser.parse_args()

assert len(sys.argv) == 3

if (not os.path.isfile(args.filepath)):
    print("Enter a valid filepath")
    exit()

# open serial connection
ser = serial.Serial('/dev/ttyUSB1', timeout=1)
ser.read_all() # flush output

# input bytes converted to a hex string
fp = binascii.hexlify(open(args.filepath,'rb').read())

# fp is now padded to a multiple of 32
if (len(fp) % 32 != 0):
    fp += (32-(len(fp) % 32)) * b'0'

i = 0
while (i < len(fp)/32):
    tmp = fp[i*32:i*32 + 32] 
    ser.write(tmp) # write chunk to serial
    time.sleep(1)
    i += 1


buf = ser.readlines()[0]

output = open(args.destination,"wb")
output.write(binascii.unhexlify(buf))

output.close()
ser.close()

