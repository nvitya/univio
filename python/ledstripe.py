import univio
import time

def SetBarrBit(barr, bitidx):
	byteidx = (bitidx >> 3)
	sbidx = 7 - (bitidx & 7)
	barr[byteidx] |= (1 << sbidx)

class UioLedStripe:
	def __init__(self, aconn, aledcount):
		self.conn = aconn
		self.ledcount = aledcount
		self.resetbytes = 30
		self.tailresetbytes = 30
		self.spidata = bytearray()
		self.SetLedCount(aledcount)
		self.conn.write_uint(0x1600, 2400000, 4)  # set SPI speed to 2.4 MHz

	def SetLedCount(self, aledcount):
		self.spidata = bytearray()
		self.ledcount = aledcount
		for n in range(0, self.resetbytes):
			self.spidata.extend([0])
		for n in range(0, self.ledcount):
			self.spidata.extend([0,0,0, 0,0,0, 0,0,0])
		for n in range(0, self.tailresetbytes):
			self.spidata.extend([0])
		#/
		conn.write_uint(0x1601, len(self.spidata), 2)  # length

	def SetLed(self, aidx, ar, ag, ab):
		ldata = bytearray([0,0,0, 0,0,0, 0,0,0])
		grbcode = (ag << 16) | (ar << 8) | ab
		cbmask = (1 << 23)
		bidx = 0
		while cbmask:
			SetBarrBit(ldata, bidx)
			if grbcode & cbmask:
				SetBarrBit(ldata, bidx+1)
			#/
			bidx += 3
			cbmask = (cbmask >> 1)
		#/
		ledpos = self.resetbytes + 9 * aidx
		for n in range(0, 9):
			self.spidata[ledpos + n] = ldata[n]
		#/

	def Update(self):
		conn.write(0xC000, self.spidata)  # upload the SPI data
		#conn.write_uint(0x1601, len(self.spidata), 2)  # length
		conn.write_uint(0x1602, 1, 1)  # start
	#/
#/  class UioLedStripe

print("Led stripe test")
conn = univio.UioComm()
#conn.open("/dev/ttyUSB0")
conn.open("/dev/ttyACM1")
conn.write_uint(0x1500, 0xFFFFFFFC, 4)

ledcnt = 30

ls = UioLedStripe(conn, ledcnt)

for n in range(0, ledcnt):
	ls.SetLed(n, 0, 0, 20)

ls.Update()

cnt = 0
upcnt = 1
while True:
	for n in range(0, ledcnt):
		if n == cnt:
			ls.SetLed(n, 20, 0, 0)
		else:
			ls.SetLed(n, 0, 0, 0)
		#/
	#/
	ls.Update()
	time.sleep(0.01)
	#
	if upcnt:
		if cnt < ledcnt - 1:
			cnt = cnt + 1
		else:
			upcnt = 0
			cnt -= 1
	else:
		if cnt > 0:
			cnt -= 1
		else:
			cnt = 1
			upcnt = 1
#/

#ledstripe_test()
