unit form_main;

{$mode objfpc}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, StdCtrls, Buttons, ExtCtrls,
  udo_comm, commh_udosl, commh_udoip, uio_iohandler,
  frame_dout, frame_din, frame_ain, frame_pwm, frame_ledblp;

type

  { Tfrm_main }

  Tfrm_main = class(TForm)
    btnConnect : TBitBtn;
    edComPort : TEdit;
    Label1 : TLabel;
    Label2 : TLabel;
    pnl : TPanel;
    txtDeviceInfo : TStaticText;
    grpDOUT : TGroupBox;
    frameDOUT0 : TframeDOUT;
    grpDIN : TGroupBox;
    frameDIN0 : TframeDIN;
    grpAIN : TGroupBox;
    frameAIN0 : TframeAIN;
    timer : TTimer;
    grpLEDBLP : TGroupBox;
    grpPWM : TGroupBox;
    framePWM0 : TframePWM;
    frameLEDBLP0 : TframeLEDBLP;
    pnlAdc : TPanel;
    lbAINRange : TLabel;
    edADCRange : TEdit;
    Label6 : TLabel;
    btnMoreInfo : TSpeedButton;
    procedure btnConnectClick(Sender : TObject);

    procedure FormCreate(Sender : TObject);
    procedure timerTimer(Sender : TObject);
    procedure edADCRangeChange(Sender : TObject);
    procedure btnMoreInfoClick(Sender : TObject);
  private

  public
    ioh : TUnivioHandler;

    flist_dout : array of TframeDOUT;
    flist_din  : array of TframeDIN;
    flist_ain  : array of TframeAIN;
    flist_pwm  : array of TframePWM;
    flist_ledblp : array of TframeLEDBLP;

    procedure Connect;
    procedure Disconnect;

    procedure CleanupFrames;

    procedure Prepare_DOUT;
    procedure Prepare_DIN;
    procedure Prepare_AIN;
    procedure Prepare_PWM;
    procedure Prepare_LEDBLP;

    procedure UpdateData;

    procedure ReadDeviceId;
    procedure ReadStatus;

    function ReadString(aobj : uint16) : string;
  end;

var
  frm_main : Tfrm_main;

implementation

uses
  iohandler_base, form_more_info;

{$R *.lfm}

{ Tfrm_main }

procedure Tfrm_main.btnConnectClick(Sender : TObject);
begin
  timer.Enabled := false;;
  Disconnect;
  Connect;
end;

procedure Tfrm_main.FormCreate(Sender : TObject);
begin
  ioh := TUnivIoHandler.Create(udocomm);
  flist_dout := [];

{$ifndef WINDOWS}
  edComPort.Text := '/dev/ttyACM0';
{$endif}
end;

procedure Tfrm_main.timerTimer(Sender : TObject);
begin
  UpdateData;
end;

procedure Tfrm_main.edADCRangeChange(Sender : TObject);
var
  rv : integer;
  fra : TframeAIN;
begin
  rv := StrToIntDef(edADCRange.Text, 3300);
  for fra in flist_ain do
  begin
    fra.maxrange_mV := rv;
  end;
  UpdateData;
end;

procedure Tfrm_main.Connect;
begin
  if udocomm.Opened then EXIT;

  udosl_commh.devstr := edComPort.Text;
  udocomm.SetHandler(udosl_commh);
  try
    udocomm.Open();
  except on e : Exception do
    begin
      MessageDlg('Comm Error', 'Error opening comport '+edComPort.Text+': '+e.Message, mtError, [mbAbort], 0);
      EXIT;
    end;
  end;

  pnl.Visible := true;
  btnMoreInfo.Visible := true;;

  ReadDeviceId;

  ioh.LoadConfigFromDevice;

  Prepare_DOUT;
  Prepare_DIN;
  Prepare_AIN;
  Prepare_PWM;
  Prepare_LEDBLP;

  ReadStatus;

  timer.Enabled := True;
end;

procedure Tfrm_main.Disconnect;
begin
  udocomm.Close;
  pnl.Visible := false;
  btnMoreInfo.Visible := false;
  CleanupFrames;
end;

procedure Tfrm_main.CleanupFrames;
var
  i : integer;
begin
  for i := 1 to length(flist_dout)-1 do flist_dout[i].Free;
  for i := 1 to length(flist_din)-1  do flist_din[i].Free;
  for i := 1 to length(flist_ain)-1  do flist_ain[i].Free;
  for i := 1 to length(flist_pwm)-1  do flist_pwm[i].Free;
  for i := 1 to length(flist_ledblp)-1  do flist_ledblp[i].Free;
end;

procedure Tfrm_main.Prepare_DOUT;
var
  line : TDigOutLine;
  fra, refra : TframeDOUT;
  n : integer;
begin
  flist_dout := [];

  refra := frameDOUT0;
  refra.Visible := false;

  for n := 0 to length(ioh.dout_lines) - 1 do
  begin
    line := ioh.dout_lines[n];
    if n = 0 then
    begin
      fra := refra;
    end
    else
    begin
      fra := TframeDOUT.Create(self);
      fra.Parent := refra.Parent;
      fra.Left := refra.Left;
      fra.Name := 'frame'+line.id;
      fra.Top := refra.Top + n * refra.Height;
    end;
    fra.line := line;
    fra.lb.Caption := IntToStr(n);
    fra.visible := true;

    insert(fra, flist_dout, length(flist_dout));
  end;

end;

procedure Tfrm_main.Prepare_DIN;
var
  line : TDigInLine;
  fra, refra : TframeDIN;
  n : integer;
