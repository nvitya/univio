unit iohandler_base;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils;

type
  TIoLineType = (IOLT_UNKNOWN, IOLT_DIGIN, IOLT_DIGOUT, IOLT_ANAIN, IOLT_ANAOUT, IOLT_PWM, IOLT_LEDBLP);

  TIoHandler = class;

  { TIoLine }

  TIoLine = class
  public
    id : string;
    handler  : TIoHandler;
    linetype : TIoLineType;
    unitid   : integer;

    constructor Create(aid : string; ahandler : TIoHandler; aunitid : integer); virtual;
  end;

  { TDigInLine }

  TDigInLine = class(TIoLine)
  public
    constructor Create(aid : string; ahandler : TIoHandler; aunitid : integer); override;

    function Value() : boolean; virtual;
  end;

  { TDigOutLine }

  TDigOutLine = class(TIoLine)
  public
    constructor Create(aid : string; ahandler : TIoHandler; aunitid : integer); override;

    function  Value() : boolean; virtual;
    procedure SetTo(avalue : boolean); virtual;
  end;

  { TAnaInLine }

  TAnaInLine = class(TIoLine)
  public
    constructor Create(aid : string; ahandler : TIoHandler; aunitid : integer); override;

    function  Value() : integer; virtual;
  end;

  { TAnaOutLine }

  TAnaOutLine = class(TIoLine)
  public
    constructor Create(aid : string; ahandler : TIoHandler; aunitid : integer); override;

    function  Value() : integer; virtual;
    procedure SetTo(avalue : integer); virtual;
  end;

  { TPwmLine }

  TPwmLine = class(TIoLine)
  public
    constructor Create(aid : string; ahandler : TIoHandler; aunitid : integer); override;

    function  Frequency() : integer; virtual;
    function  Duty() : integer; virtual;
    procedure SetFrequency(avalue : integer); virtual;
    procedure SetDuty(avalue : uint16); virtual;
  end;

  { TLedBlpLine }

  TLedBlpLine = class(TIoLine)
  public
    constructor Create(aid : string; ahandler : TIoHandler; aunitid : integer); override;

    function  Value() : integer; virtual;
    procedure SetTo(avalue : uint32); virtual;
  end;


  { TIoHandler }

  TIoHandler = class
  public
    linelist : array of TIoLine;

    constructor Create(); virtual;

    procedure ClearLines; virtual;

    function  GetIoLine(aid : string) : TIoLine;

    function  GetDigIn(aid : string) : TDigInLine;
    function  GetDigOut(aid : string) : TDigOutLine;
    function  GetAnaIn(aid : string) : TAnaInLine;
    function  GetAnaOut(aid : string) : TAnaOutLine;

    function  GetPwm(aid : string) : TPwmLine;
    function  GetLedBlp(aid : string) : TLedBlpLine;

  public
    procedure SetDigOutValue(aline : TIoLine; avalue : boolean); virtual;
    function  GetDigOutValue(aline : TIoLine) : boolean; virtual;
    function  GetDigInValue(aline : TIoLine) : boolean; virtual;
    function  GetAnaInValue(aline : TIoLine) : integer; virtual;
    function  GetAnaOutValue(aline : TIoLine) : integer; virtual;
    procedure SetAnaOutValue(aline : TIoLine; avalue : integer); virtual;

    function  GetPwmFrequency(aline : TIoLine) : integer; virtual;
    procedure SetPwmFrequency(aline : TIoLine; avalue : integer); virtual;
    function  GetPwmDuty(aline : TIoLine) : uint16; virtual;
    procedure SetPwmDuty(aline : TIoLine; avalue : uint16); virtual;
    function  GetLedBlpValue(aline : TIoLine) : uint32; virtual;
    procedure SetLedBlpValue(aline : TIoLine; avalue : uint32); virtual;

  protected
    function AddIoLine(aline : TIoLine) : TIoLine;

  end;


implementation

{ TIoLine }

constructor TIoLine.Create(aid : string; ahandler : TIoHandler; aunitid : integer);
begin
  id := aid;
  linetype := IOLT_UNKNOWN;
  handler := ahandler;
  unitid := aunitid;
end;

{ TAnaInLine }

constructor TAnaInLine.Create(aid : string; ahandler : TIoHandler; aunitid : integer);
begin
  inherited;
  linetype := IOLT_ANAIN;
end;

function TAnaInLine.Value : integer;
begin
  result := handler.GetAnaInValue(self);
end;

{ TAnaOutLine }

constructor TAnaOutLine.Create(aid : string; ahandler : TIoHandler; aunitid : integer);
begin
  inherited;
  linetype := IOLT_ANAOUT;
end;

procedure TAnaOutLine.SetTo(avalue : integer);
begin
  handler.SetAnaOutValue(self, avalue);
end;

function TAnaOutLine.Value : integer;
begin
  result := handler.GetAnaOutValue(self);
end;

{ TDigOutLine }

constructor TDigOutLine.Create(aid : string; ahandler : TIoHandler; aunitid : integer);
begin
  inherited;
  linetype := IOLT_DIGOUT;
end;

function TDigOutLine.Value : boolean;
begin
  result := handler.GetDigOutValue(self);
end;

procedure TDigOutLine.SetTo(avalue : boolean);
begin
  handler.SetDigOutValue(self, avalue);
end;

{ TDigInLine }

constructor TDigInLine.Create(aid : string; ahandler : TIoHandler; aunitid : integer);
begin
  inherited;
  linetype := IOLT_DIGIN;
end;

function TDigInLine.Value : boolean;
begin
  result := handler.GetDigInValue(self);
end;

{ TLedBlpLine }

