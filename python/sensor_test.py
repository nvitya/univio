import univio
import sys
import time
import math

print("USB-Sensor test")
conn = univio.UioComm()
#conn.open("/dev/ttyUSB0")
conn.open("/dev/ttyACM1")

r = conn.read_uint(0x0000)
if 0x66CCAA55 != r:
	print('Device Check error: %08X' % r)
	sys.exit(1)

r = conn.read_uint(0x2001) # get bmp280 measure count
print("BMP280 measure count=", r)
r = conn.read_uint(0x2002) # get bmp280 pressure
print("  pressure = ", r)

r = conn.read_uint(0x2101) # get aht10 measure count
print("AHT10 measure count=", r)
r = conn.read_uint(0x2102) # get rh %
print("  Rh % = ", r / 100)
t = conn.read_uint(0x2103) # get tem
print("  T = ", t / 100)

print('setting clock...')
tobj = time.localtime(time.time())
tsecs = tobj.tm_hour * 60 * 60 + tobj.tm_min * 60 + tobj.tm_sec
conn.write_uint(0x3000, tsecs)

datetext_bin = str('2022-08-31').encode('ascii')
conn.write(0x3001, datetext_bin)

daytext_bin = str('Wednesday').encode('ascii')
conn.write(0x3002, daytext_bin)

print('updating history')
# prepare some data for the history
barr = bytearray()
for n in range(0, 288):
		v = 32 + 20 * math.sin(n / 6)
		barr.append(int(v))
conn.write(0x8200, barr)
barr = bytearray()
for n in range(0, 288):
		v = 32 + 20 * math.sin(n / 4)
		barr.append(int(v))
conn.write(0x8400, barr)
barr = bytearray()
for n in range(0, 288):
		v = 32 + 20 * math.sin(n / 30)
		if n > 200:  v = 0
		barr.append(int(v))
conn.write(0x8600, barr)
