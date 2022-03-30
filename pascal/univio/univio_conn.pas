unit univio_conn;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, univio, sercomm, util_nstime;

type

  { TUnivioConn }

  TUnivioConn = class
  public
    constructor Create();
    destructor  Destroy; override;

  public
    serdevname : string;

    comm       : TSerComm;

    receive_timeout_us : integer;

  	rq         : TUnivioRequest;
    iserror    : boolean;

    function Open(adevname : string) : boolean;
    procedure Close();

    function Read(aaddr  : uint16; out pdst; adstlen : word; rlen : PInteger) : uint16;
    function Write(aaddr : uint16; const psrc; alen : word) : uint16;

    function ReadUint32(aaddr : word; out rdata : uint32) : uint16;
    function WriteUint32(aaddr : uint16; adata : UInt32) : uint16;

  protected
    bufcnt     : integer;
    rxcnt      : integer;
    rxreadpos  : integer;
    rxstate    : integer;
    crc        : byte;
    buf        : array[0..UNIVIO_MAX_DATA_LEN+63] of byte;
    lastrecvtime : int64;

    procedure ExecRequest();
    function  AddTx(const asrc; len : integer) : integer;
    function  TxAvailable() : integer;

  end;

implementation

{ TUnivioConn }

constructor TUnivioConn.Create;
begin
  comm := TSerComm.Create();
  comm.baudrate := 1000000;
  receive_timeout_us := 100000;  // 100 ms
  rxstate := 0;
  iserror := false;
end;

destructor TUnivioConn.Destroy;
begin
  comm.Free;
  inherited Destroy;
end;

function TUnivioConn.Open(adevname: string) : boolean;
begin
  serdevname := adevname;
  if not comm.Open(serdevname) then
  begin
    exit(false);
  end;
  result := true;
end;

procedure TUnivioConn.Close;
begin
  comm.Close();
end;

function TUnivioConn.Read(aaddr: uint16; out pdst; adstlen: word; rlen : PInteger) : uint16;
begin
	rq.address := aaddr;
	rq.metalen := 0;
	rq.length := adstlen;
	rq.iswrite := 0;

	ExecRequest();

	if 0 = rq.result then
  begin
		if rq.length > adstlen then
    begin
			rq.result := UIOERR_DATA_TOO_BIG;
			exit(rq.result);
		end;

		if rq.length < adstlen then
    begin
      PByte(@pdst)^ := 0; // to avoid silly FPC warning
			FillByte(pdst, adstlen, 0);
    end;

		move(rq.data[0], pdst, rq.length);

    if rlen <> nil then rlen^ := rq.length;
	end;

	result := rq.result;
end;

function TUnivioConn.Write(aaddr: uint16; const psrc; alen: word): uint16;
begin
	rq.address := aaddr;
	rq.metalen := 0;
	rq.length := alen;
	rq.iswrite := 1;
	if alen > sizeof(rq.data) then
  begin
		exit(UIOERR_DATA_TOO_BIG);
  end;

	move(psrc, rq.data[0], alen);

	ExecRequest();

	result := rq.result;
end;

procedure TUnivioConn.ExecRequest;
var
  r : integer;
  b : byte;
