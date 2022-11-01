unit frame_dout;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, Dialogs, Forms, Controls, StdCtrls, Buttons, iohandler_base;

type

  { TframeDOUT }

  TframeDOUT = class(TFrame)
    btnOn : TSpeedButton;
    btnOff : TSpeedButton;
    lb : TLabel;
    procedure btnOffClick(Sender : TObject);
    procedure btnOnClick(Sender : TObject);
  private

  public
    line : TDigOutLine;

    constructor Create(aowner: TComponent); override;

    procedure UpdateFromLine;
    procedure SetStatus(avalue : boolean);
  end;

implementation

{$R *.lfm}

{ TframeDOUT }

constructor TframeDOUT.Create(aowner : TComponent);
begin
  inherited;
  line := nil;
end;

procedure TframeDOUT.btnOffClick(Sender : TObject);
begin
  SetStatus(false);
  if line <> nil then
  begin
    line.SetTo(false);
  end;
end;

procedure TframeDOUT.btnOnClick(Sender : TObject);
begin
  SetStatus(true);
  if line <> nil then
  begin
    line.SetTo(true);
  end;
end;

procedure TframeDOUT.SetStatus(avalue : boolean);
begin
  //cbStatus.Checked := avalue;
  btnOn.Down := avalue;
  btnOff.Down := not avalue;
end;

procedure TframeDOUT.UpdateFromLine;
begin
  if line = nil then Exit;

  SetStatus(line.Value());
end;

end.

