program cantest;

uses
  SysUtils,
  udo_comm, commh_udosl, util_nstime;

type
  TUioCanStatus = packed record
    status       : uint8;    // actual status flags
    ctrl         : uint8;    // actual conrol flags
    acterr_rx    : uint8;    // CAN std actual RX error counter (0-255)
    acterr_tx    : uint8;    // CAN std actual TX error counter (0-255)
    timestamp    : uint16;   // Actual CAN bit time counter, which is used for timestamping
    _reserved    : uint16;

    errcnt_ack   : uint32;   // Count of CAN ACK Errors
    errcnt_crc   : uint32;   // Count of CAN CRC Errors
    errcnt_form  : uint32;   // Count of CAN Frame Errors
    errcnt_stuff : uint32;   // Count of CAN Stuffing errors
    errcnt_bit0  : uint32;   // Count of CAN bit 0 sending error (bit 1 received)
    errcnt_bit1  : uint32;   // Count of CAN bit 1 sending error (bit 0 received)

    rx_cnt       : uint32;   // CAN messages received since the start
    tx_cnt       : uint32;   // CAN messages transmitted (queued) since the start
  end;

  TUioCanMsg = packed record
    can_id    : uint32;
    can_dlc   : uint8;
    __pad     : uint8;
    timestamp : uint16;
    data      : array[0..7] of byte;
  end;

var
  canst : TUioCanStatus;

procedure PrintCanMsg(asn : uint32; const msg : TUioCanMsg);
var
  i : integer;
begin
  write(format('%d. (%6d) %.3X ', [asn, msg.timestamp, msg.can_id]));
  for i := 0 to msg.can_dlc - 1 do
  begin
    write(' '+IntToHex(msg.data[i], 2));
  end;
  writeln;
end;

procedure CanTest;
var
  n : integer;
  u32 : uint32;
  r : integer;
  scnt : uint32;

  buf  : array[0..1023] of byte;
  pmsg : ^TUioCanMsg;
  pu32 : ^uint32;
  mcnt : integer;
  start_sn : uint32;

begin
  writeln('Testing CAN...');

  u32 := udocomm.ReadU32($1806, 0);
  writeln('CAN Rx buffer size: ', u32);

  // setup
  udocomm.WriteU32($1800, 0, 1000000); // Set Baudrate
  udocomm.WriteU32($1801, 0, $01);     // activate without RECVOWN

{$if 0}

  writeln('Polling status...');

  scnt := 0;

  while true do
  begin
    r := udocomm.UdoRead($1803, 0, canst, sizeof(canst));
    if r < 0 then
    begin
      writeln('Status read error: ',r);
      EXIT;
    end;

    writeln(scnt, ': CTRL=',canst.ctrl, ', RXCNT=' , canst.rx_cnt);

    Sleep(1000);
    inc(scnt);
  end;

{$endif}

  writeln('Reading messages:');
  r := udocomm.UdoRead($1808, 0, buf[0], sizeof(buf));
  writeln('  r = ',r);
  pu32 := @buf[0];
  writeln('  rxcnt=', pu32^);
  inc(pu32);
  writeln('  rxcnt_start=', pu32^);
  start_sn := pu32^;
  mcnt := (r - 8) div sizeof(TUioCanMsg);
  writeln('  mcnt = ', mcnt);

  pmsg := @buf[8];

  for n := 0 to mcnt - 1 do
  begin
    PrintCanMsg(start_sn + n, pmsg^);
    inc(pmsg);
  end;

  writeln('CAN Test finished.');
end;

procedure RunTest;
var
  u32 : UInt32;
begin
  writeln('USB-IO CAN Test');

  //udosl_commh.devstr := 'COM14';
  udosl_commh.devstr := '/dev/ttyACM1';
  udocomm.SetHandler(udosl_commh);

  udocomm.Open;

  writeln('Com port ', udosl_commh.devstr, ' opened.');

	u32 := udocomm.ReadU32($0000, 0);
	//iores := conn.ReadU32($0000, u32);
	writeln(format('ReadU32(0x0000, 0) = %08X', [u32]));

 	// set the led blink pattern
	//iores := conn.WriteU32($1500, $F0F0F0F0);
	//udocomm.WriteU32($1500, 0, $F0F0F055);

  CanTest;

  udocomm.Close;
end;

begin
  RunTest;
  writeln('Test finished.');

  //writeln('Press ENTER to exit.');
  //readln;
end.

