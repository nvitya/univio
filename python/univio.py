import serial

# CRC8 table with the standard polynom of 0x07:
univio_crc_table = bytearray([
  0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
  0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
  0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5, 0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
  0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85, 0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
  0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2, 0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
  0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2, 0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
  0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32, 0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
  0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42, 0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
  0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c, 0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
  0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec, 0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
  0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c, 0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
  0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c, 0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
  0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b, 0x76, 0x71, 0x78, 0x7f, 0x6a, 0x6d, 0x64, 0x63,
  0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b, 0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
  0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb, 0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8d, 0x84, 0x83,
  0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb, 0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3
])

UIOERR_CONNECTION     = 0x1001  # not connected, send / receive error
UIOERR_CRC            = 0x1002
UIOERR_TIMEOUT        = 0x1003

UIOERR_WRONG_ADDR     = 0x2001  # address not existing
UIOERR_WRONG_ACCESS   = 0x2002
UIOERR_READ_ONLY      = 0x2003
UIOERR_WRITE_ONLY     = 0x2004
UIOERR_VALUE          = 0x2005  # invalid value
UIOERR_RUN_MODE       = 0x2006  # config mode required
UIOERR_UNITSEL        = 0x2007  # the referenced unit is not existing

UIOERR_NOT_IMPLEMENTED = 0x9001
UIOERR_INTERNAL        = 0x9002  # internal implementation error

def calculate_crc(abarr):
  crc = 0
  for b in abarr:
    idx = (crc ^ b)
    crc = univio_crc_table[idx]
  #
  return crc

class UioError(Exception):
  """Base class for all communication exceptions."""
  def __init__(self, aerrorcode, amessage):
    self.errorcode = aerrorcode
    self.message = amessage