begin
	bufcnt := 0;
	crc := 0;

	comm.FlushInput();
	comm.FlushOutput();

	// prepare the request

	b := $55;
	AddTx(b, 1);
  b := (rq.iswrite and 1);
	if      2 = rq.metalen then b := b or (1 << 2)
	else if 4 = rq.metalen then b := b or (2 << 2)
	else if 8 = rq.metalen then b := b or (3 << 2);

	if rq.length >= 15 then
	begin
		b := b or $F0;
	end
	else
	begin
		b := b or (rq.length << 4);
	end;
	AddTx(b, 1);

	if rq.length >= 15 then
	begin
		AddTx(rq.length, 2);
	end;
	AddTx(rq.address, 2);

	if rq.metalen > 0 then
	begin
		AddTx(rq.metadata[0], rq.metalen);
	end;

	if (rq.iswrite <> 0) and (rq.length > 0) then
	begin
		AddTx(rq.data[0], rq.length);
	end;

	AddTx(crc, 1);

	// send the request

	r := comm.Write(buf[0], bufcnt);
	if r <= 0 then
	begin
		rq.result := UIOERR_CONNECTION;
		exit;
	end;

	if r <> bufcnt then
	begin
		rq.result := UIOERR_CONNECTION;
		exit;
	end;

	// receive the response

	bufcnt := 0;
	rxstate := 0;
	rxcnt := 0;
	rxreadpos := 0;
	crc := 0;

	lastrecvtime := nstime();

  while true do
  begin
  	r := comm.Read(buf[bufcnt], sizeof(buf) - bufcnt);
  	if r <= 0 then
  	begin
  		if r = -11 then  // 11 = EAGAIN
  		begin
  			if nstime() - lastrecvtime > 1000000000 then //receive_timeout_us * 1000)
  			begin
  				rq.result := UIOERR_CONNECTION;
  				exit;
  			end;
  			continue;
  		end;
  		rq.result := UIOERR_CONNECTION;
    	exit;
  	end;

  	lastrecvtime := nstime();

  	Inc(bufcnt, r);

  	while rxreadpos < bufcnt do
  	begin
			b := buf[rxreadpos];

			if (rxstate > 0) and (rxstate < 10) then
			begin
				crc := univio_calc_crc(crc, b);
			end;

			if 0 = rxstate then  // waiting for the sync byte
			begin
				if $55 = b then
				begin
					crc := univio_calc_crc(0, b); // start the CRC from zero
					rxstate := 1;
				end;
			end
			else if 1 = rxstate then // command and length
			begin
				rq.iswrite := (b and 1); // bit0: 0 = read, 1 = write
				iserror := ((b and 2) <> 0);

				// calculate the metadata length
				rq.metalen := (($8420 shr (b and $0C)) and $0F);  // this small code replaces a case construct
				rq.length := (b shr 4);
				rxcnt := 0;
				if 15 = rq.length then
				begin
					rxstate := 2;  // extended length follows
				end
				else
				begin
					rxstate := 3;  // address follows
				end;
			end
			else if 2 = rxstate then // extended length
			begin
				if 0 = rxcnt then
				begin
					rq.length := b; // low byte
					rxcnt := 1;
				end
				else
				begin
					rq.length := rq.length or (b shl 8); // high byte
					rxcnt := 0;
					rxstate := 3; // address follows
				end;
			end
			else if 3 = rxstate then // address
			begin
				if 0 = rxcnt then
				begin
					rq.address := b;  // address low
					rxcnt := 1;
				end
				else
				begin
					rq.address := rq.address or (b shl 8);  // address high
					rxcnt := 0;
					if rq.metalen > 0 then
					begin
						rxstate := 4;  // meta follows
					end
					else
					begin
						if (rq.iswrite = 0) or iserror then
						begin
							rxstate := 5;  // read data or error code
						end
						else
						begin
							rxstate := 10;  // then crc check
						end;
					end;
				end;
			end
			else if 4 = rxstate then // metadata
			begin
				rq.metadata[rxcnt] := b;
				Inc(rxcnt);
				if rxcnt >= rq.metalen then
				begin
					if (rq.iswrite = 0) or iserror then
					begin
						rxcnt := 0;
						rxstate := 5; // read data or error code follows
					end
					else
					begin
						rxstate := 10;  // crc check
					end;
				end;
			end
			else if 5 = rxstate then // read data or error code
			begin
				if iserror and (rq.length <> 2) then
				begin
					rq.length := 2;
				end;
				rq.data[rxcnt] := b;
				Inc(rxcnt);
				if rxcnt >= rq.length then
				begin
					rxstate := 10;
				end;
			end
			else if 10 = rxstate then // crc check
			begin
				if b <> crc then
				begin
					rq.result := UIOERR_CRC;
					exit;
				end
				else
				begin
					if iserror then	rq.result := PUint16(@rq.data[0])^
					           else	rq.result := 0;
					exit;
				end;
			end;

			Inc(rxreadpos);
  	end;
  end;
end;

function TUnivioConn.AddTx(const asrc; len : integer) : integer;
var
  available : integer;
  srcp, dstp, endp : PByte;
  b : byte;
begin
  available := TxAvailable();
  if 0 = available then exit(0);

  if len > available then len := available;

  srcp := PByte(@asrc);
  dstp := @buf[bufcnt];
  endp := dstp + len;
  while dstp < endp do
  begin
    b := srcp^;
    dstp^ := b;
    crc := univio_calc_crc(crc, b);
    Inc(srcp);
    Inc(dstp);
  end;

  bufcnt += len;

  result := len;
end;

function TUnivioConn.TxAvailable: integer;
begin
  result := sizeof(buf) - bufcnt;
end;

function TUnivioConn.ReadUint32(aaddr: word; out rdata: uint32) : uint16;
var
  rlen : integer;
begin
  result := Read(aaddr, rdata, 4, @rlen);
end;

function TUnivioConn.WriteUint32(aaddr: uint16; adata : UInt32): uint16;
var
  u32 : UInt32;
begin
  u32 := adata;
  result := Write(aaddr, u32, 4);
end;


end.

