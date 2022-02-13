import univio
import sys

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

