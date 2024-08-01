unit univio_spiflash;

{$mode Delphi}

interface

uses
  Classes, SysUtils, udo_comm;

type

  { TUnivIoSpiFlash }

  TUnivIoSpiFlash = class
  public
    comm : TUdoComm;
    speed : integer;
    mpram_txoffs : integer;
    mpram_rxoffs : integer;

    rxbuf : TBytes;
    rxlen : integer;
    txlen : integer;

    constructor Create(acomm : TUdoComm);
    procedure Prepare(aspeed : integer);

    procedure RunTransfer(atxdata : TBytes; readrx : Boolean);
  end;


implementation

{ TUnivIoSpiFlash }

constructor TUnivIoSpiFlash.Create(acomm : TUdoComm);
begin
  comm := acomm;
  speed := 1000000;
  mpram_txoffs := $0000;
  mpram_rxoffs := $0000;

  SetLength(rxbuf, 1024 * 16);
end;

procedure TUnivIoSpiFlash.Prepare(aspeed : integer);
begin
  speed := aspeed;
  comm.WriteU32($1600, 0, speed);
  comm.WriteU16($1604, 0, mpram_txoffs);
  comm.WriteU16($1605, 0, mpram_rxoffs);
end;

procedure TUnivIoSpiFlash.RunTransfer(atxdata : TBytes; readrx : Boolean);
var
  r : integer;
begin
  txlen := length(atxdata);
  comm.WriteBlob($C000, mpram_txoffs, atxdata[0], txlen);
  comm.WriteU16($1601, 0, txlen);
  comm.WriteU8($1602, 0, 1);
  while comm.ReadU8($1602, 0) <> 0 do
  begin
    // wait
  end;
  if readrx then
  begin
    r := comm.ReadBlob($C000, mpram_rxoffs, rxbuf[0], txlen);
    rxlen := txlen;
  end
  else
  begin
    rxlen := 0;
  end;
end;

end.