begin
  flist_din := [];

  refra := frameDIN0;
  refra.Visible := false;

  for n := 0 to length(ioh.din_lines) - 1 do
  begin
    line := ioh.din_lines[n];
    if n = 0 then
    begin
      fra := refra;
    end
    else
    begin
      fra := TframeDIN.Create(self);
      fra.Parent := refra.Parent;
      fra.Left := refra.Left;
      fra.Name := 'frame'+line.id;
      fra.Top := refra.Top + n * refra.Height;
    end;
    fra.line := line;
    fra.lb.Caption := IntToStr(n);
    fra.visible := true;

    insert(fra, flist_din, length(flist_din));
  end;

end;

procedure Tfrm_main.Prepare_AIN;
var
  line : TAnaInLine;
  fra, refra : TframeAIN;
  n : integer;
begin
  flist_ain := [];

  refra := frameAIN0;
  refra.Visible := false;

  for n := 0 to length(ioh.ain_lines) - 1 do
  begin
    line := ioh.ain_lines[n];
    if n = 0 then
    begin
      fra := refra;
    end
    else
    begin
      fra := TframeAIN.Create(self);
      fra.Parent := refra.Parent;
      fra.Left := refra.Left;
      fra.Name := 'frame'+line.id;
      fra.Top := refra.Top + n * refra.Height;
    end;
    fra.line := line;
    fra.lb.Caption := IntToStr(n);
    fra.Visible := true;

    insert(fra, flist_ain, length(flist_ain));
  end;
end;

procedure Tfrm_main.Prepare_PWM;
var
  line : TPwmLine;
  fra, refra : TframePWM;
  n : integer;
begin
  flist_pwm := [];

  refra := framePWM0;
  refra.Visible := false;

  for n := 0 to length(ioh.pwm_lines) - 1 do
  begin
    line := ioh.pwm_lines[n];
    if n = 0 then
    begin
      fra := refra;
    end
    else
    begin
      fra := TframePWM.Create(self);
      fra.Parent := refra.Parent;
      fra.Left := refra.Left;
      fra.Name := 'frame'+line.id;
      fra.Top := refra.Top + n * refra.Height;
    end;
    fra.line := line;
    fra.lb.Caption := IntToStr(n);
    fra.Visible := true;

    insert(fra, flist_pwm, length(flist_pwm));
  end;
end;

procedure Tfrm_main.Prepare_LEDBLP;
var
  line : TLedBlpLine;
  fra, refra : TframeLEDBLP;
  n : integer;
begin
  flist_ledblp := [];

  refra := frameLEDBLP0;
  refra.Visible := false;

  for n := 0 to length(ioh.ledblp_lines) - 1 do
  begin
    line := ioh.ledblp_lines[n];
    if n = 0 then
    begin
      fra := refra;
    end
    else
    begin
      fra := TframeLEDBLP.Create(self);
      fra.Parent := refra.Parent;
      fra.Left := refra.Left;
      fra.Name := 'frame'+line.id;
      fra.Top := refra.Top + n * refra.Height;
    end;
    fra.line := line;
    fra.lb.Caption := IntToStr(n);
    fra.visible := true;

    insert(fra, flist_ledblp, length(flist_ledblp));
  end;
end;

procedure Tfrm_main.UpdateData;
var
  i : integer;
begin
  for i := 0 to length(flist_din)-1
  do
    flist_din[i].UpdateFromLine;

  for i := 0 to length(flist_ain)-1
  do
    flist_ain[i].UpdateFromLine;
end;

procedure Tfrm_main.ReadStatus;
var
  i : integer;
begin

  for i := 0 to length(flist_dout)-1
  do
    flist_dout[i].UpdateFromLine;

  for i := 0 to length(flist_pwm)-1
  do
    flist_pwm[i].UpdateFromLine;

  for i := 0 to length(flist_ledblp)-1
  do
    flist_ledblp[i].UpdateFromLine;

  UpdateData;
end;

function Tfrm_main.ReadString(aobj : uint16) : string;
var
  s : string;
  rlen : integer;
begin
  s := '';
  SetLength(s, 1024);
  try
    rlen := udocomm.UdoRead(aobj, 0, s[1], length(s));
  except
    on e : Exception do
    begin
      result := '[read error: '+e.message+']';
      exit;
    end;
  end;

  result := trim(copy(s, 1, rlen))
end;

procedure Tfrm_main.ReadDeviceId;
begin
  txtDeviceInfo.Caption := ReadString($0181);
end;

procedure Tfrm_main.btnMoreInfoClick(Sender : TObject);
var
  frm : TfrmMoreInfo;
begin
  Application.CreateForm(TfrmMoreInfo, frm);
  frm.memo.Clear;

  frm.memo.Append('Device Implementation ID: "'+ReadString($0101)+'", version: '+IntToHex(udocomm.ReadU32($0102, 0)));
  frm.memo.Append('');
  frm.memo.Append('Device ID:    "'+ReadString($0181)+'"');
  frm.memo.Append('');
  frm.memo.Append('USB Vendor ID:  '+IntToHex(udocomm.ReadU32($0182, 0)));
  frm.memo.Append('USB Product ID: '+IntToHex(udocomm.ReadU32($0183, 0)));
  frm.memo.Append('Manufacturer Name: "'+ReadString($0184)+'"');
  frm.memo.Append('Serial Number:     "'+ReadString($0185)+'"');
  frm.memo.Append('');

  frm.ShowModal;
  frm.Free;
end;


end.