constructor TLedBlpLine.Create(aid : string; ahandler : TIoHandler; aunitid : integer);
begin
  inherited;
  linetype := IOLT_LEDBLP;
end;

function TLedBlpLine.Value : integer;
begin
  result := handler.GetLedBlpValue(self);
end;

procedure TLedBlpLine.SetTo(avalue : uint32);
begin
  handler.SetLedBlpValue(self, avalue);
end;

{ TPwmLine }

constructor TPwmLine.Create(aid : string; ahandler : TIoHandler; aunitid : integer);
begin
  inherited;
  linetype := IOLT_PWM;
end;

function TPwmLine.Frequency : integer;
begin
  result := handler.GetPwmFrequency(self);
end;

function TPwmLine.Duty : integer;
begin
  result := handler.GetPwmDuty(self);
end;

procedure TPwmLine.SetFrequency(avalue : integer);
begin
  handler.SetPwmFrequency(self, avalue);
end;

procedure TPwmLine.SetDuty(avalue : uint16);
begin
  handler.SetPwmDuty(self, avalue);
end;

{ TIoHandler }

constructor TIoHandler.Create;
begin
  linelist := [];
end;

procedure TIoHandler.ClearLines;
var
  line : TIoLine;
begin
  for line in linelist do line.Free;
  linelist := [];
end;

function TIoHandler.GetIoLine(aid : string) : TIoLine;
begin
  for result in linelist do
  begin
    if result.id = aid then EXIT;
  end;

  raise Exception.Create('IO Line "'+aid+'" was not found!');
end;

function TIoHandler.GetDigIn(aid : string) : TDigInLine;
var
  iol : TIoLine;
begin
  iol := self.GetIoLine(aid);
  if iol is TDigInLine then
  begin
    result := TDigInLine(iol);
  end
  else raise Exception.Create('IO Line "'+aid+'" type is not Digital Input!');
end;

function TIoHandler.GetDigOut(aid : string) : TDigOutLine;
var
  iol : TIoLine;
begin
  iol := self.GetIoLine(aid);
  if iol is TDigOutLine then
  begin
    result := TDigOutLine(iol);
  end
  else raise Exception.Create('IO Line "'+aid+'" type is not Digital Output!');
end;

function TIoHandler.GetAnaIn(aid : string) : TAnaInLine;
var
  iol : TIoLine;
begin
  iol := self.GetIoLine(aid);
  if iol is TAnaInLine then
  begin
    result := TAnaInLine(iol);
  end
  else raise Exception.Create('IO Line "'+aid+'" type is not Analogue Input!');
end;

function TIoHandler.GetAnaOut(aid : string) : TAnaOutLine;
var
  iol : TIoLine;
begin
  iol := self.GetIoLine(aid);
  if iol is TAnaOutLine then
  begin
    result := TAnaOutLine(iol);
  end
  else raise Exception.Create('IO Line "'+aid+'" type is not Analogue Output!');
end;

function TIoHandler.GetPwm(aid : string) : TPwmLine;
var
  iol : TIoLine;
begin
  iol := self.GetIoLine(aid);
  if iol is TPwmLine then
  begin
    result := TPwmLine(iol);
  end
  else raise Exception.Create('IO Line "'+aid+'" type is not PWM Output!');
end;

function TIoHandler.GetLedBlp(aid : string) : TLedBlpLine;
var
  iol : TIoLine;
begin
  iol := self.GetIoLine(aid);
  if iol is TLedBlpLine then
  begin
    result := TLedBlpLine(iol);
  end
  else raise Exception.Create('IO Line "'+aid+'" type is not LEDBLP Output!');
end;

procedure TIoHandler.SetDigOutValue(aline : TIoLine; avalue : boolean);
begin
  if (aline <> nil) or avalue then ;
end;

function TIoHandler.GetDigOutValue(aline : TIoLine) : boolean;
begin
  if aline <> nil then ;
  result := false;
end;

function TIoHandler.GetDigInValue(aline : TIoLine) : boolean;
begin
  if aline <> nil then ;
  result := false;
end;

function TIoHandler.GetAnaInValue(aline : TIoLine) : integer;
begin
  if aline <> nil then ;
  result := 0;
end;

function TIoHandler.GetAnaOutValue(aline : TIoLine) : integer;
begin
  if aline <> nil then ;
  result := 0;
end;

procedure TIoHandler.SetAnaOutValue(aline : TIoLine; avalue : integer);
begin
  if aline <> nil then ;
  if avalue <> 0 then ;
end;

function TIoHandler.GetPwmFrequency(aline : TIoLine) : integer;
begin
  if aline <> nil then ;
  result := 0;
end;

procedure TIoHandler.SetPwmFrequency(aline : TIoLine; avalue : integer);
begin
  if aline <> nil then ;
  if avalue <> 0 then ;
end;

function TIoHandler.GetPwmDuty(aline : TIoLine) : uint16;
begin
  if aline <> nil then ;
  result := 0;
end;

procedure TIoHandler.SetPwmDuty(aline : TIoLine; avalue : uint16);
begin
  if aline <> nil then ;
  if avalue <> 0 then ;
end;

function TIoHandler.GetLedBlpValue(aline : TIoLine) : uint32;
begin
  if aline <> nil then ;
  result := 0;
end;

procedure TIoHandler.SetLedBlpValue(aline : TIoLine; avalue : uint32);
begin
  if aline <> nil then ;
  if avalue <> 0 then ;
end;

function TIoHandler.AddIoLine(aline : TIoLine) : TIoLine;
begin
  insert(aline, linelist, length(linelist));
  result := aline;
end;

end.

