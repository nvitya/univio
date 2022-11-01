unit frame_ain;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, StdCtrls, iohandler_base;

type

  { TframeAIN }

  TframeAIN = class(TFrame)
    lb : TLabel;
    txt : TStaticText;
  private

  public
    line : TAnaInLine;
    maxrange_mV : integer;

    constructor Create(aowner: TComponent); override;

    procedure UpdateFromLine;
    procedure SetStatus(avalue : uint16);

  end;

implementation

{$R *.lfm}

{ TframeAIN }

constructor TframeAIN.Create(aowner : TComponent);
begin
  inherited Create(aowner);
  maxrange_mV := 3300;
end;

procedure TframeAIN.UpdateFromLine;
begin
  if line = nil then Exit;

  SetStatus(line.Value());
end;

procedure TframeAIN.SetStatus(avalue : uint16);
var
  mv : integer;
begin
  mv := round(maxrange_mV * avalue / 65535);
  txt.Caption := IntToStr(mv);
end;

end.

