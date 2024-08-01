program uiocommtest;

uses
  SysUtils,
  udo_comm, commh_udosl, util_nstime, univio_spiflash;


var
  spiflash : TUnivIoSpiFlash;

procedure SpiTest;
var
  n : integer;
begin
  writeln('Testing SPI speed...');

  spiflash := TUnivIoSpiFlash.Create(udocomm);
  spiflash.Prepare(20000000);

  for n := 1 to 100 do
  begin
    spiflash.RunTransfer([$05, $00], false);
  end;

  sleep(20);

  for n := 1 to 100 do
  begin
    spiflash.RunTransfer([$05, $00], true);
  end;

  writeln('Spi Test finished.');
end;

procedure RunTest;
var
  comport : string;

  u32 : UInt32;
  iores : UInt16;

begin
  writeln('UnivIO Communication Test');

  //udosl_commh.devstr := 'COM14';
  udosl_commh.devstr := '/dev/ttyACM0';
  udocomm.SetHandler(udosl_commh);

  udocomm.Open;

  writeln('Com port ', udosl_commh.devstr, ' opened.');

	u32 := udocomm.ReadU32($0000, 0);
	//iores := conn.ReadU32($0000, u32);
	writeln(format('ReadU32(0x0000, 0) = %08X', [u32]));

 	// set the led blink pattern
	//iores := conn.WriteU32($1500, $F0F0F0F0);
	udocomm.WriteU32($1500, 0, $F0F0F055);

  SpiTest;

  udocomm.Close;
end;

begin
  RunTest;
  writeln('Test finished.');

  //writeln('Press ENTER to exit.');
  //readln;
end.

