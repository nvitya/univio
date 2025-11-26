unit univio_i2c;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, udo_common, udo_comm;

const
  UDOERR_I2C   = $A010;

const
  I2CEX_0      = $00000000;  // do not send extra data
  I2CEX_1      = $01000000;  // send 1 extra byte
  I2CEX_2      = $02000000;  // send 2 extra bytes
  I2CEX_3      = $03000000;  // send 3 extra bytes
  I2CEX_MASK   = $03000000;

  I2C_MPRAM_OFFS = $1000;

type

  { TUnivioI2c }

  TUnivioI2c = class
  public
    comm : TUdoComm;
    port : integer;
    baseobj : uint16;

    mpramofs : uint32;

    constructor Create(acomm : TUdoComm);

    procedure Init(aport : integer; aspeed : uint32);

    procedure SetPort(aport : integer);
    procedure SetSpeed(aspeed : uint32);
    procedure SetMpramOfs(aofs : uint32);

    procedure Write(aaddr : byte; aextra : uint32; dataptr : pointer; len : uint32);
    procedure Read(aaddr : byte; aextra : uint32; dataptr : pointer; len : uint32);
  end;

implementation

{ TUnivioI2c }

constructor TUnivioI2c.Create(acomm : TUdoComm);
begin
  comm := acomm;
  port := 0;
  mpramofs := I2C_MPRAM_OFFS;
end;

procedure TUnivioI2c.Init(aport : integer; aspeed : uint32);
begin
  SetPort(aport);
  SetSpeed(aspeed);
  SetMpramOfs(mpramofs);
end;

procedure TUnivioI2c.SetPort(aport : integer);
begin
  port := aport;
  baseobj := $1700 + port * $20;
end;

procedure TUnivioI2c.SetSpeed(aspeed : uint32);
begin
  comm.WriteU32(baseobj + 0, 0, aspeed);
end;

procedure TUnivioI2c.SetMpramOfs(aofs : uint32);
begin
  mpramofs := aofs;
  comm.WriteU16(baseobj + 4, 0, mpramofs);
end;

procedure TUnivioI2c.Write(aaddr : byte; aextra : uint32; dataptr : pointer; len : uint32);
var
  elen : uint32;
  r : uint16;
begin
  elen := (aextra shr 24);

  if elen > 0 then
  begin
    comm.WriteU32(baseobj + 1, 0, aextra and $00FFFFFF);
  end;

  // put the write data
  udocomm.UdoWrite($C000, mpramofs, dataptr^, len);

  // start the transaction
  udocomm.WriteU32(baseobj + 2, 0, (0
    + (1     shl  0)  // DATADIR: 0 = read, 1 = write
    + (aaddr shl  1)  // DADDR(7)
    + (elen  shl 12)  // EADDR_LEN(2)
    + (1     shl 16)  // RWLEN: read/write length
  ));

  // wait for completition
  repeat
    r := udocomm.ReadU16(baseobj + 3, 0);
  until r <> $FFFF;

  if r > 0
  then
      raise EUdoAbort.Create(UDOERR_I2C + r, 'I2C Write Error: %d', [r]);
end;

procedure TUnivioI2c.Read(aaddr : byte; aextra : uint32; dataptr : pointer; len : uint32);
var
  elen : uint32;
  r : uint16;
begin
  elen := (aextra shr 24);

  if elen > 0 then
  begin
    comm.WriteU32(baseobj + 1, 0, aextra and $00FFFFFF);
  end;

  // start the transaction
  udocomm.WriteU32(baseobj + 2, 0, (0
    + (0      shl  0)  // DATADIR: 0 = read, 1 = write
    + (aaddr  shl  1)  // DADDR(7)
    + (elen   shl 12)  // EADDR_LEN(2)
    + (len    shl 16)  // RWLEN: read/write length
  ));

  // wait for completition
  repeat
    r := udocomm.ReadU16(baseobj + 3, 0);
  until r <> $FFFF;

  if r > 0
  then
      raise EUdoAbort.Create(UDOERR_I2C + r, 'I2C Read Error: %d', [r]);

  udocomm.UdoRead($C000, mpramofs, dataptr^, len);

end;

end.

