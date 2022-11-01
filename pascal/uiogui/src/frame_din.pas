unit frame_din;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, StdCtrls, iohandler_base;

type

  { TframeDIN }

  TframeDIN = class(TFrame)
    lb : TLabel;
    txt : TStaticText;
  private

  public
    line : TDigInLine;

    constructor Create(aowner: TComponent); override;

    procedure UpdateFromLine;
    procedure SetStatus(avalue : boolean);

  end;

implementation

{$R *.lfm}

{ TframeDIN }

constructor TframeDIN.Create(aowner : TComponent);
begin
  inherited;
  line := nil;
end;

procedure TframeDIN.UpdateFromLine;
begin
  if line = nil then Exit;

  SetStatus(line.Value());
end;

procedure TframeDIN.SetStatus(avalue : boolean);
begin
  if avalue
  then
      txt.Caption := '1 - ON'
  else
      txt.Caption := '0 - off';
end;

end.

