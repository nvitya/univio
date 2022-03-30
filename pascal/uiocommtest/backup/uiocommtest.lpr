program uiocommtest;

uses
  SysUtils,
  univio, univio_conn, sercomm, util_nstime;

var
  conn : TUnivioConn;

procedure RunTest;
var
  comport : string;

  u32 : UInt32;
  iores : UInt16;

begin
  writeln('UnivIO Communication Test');

  comport := '/dev/ttyACM0';

  conn := TUnivioConn.Create;
  if not conn.Open(comport) then
  begin
    writeln('Error opening com port: ', comport);
    exit;
  end;

  writeln('Com port ', comport, ' opened.');

	iores := conn.ReadUint32($0000, u32);
	//iores := conn.ReadUint32($0000, u32);
	writeln(format('ReadUint32(0x0000) result = %04X, data = %08X', [iores, u32]));

 	// set the led blink pattern
	iores := conn.WriteUint32($1500, $FFFFFFFF);
end;

begin
  RunTest;
  conn.Free;
  writeln('Test finished.');
end.