class UioComm:
  def __init__(self):
    self.com = None
    self.baudrate = 1000000
    self.timeout = 0.1
    self.comdev = ""
    self.txdata = bytearray()
    self.rxdata = bytearray()

    self.rq_addr = 0x0000
    self.rq_iswrite = False
    self.rq_result = 0
    self.rq_length = 0
    self.rq_metalen = 0
    self.rq_metadata = bytearray([])
    self.rq_data =  bytearray([])
    self.rsp_iswrite = 0
    self.rsp_iserr = 0
    self.rsp_addr = 0
    self.rsp_result = 0
    self.rsp_metalen = 0
    self.rsp_errorcode = 0
    self.rsp_length = 0
    self.rsp_data =  bytearray([])
    self.rsp_metadata = bytearray([])

  def open(self, acomdev):
    if self.com:
      self.close()
    self.comdev = acomdev
    self.com = serial.Serial(self.comdev, self.baudrate, timeout = self.timeout)

  def close(self):
    if self.com:
      self.com.close()
    self.com = None

  def exec_request(self):

    # 1. Sending Request

    if self.rq_iswrite:
      self.rq_length = len(self.rq_data)
    self.rq_metalen = len(self.rq_metadata)
    self.rsp_data = bytearray()
    self.rsp_metadata = bytearray()
    #
    self.txdata = bytearray([0x55])
    #
    b = 0
    if self.rq_iswrite:  b = 1
    if 2 == self.rq_metalen:     b |= (1 << 2)
    elif 4 == self.rq_metalen:   b |= (2 << 2)
    elif 8 == self.rq_metalen:   b |= (3 << 2)
    else:
      # invalid metadata length
      self.rq_metadata = bytearray()
      self.rq_metalen = 0
    #
    if self.rq_length >= 15:
      b |= 0xF0
    else:
      b |= (self.rq_length << 4)
    #
    self.txdata.extend([b])
    if self.rq_length >= 15:
      self.txdata.extend([self.rq_length & 0xFF, (self.rq_length >> 8) & 0xFF])
    #
    self.txdata.extend([self.rq_addr & 0xFF, (self.rq_addr >> 8) & 0xFF])
    #
    if self.rq_metalen > 0:
      self.txdata.extend(self.rq_metadata)
    #
    if self.rq_iswrite and self.rq_length > 0:
      self.txdata.extend(self.rq_data)
    #
    self.txdata.extend([calculate_crc(self.txdata)])
    #
    #print('request:')
    #print("".join(" %02X" % b for b in self.txdata))

    r = self.com.write(self.txdata)
    #print("send result: ", r)

    # 2. Receiving Response

    self.rxdata = bytearray()
    rxstate = 0
    rxcnt = 0
    rxpos = 0
    while True:
      rxch = self.com.read(1)
      #print('received: ', rxch)
      if len(rxch) == 0:
        raise UioError(UIOERR_CONNECTION, "UioConn: Error receiving response")
      #/
      self.rxdata.extend(bytearray(rxch))
      #
      while rxpos < len(self.rxdata):
        b = self.rxdata[rxpos]
        if 0 == rxstate:
          rxpos = 0
          rxcnt = 0
          if 0x55 == self.rxdata[0]:
            rxstate = 1
          else:
            self.rxdata.pop(0) # remove the first byte and continue
          #/
        elif 1 == rxstate: # command and length
          self.rsp_iswrite = ((b & 1) != 0)
          self.rsp_iserr   = ((b & 2) != 0)
          #
          self.rsp_metalen = ((0x8420 >> (b & 0xC)) & 0xF)
          self.rsp_length = (b >> 4)
          if self.rsp_length == 15:
            rxstate = 2 # extended length follows
          else:
            rxstate = 3 # address
          #/
        elif 2 == rxstate: # extended length
          if 0 == rxcnt:
            self.rsp_length = b
            rxcnt = 1
          else:
            self.rsp_length |= (b << 8)
            rxcnt = 0
            rxstate = 3
          #/
        elif 3 == rxstate:  # address
          if 0 == rxcnt:
            self.rsp_addr = b
            rxcnt = 1
          else:
            self.rsp_addr |= (b << 8)
            rxcnt = 0
            if self.rsp_metalen:
              rxstate = 4  # receive metadata
            else:
              if not self.rsp_iswrite or self.rsp_iserr:
                rxcnt = 0
                rxstate = 5  # read data or error code
              else:
                rxstate = 10  # crc check
              #/
            #/
          #/
        elif 4 == rxstate:  # metadata
          self.rsp_metadata.append(b)
          rxcnt += 1
          if rxcnt >= self.rsp_metalen:
            if not self.rsp_iswrite or self.rsp_iserr:
              rxcnt = 0
              rxstate = 5  # read data or error code follows
            else:
              rxstate = 10 # crc check
            #/
          #/
        elif 5 == rxstate:  # read data or error code
          if self.rsp_iserr:
            if 0 == rxcnt:
              self.rsp_errorcode = b
              rxcnt = 1
            else:
              self.rsp_errorcode |= (b << 8)
              rxstate = 10 # crc check
            #/
          else: # receive data bytes
            self.rsp_data.append(b)
            rxcnt += 1
            if rxcnt >= self.rsp_length:
              rxstate = 10  # crc check
            #/
        elif 10 == rxstate:  # crc check
          crc = calculate_crc(self.rxdata[0:rxpos])
          if b != crc:
            self.rsp_result = UIOERR_CRC
          else:
            if self.rsp_iserr:
              self.rsp_result = self.rsp_errorcode
            else:
              #TODO: check address, iswrite
              self.rsp_result = 0
            #/
          #/
          if self.rsp_result:
            raise UioError(self.rsp_result, self.errormsg(self.rsp_result))
          else:
            return self.rsp_result
        #/
        rxpos += 1
      #/  while
    #/ while

  def errormsg(self, aerr):
    addrinfo = "Address 0x%04X: " % self.rq_addr
    if   UIOERR_CONNECTION == aerr:    return "Send or receive error"
    elif UIOERR_CRC == aerr:           return "CRC error in response"
    elif UIOERR_TIMEOUT == aerr:       return "Response timeout error"
    elif UIOERR_WRONG_ADDR == aerr:    return addrinfo+"Not existing"
    elif UIOERR_WRONG_ACCESS == aerr:  return addrinfo+"Invalid access"
    elif UIOERR_READ_ONLY == aerr:     return addrinfo+"Read only object"
    elif UIOERR_WRITE_ONLY == aerr:    return addrinfo+"Write only object"
    elif UIOERR_VALUE == aerr:         return addrinfo+"Invalid write value"
    elif UIOERR_RUN_MODE == aerr:      return addrinfo+"Config mode required for write"
    elif UIOERR_UNITSEL == aerr:       return addrinfo+"The referenced unit is not existing"
    elif UIOERR_NOT_IMPLEMENTED == aerr:  return addrinfo+"Not implemented"
    elif UIOERR_INTERNAL == aerr:         return addrinfo+"Internal implementation error"
    else:                                 return addrinfo+("Result error 0x%04X" % aerr)

  def read(self, aaddr, alen):
    """ returns the read result, throws UioError on error"""
    self.rq_iswrite = False
    self.rq_addr = aaddr
    self.rq_length = alen
    self.rq_metalen = 0
    #
    self.exec_request()
    return self.rsp_data

  def write(self, aaddr, adata):
    """ returns nothing, throws UioError on error"""
    self.rq_iswrite = True
    self.rq_addr = aaddr
    self.rq_length = len(adata)
    self.rq_metalen = 0
    self.rq_data = bytearray(adata)
    #
    self.exec_request()

  def read_uint(self, aaddr):
    """ returns the read result, throws UioError on error"""
    self.read(aaddr, 4)
    if self.rsp_length > 4:
      raise UioError(self.rsp_result, "Response Length %u too big for u32" % self.rsp_length)
    #/
    res = 0
    rpos = 0
    while rpos < len(self.rsp_data):
      res |= (self.rsp_data[rpos] << (8 * rpos))
      rpos += 1
    #/
    return res

  def write_uint(self, aaddr, avalue, avaluelen = 4):
    """ returns nothing, throws UioError on error"""
    wdata = bytearray()
    pos = 0
    while pos < avaluelen:
      wdata.extend([(avalue >> (8 * pos)) & 0xFF])
      pos += 1
    #/
    return self.write(aaddr, wdata)
