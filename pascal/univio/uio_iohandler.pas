unit uio_iohandler;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, iohandler_base, univio_conn;

type

  { TUnivIoHandler }

  TUnivIoHandler = class(TIoHandler)
  public
    uio     : TUnivioConn;

    din_lines  : array of TDigInLine;
    dout_lines : array of TDigOutLine;
    ain_lines  : array of TAnaInLine;
    aout_lines : array of TAnaOutLine;
    pwm_lines  : array of TPwmLine;
    ledblp_lines : array of TLedBlpLine;

    constructor Create(auio : TUnivioConn); reintroduce;

    procedure ClearLines; override;

    procedure LoadConfigFromDevice;

    function  ReadUint32(const aobjid : uint16) : uint32;
    function  ReadUint16(const aobjid : uint16) : uint16;
    procedure WriteUint32(const aobjid : uint16; const avalue : uint32);
    procedure WriteUint16(const aobjid : uint16; const avalue : uint16);

  public
    procedure SetDigOutValue(aline : TIoLine; avalue : boolean); override;
    function  GetDigOutValue(aline : TIoLine) : boolean; override;
    function  GetDigInValue(aline : TIoLine) : boolean; override;
    function  GetAnaInValue(aline : TIoLine) : integer; override;
    function  GetAnaOutValue(aline : TIoLine) : integer; override;
    procedure SetAnaOutValue(aline : TIoLine; avalue : integer); override;

    function  GetPwmFrequency(aline : TIoLine) : integer; override;
    procedure SetPwmFrequency(aline : TIoLine; avalue : integer); override;
    function  GetPwmDuty(aline : TIoLine) : uint16; override;
    procedure SetPwmDuty(aline : TIoLine; avalue : uint16); override;
    function  GetLedBlpValue(aline : TIoLine) : uint32; override;
    procedure SetLedBlpValue(aline : TIoLine; avalue : uint32); override;
  end;

implementation

{ TUnivIoHandler }

constructor TUnivIoHandler.Create(auio : TUnivioConn);
begin
  inherited Create;
  uio := auio;

  ClearLines;
end;

procedure TUnivIoHandler.ClearLines;
begin
  inherited ClearLines;

  din_lines := [];
  dout_lines := [];
  ain_lines := [];
  aout_lines := [];
  pwm_lines := [];
  ledblp_lines := [];
end;

procedure TUnivIoHandler.LoadConfigFromDevice;
var
  cbits  : uint32;
  unitid : uint8;
  line   : TIoLine;
begin
  ClearLines;

  cbits := ReadUint32($0E01); // get the DINs
  for unitid := 0 to 31 do
  begin
    if (cbits and (1 shl unitid)) <> 0 then
    begin
      line := AddIoLine( TDigInLine.Create('DIN'+IntToStr(unitid), self, unitid) );
      insert(line, din_lines, length(din_lines));
    end;
  end;

  cbits := ReadUint32($0E02); // get the DOUTs
  for unitid := 0 to 31 do
  begin
    if (cbits and (1 shl unitid)) <> 0 then
    begin
      line := AddIoLine( TDigOutLine.Create('DOUT'+IntToStr(unitid), self, unitid) );
      insert(line, dout_lines, length(dout_lines));
    end;
  end;

  cbits := ReadUint32($0E03); // get the AINs / ADC
  for unitid := 0 to 31 do
  begin
    if (cbits and (1 shl unitid)) <> 0 then
    begin
      line := AddIoLine( TAnaInLine.Create('AIN'+IntToStr(unitid), self, unitid) );
      insert(line, ain_lines, length(ain_lines));
    end;
  end;

  cbits := ReadUint32($0E04); // get the AOUTs / DAC
  for unitid := 0 to 31 do
  begin
    if (cbits and (1 shl unitid)) <> 0 then
    begin
      line := AddIoLine( TAnaOutLine.Create('AOUT'+IntToStr(unitid), self, unitid) );
      insert(line, aout_lines, length(aout_lines));
    end;
  end;

  cbits := ReadUint32($0E05); // get the PWMs
  for unitid := 0 to 31 do
  begin
    if (cbits and (1 shl unitid)) <> 0 then
    begin
      line := AddIoLine( TPwmLine.Create('PWM'+IntToStr(unitid), self, unitid) );
      insert(line, pwm_lines, length(pwm_lines));
    end;
  end;

  cbits := ReadUint32($0E06); // get the LEDBLPs
  for unitid := 0 to 31 do
  begin
    if (cbits and (1 shl unitid)) <> 0 then
    begin
      line := AddIoLine( TLedBlpLine.Create('LEDBLP'+IntToStr(unitid), self, unitid) );
      insert(line, ledblp_lines, length(ledblp_lines));
    end;
  end;

