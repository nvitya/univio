unit frame_ledblp;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, StdCtrls, Buttons, iohandler_base;

type

  { TframeLEDBLP }

  TframeLEDBLP = class(TFrame)
    lb : TLabel;
    edLedBLP : TEdit;
    btnSet : TBitBtn;
    procedure btnSetClick(Sender : TObject);
  private

  public
    line : TLedBlpLine;

    constructor Create(aowner: TComponent); override;

    procedure UpdateFromLine;
    procedure SetStatus(avalue : uint32);

  end;

implementation

uses
  strutils;

{$R *.lfm}

{ TframeLEDBLP }

procedure TframeLEDBLP.btnSetClick(Sender : TObject);
var
  v : uint32;
begin
  v := Hex2Dec(edLedBLP.Text);
  line.SetTo(v);
  edLedBLP.Text := IntToHex(v, 8);
end;

constructor TframeLEDBLP.Create(aowner : TComponent);
begin
  inherited Create(aowner);
  line := nil
end;

procedure TframeLEDBLP.UpdateFromLine;
begin
  if line = nil then Exit;

  SetStatus(line.Value());
end;

procedure TframeLEDBLP.SetStatus(avalue : uint32);
begin
  edLedBLP.Text := IntToHex(avalue, 8);
end;

end.

