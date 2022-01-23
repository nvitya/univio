import univio

def basic_tests():
	r = conn.read_uint(0x0000)
	print('Read result: %08X' % r)
	r = conn.read_uint(0x0000)
	print('Read result: %08X' % r)

	try:
		r = conn.read_uint(0x6666) # invalid address
		print('Read result: %08X' % r)
	except univio.UioError as e:
		print("error: "+str(e))

	rs = conn.read(0x0003, 32)
	print('0003: ', rs)

	# set blinking pattern
	conn.write_uint(0x1500, 0x0F0FF00F, 4)
	conn.write_uint(0x1500, 0x555500F0, 4)
#/

def spi_test():
	print("SPI Test")
	conn.write_uint(0x1600, 2400000, 4)  # speed
	conn.write_uint(0x1601, 16, 2)  # length
	conn.write(0xC000, bytearray([1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16]))  # data
	conn.write_uint(0x1602, 1, 1) # start

	while True:
		r  = conn.read_uint(0x1602)
		if r == 0:
			break
		#/
	#/
	print("SPI test finished.")
#/

def test_nvdata():
	print("NVDATA Test")
	nvdata = []
	for n in range(0, 31):
		nvdata.append(conn.read_uint(0x0F00 + n))
	print("NVDATA = ", nvdata)
	print("incrementing all")
	conn.write_uint(0x0F80, 0x5ADEC0DE)  # unlock
	for n in range(0, 31):
		nvdata[n] = nvdata[n] + 1
		conn.write_uint(0x0F00 + n, nvdata[n])
	#/
	print("Reading again:")
	nvdata = []
	for n in range(0, 31):
		nvdata.append(conn.read_uint(0x0F00 + n))
	print("NVDATA = ", nvdata)


print("Generic UnivIO Device Test")
conn = univio.UioComm()
#conn.open("/dev/ttyUSB0")
conn.open("/dev/ttyACM1")

r = conn.read_uint(0x0000)
if 0x66CCAA55 != r:
	print('Device Check error: %08X' % r)

#basic_tests()
#spi_test()
test_nvdata()