end;

function TUnivIoHandler.ReadUint32(const aobjid : uint16) : uint32;
var
  r : uint16;
begin
  r := uio.ReadUint32(aobjid, result);
  if r <> 0 then
  begin
    raise Exception.Create(format('UnivIO Read(%.4X) error: %.4X', [aobjid, r]));
  end;
end;

function TUnivIoHandler.ReadUint16(const aobjid : uint16) : uint16;
var
  r : uint16;
begin
  r := uio.ReadUint16(aobjid, result);
  if r <> 0 then
  begin
    raise Exception.Create(format('UnivIO Read(%.4X) error: %.4X', [aobjid, r]));
  end;
end;

procedure TUnivIoHandler.WriteUint32(const aobjid : uint16; const avalue : uint32);
var
  r : uint16;
begin
  r := uio.WriteUint32(aobjid, avalue);
  if r <> 0 then
  begin
    raise Exception.Create(format('UnivIO Write(%.4X, %.8X) error: %.4X', [aobjid, avalue, r]));
  end;
end;

procedure TUnivIoHandler.WriteUint16(const aobjid : uint16; const avalue : uint16);
var
  r : uint16;
begin
  r := uio.WriteUint16(aobjid, avalue);
  if r <> 0 then
  begin
    raise Exception.Create(format('UnivIO Write(%.4X, %.4X) error: %.4X', [aobjid, avalue, r]));
  end;
end;

procedure TUnivIoHandler.SetDigOutValue(aline : TIoLine; avalue : boolean);
var
  objid : uint16;
  v : uint32;
begin
  if aline.unitid >= 16 then
  begin
    objid := $1001;
  end
  else
  begin
    objid := $1000;
  end;

  v := (1 shl (aline.unitid and $0F));
  if not avalue then v := v shl 16;

  WriteUint32(objid, v);
end;

function TUnivIoHandler.GetDigOutValue(aline : TIoLine) : boolean;
var
  v : uint32;
begin
  v := ReadUint32($1010);
  result := (0 <> (v and (1 shl aline.unitid)));
end;

function TUnivIoHandler.GetDigInValue(aline : TIoLine) : boolean;
var
  v : uint32;
begin
  v := ReadUint32($1100);
  result := (0 <> (v and (1 shl aline.unitid)));
end;

function TUnivIoHandler.GetAnaInValue(aline : TIoLine) : integer;
begin
  result := ReadUint16($1200 + aline.unitid);
end;

function TUnivIoHandler.GetAnaOutValue(aline : TIoLine) : integer;
begin
  result := ReadUint16($1300 + aline.unitid);
end;

procedure TUnivIoHandler.SetAnaOutValue(aline : TIoLine; avalue : integer);
begin
  WriteUint16($1300 + aline.unitid, avalue);
end;

function TUnivIoHandler.GetPwmFrequency(aline : TIoLine) : integer;
begin
  result := ReadUint32($0700 + aline.unitid);
end;

procedure TUnivIoHandler.SetPwmFrequency(aline : TIoLine; avalue : integer);
begin
  WriteUint32($0700 + aline.unitid, avalue);
end;

function TUnivIoHandler.GetPwmDuty(aline : TIoLine) : uint16;
begin
  result := ReadUint16($1400 + aline.unitid);
end;

procedure TUnivIoHandler.SetPwmDuty(aline : TIoLine; avalue : uint16);
begin
  WriteUint16($1400 + aline.unitid, avalue);
end;

function TUnivIoHandler.GetLedBlpValue(aline : TIoLine) : uint32;
begin
  result := ReadUint32($1500 + aline.unitid);
end;

procedure TUnivIoHandler.SetLedBlpValue(aline : TIoLine; avalue : uint32);
begin
  WriteUint32($1500 + aline.unitid, avalue);
end;


end.

