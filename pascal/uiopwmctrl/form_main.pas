unit form_main;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, StdCtrls, Buttons, ExtCtrls, Spin,
  univio_conn;

type

  { Tfrm_main }

  Tfrm_main = class(TForm)
    btnConnect : TBitBtn;
    btnSetLedBLP : TBitBtn;
    btnSetPWM : TBitBtn;
    edComPort : TEdit;
    edLedBLP : TEdit;
    edPWMDuty : TSpinEdit;
    Label1 : TLabel;
    Label2 : TLabel;
    Label3 : TLabel;
    Label4 : TLabel;
    Label5 : TLabel;
    lbPWMDuty : TLabel;
    lbPwm : TLabel;
    pnl : TPanel;
    edPWMFreq : TSpinEdit;
    txtDeviceInfo : TStaticText;
    procedure btnConnectClick(Sender : TObject);
    procedure btnSetLED(Sender : TObject);
    procedure btnSetPWMClick(Sender : TObject);
    procedure FormCreate(Sender : TObject);
  private

  public
    uio : TUnivioConn;

    procedure Connect;
    procedure Disconnect;

    procedure ReadStatus;
  end;

var
  frm_main : Tfrm_main;

implementation

uses
  StrUtils;

{$R *.lfm}

{ Tfrm_main }

procedure Tfrm_main.btnConnectClick(Sender : TObject);
begin
  Disconnect;
  Connect;
end;

procedure Tfrm_main.btnSetLED(Sender : TObject);
var
  v : uint32;
begin
  v := Hex2Dec(edLedBLP.Text);
  uio.WriteUint32($1500, v);
  edLedBLP.Text := IntToHex(v, 8);
end;

procedure Tfrm_main.btnSetPWMClick(Sender : TObject);
begin
  uio.WriteUint32($0700, edPWMFreq.Value);
  uio.WriteUint32($1400, round(65535 * edPWMDuty.Value / 100));
end;

procedure Tfrm_main.FormCreate(Sender : TObject);
begin
  uio := TUnivioConn.Create();
end;

procedure Tfrm_main.Connect;
begin
  if uio.Opened then EXIT;

  if uio.Open(edComPort.Text) then
  begin
    pnl.Visible := true;
    ReadStatus;
  end
  else
  begin
    MessageDlg('Comm Error', 'Error opening comport '+edComPort.Text, mtError, [mbAbort], 0);
  end;
end;

procedure Tfrm_main.Disconnect;
begin
  uio.Close;
  pnl.Visible := false;
end;

procedure Tfrm_main.ReadStatus;
var
  s : string;
  n : uint32;
  rlen : integer;
begin
  s := '';
  SetLength(s, 1024);
  if uio.Read($0011, s[1], length(s), @rlen) = 0 then
  begin
    SetLength(s, rlen);
    txtDeviceInfo.Caption := s;
  end;

  if 0 = uio.ReadUint32($1500, n) then
  begin
    edLedBLP.Text := IntToHex(n, 8);
  end;

  if 0 = uio.ReadUint32($0700, n) then edPWMFreq.Value := n;
  if 0 = uio.ReadUint32($1400, n) then edPWMDuty.Value := round(100 * n / 65535);
end;

end.

