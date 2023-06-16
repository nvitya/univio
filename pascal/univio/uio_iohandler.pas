unit uio_iohandler;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, iohandler_base, udo_comm;

type

  { TUnivIoHandler }

  TUnivIoHandler = class(TIoHandler)
  public
    comm     : TUdoComm;

    din_lines  : array of TDigInLine;
    dout_lines : array of TDigOutLine;
    ain_lines  : array of TAnaInLine;
    aout_lines : array of TAnaOutLine;
    pwm_lines  : array of TPwmLine;
    ledblp_lines : array of TLedBlpLine;

    constructor Create(acomm : TUdoComm); reintroduce;

    procedure ClearLines; override;

    procedure LoadConfigFromDevice;

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

constructor TUnivIoHandler.Create(acomm : TUdoComm);
begin
  inherited Create;
  comm := acomm;

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

  cbits := comm.ReadUint32($0E01, 0); // get the DINs
  for unitid := 0 to 31 do
  begin
    if (cbits and (1 shl unitid)) <> 0 then
    begin
      line := AddIoLine( TDigInLine.Create('DIN'+IntToStr(unitid), self, unitid) );
      insert(line, din_lines, length(din_lines));
    end;
  end;

  cbits := comm.ReadUint32($0E02, 0); // get the DOUTs
  for unitid := 0 to 31 do
  begin
    if (cbits and (1 shl unitid)) <> 0 then
    begin
      line := AddIoLine( TDigOutLine.Create('DOUT'+IntToStr(unitid), self, unitid) );
      insert(line, dout_lines, length(dout_lines));
    end;
  end;

  cbits := comm.ReadUint32($0E03, 0); // get the AINs / ADC
  for unitid := 0 to 31 do
  begin
    if (cbits and (1 shl unitid)) <> 0 then
    begin
      line := AddIoLine( TAnaInLine.Create('AIN'+IntToStr(unitid), self, unitid) );
      insert(line, ain_lines, length(ain_lines));
    end;
  end;

  cbits := comm.ReadUint32($0E04, 0); // get the AOUTs / DAC
  for unitid := 0 to 31 do
  begin
    if (cbits and (1 shl unitid)) <> 0 then
    begin
      line := AddIoLine( TAnaOutLine.Create('AOUT'+IntToStr(unitid), self, unitid) );
      insert(line, aout_lines, length(aout_lines));
    end;
  end;

  cbits := comm.ReadUint32($0E05, 0); // get the PWMs
  for unitid := 0 to 31 do
  begin
    if (cbits and (1 shl unitid)) <> 0 then
    begin
      line := AddIoLine( TPwmLine.Create('PWM'+IntToStr(unitid), self, unitid) );
      insert(line, pwm_lines, length(pwm_lines));
    end;
  end;

  cbits := comm.ReadUint32($0E06, 0); // get the LEDBLPs
  for unitid := 0 to 31 do
  begin
    if (cbits and (1 shl unitid)) <> 0 then
    begin
      line := AddIoLine( TLedBlpLine.Create('LEDBLP'+IntToStr(unitid), self, unitid) );
      insert(line, ledblp_lines, length(ledblp_lines));
    end;
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

  comm.WriteUint32(objid, 0, v);
end;

function TUnivIoHandler.GetDigOutValue(aline : TIoLine) : boolean;
var
  v : uint32;
begin
  v := comm.ReadUint32($1010, 0);
  result := (0 <> (v and (1 shl aline.unitid)));
end;

function TUnivIoHandler.GetDigInValue(aline : TIoLine) : boolean;
var
  v : uint32;
begin
  v := comm.ReadUint32($1100, 0);
  result := (0 <> (v and (1 shl aline.unitid)));
end;

function TUnivIoHandler.GetAnaInValue(aline : TIoLine) : integer;
begin
  result := comm.ReadUint16($1200 + aline.unitid, 0);
end;

function TUnivIoHandler.GetAnaOutValue(aline : TIoLine) : integer;
begin
  result := comm.ReadUint16($1300 + aline.unitid, 0);
end;

procedure TUnivIoHandler.SetAnaOutValue(aline : TIoLine; avalue : integer);
begin
  comm.WriteUint16($1300 + aline.unitid, 0, avalue);
end;

function TUnivIoHandler.GetPwmFrequency(aline : TIoLine) : integer;
begin
  result := comm.ReadUint32($0700 + aline.unitid, 0);
end;

procedure TUnivIoHandler.SetPwmFrequency(aline : TIoLine; avalue : integer);
begin
  comm.WriteUint32($0700 + aline.unitid, 0, avalue);
end;

function TUnivIoHandler.GetPwmDuty(aline : TIoLine) : uint16;
begin
  result := comm.ReadUint16($1400 + aline.unitid, 0);
end;

procedure TUnivIoHandler.SetPwmDuty(aline : TIoLine; avalue : uint16);
begin
  comm.WriteUint16($1400 + aline.unitid, 0, avalue);
end;

function TUnivIoHandler.GetLedBlpValue(aline : TIoLine) : uint32;
begin
  result := comm.ReadUint32($1500 + aline.unitid, 0);
end;

procedure TUnivIoHandler.SetLedBlpValue(aline : TIoLine; avalue : uint32);
begin
  comm.WriteUint32($1500 + aline.unitid, 0, avalue);
end;


end.

