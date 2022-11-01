unit frame_pwm;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Spin, StdCtrls, Buttons, iohandler_base;

type

  { TframePWM }

  TframePWM = class(TFrame)
    edPWMFreq : TSpinEdit;
    Label4 : TLabel;
    edPWMDuty : TSpinEdit;
    Label5 : TLabel;
    btnSet : TBitBtn;
    lb : TLabel;
    procedure btnSetClick(Sender : TObject);
  private

  public
    line : TPwmLine;

    constructor Create(aowner: TComponent); override;

    procedure UpdateFromLine;
  end;

implementation

{$R *.lfm}

{ TframePWM }

procedure TframePWM.btnSetClick(Sender : TObject);
begin
  line.SetFrequency(edPWMFreq.Value);
  line.SetDuty(round(65535 * edPWMDuty.Value / 100));
end;

constructor TframePWM.Create(aowner : TComponent);
begin
  inherited Create(aowner);
  line := nil;
end;

procedure TframePWM.UpdateFromLine;
begin
  if line = nil then Exit;

  edPWMFreq.Value := line.Frequency();
  edPWMDuty.Value := (100 * line.Duty() / 65535);
end;

end.

