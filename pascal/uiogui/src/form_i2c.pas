unit form_i2c;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, StdCtrls, Spin,
  ExtCtrls, StrUtils, udo_comm, jsontools;
type

  { TfrmI2c }

  TfrmI2c = class(TForm)
    Label1 : TLabel;
    edSpeed : TSpinEdit;
    Label2 : TLabel;
    edDevAddr : TEdit;
    Label3 : TLabel;
    edExAddr : TEdit;
    rbRead : TRadioButton;
    rbWrite : TRadioButton;
    edReadCount : TSpinEdit;
    Label4 : TLabel;
    pnlBot : TPanel;
    Bevel1 : TBevel;
    memoLog : TMemo;
    btnStart : TButton;
    edWriteData : TEdit;
    timerStatus : TTimer;
    Label5 : TLabel;
    edMPRAMOffs : TSpinEdit;
    procedure FormClose(Sender : TObject; var CloseAction : TCloseAction);
    procedure FormActivate(Sender : TObject);
    procedure btnStartClick(Sender : TObject);
    procedure timerStatusTimer(Sender : TObject);
    procedure FormCreate(Sender : TObject);
  private

  public
    comm     : TUdoComm;

    procedure LoadSetup;
    procedure SaveSetup;

  end;

var
  frmI2c : TfrmI2c;

procedure ShowI2cWindow(acomm : TUdoComm);

implementation

procedure ShowI2cWindow(acomm : TUdoComm);
begin
  if frmI2c = nil then
  begin
    Application.CreateForm(TfrmI2c, frmI2c);
  end;
  frmI2c.comm := acomm;
  frmI2c.Show;
end;

{$R *.lfm}

{ TfrmI2c }

procedure TfrmI2c.FormClose(Sender : TObject; var CloseAction : TCloseAction);
begin
  SaveSetup;
  CloseAction := caFree;
  frmI2c := nil
end;

procedure TfrmI2c.FormActivate(Sender : TObject);
begin
  edSpeed.Value := comm.ReadU32($1700, 0);
end;

procedure TfrmI2c.btnStartClick(Sender : TObject);
var
  sarr : array of string;
  wbytes : array of byte;
  cmd : cardinal;
  d : cardinal;
  s : string;
  excnt : cardinal;
  exaddr  : cardinal;
  exaddrinfo : string;
  devaddr : cardinal;
begin
  SaveSetup;
{
  if comm.ReadU16($1703, 0) = $FFFF then
  begin
    memoLog.Append('START ERROR: The Previous transaction not finished yet');
    exit;
  end;
}

  devaddr := Hex2Dec(edDevAddr.Text);
  excnt := (length(edExAddr.Text) + 1) div 2;

  exaddrinfo := '';
  if excnt > 0 then
  begin
    exaddr := Hex2Dec(edExAddr.Text);
    exaddrinfo := ', exaddr=' + format('%.'+IntToStr(2*excnt)+'X', [exaddr]);
  end;

  wbytes := [];
  sarr := string(edWriteData.Text).split(' ');
  for s in sarr do
  begin
    if s <> '' then
    begin
      try
        d := Hex2Dec(s);
        insert(d and $FF, wbytes, length(wbytes));
      except
        ; // ignore
      end;
    end;
  end;

  if length(wbytes) < 1 then
  begin
    MessageDlg('Error', 'Specify some write bytes', mtError, [mbAbort], 0);
    edWriteData.SetFocus;
    exit;
  end;

  if rbRead.Checked then
  begin
    cmd := (devaddr shl 1) or (excnt shl 12) or (edReadCount.Value shl 16);
    memoLog.Append(format('READ: speed=%u, dev=%.2X%s', [edSpeed.Value, devaddr, exaddrinfo]));
  end
  else
  begin
    cmd := 1 or (devaddr shl 1) or (excnt shl 12) or (length(wbytes) shl 16);
    memoLog.Append(format('WRITE: speed=%u, dev=%.2X%s', [edSpeed.Value, devaddr, exaddrinfo]));
    s := '';
    for d in wbytes do s := s + format(' %.2X', [d]);
    memoLog.Append(' >>> '+s);
  end;

  comm.WriteU32($1700, 0, edSpeed.Value);
  comm.WriteU32($1701, 0, exaddr);
  comm.WriteU16($1704, 0, edMPRAMOffs.Value);

  if rbWrite.Checked then
  begin
    comm.WriteBlob($C000, edMPRAMOffs.Value, wbytes[0], length(wbytes));
  end;

  comm.WriteU32($1702, 0, cmd);

  timerStatus.Interval := 50;  // shorter wait time first
  timerStatus.Enabled := True;
end;

procedure TfrmI2c.timerStatusTimer(Sender : TObject);
var
  st : cardinal;
  rdata : array of byte;
  s : string;
  b : byte;
begin

  timerStatus.Enabled := false;
  try
    st := comm.ReadU16($1703, 0);
  except
    on e : Exception do
    begin
      memoLog.Append('Status query error! :'+e.Message);
      timerStatus.Enabled := false;
    end;
  end;

  //memoLog.Append(format('  ... st=%.4X', [st]));

  if st = $FFFF then  // still running
  begin
    timerStatus.Interval := 250;
    timerStatus.Enabled := true;
    exit; // keep on querying
  end;

  if st = 0 then
  begin
    if rbRead.Checked then
    begin
      rdata := [];
      SetLength(rdata, edReadCount.Value);
      comm.UdoRead($C000, edMPRAMOffs.Value, rdata[0], length(rdata));
      SetLength(rdata, edReadCount.Value);
      s := '';
      for b in rdata do s := s + format(' %.2X', [b]);
      memoLog.Append(' <<< '+s);
    end
    else
    begin
      memoLog.Append('  OK');
    end;
  end
  else
  begin
    memoLog.Append(format('  ERROR: %.4X', [st]));
  end;
end;

procedure TfrmI2c.FormCreate(Sender : TObject);
begin
  LoadSetup;
end;

procedure TfrmI2c.LoadSetup;
var
  jroot, jn : TJsonNode;
begin
  jroot := TJsonNode.Create;
  try
    jroot.LoadFromFile('i2c.conf');
  except
    ;
  end;

  if jroot.Find('SPEED',     jn) then edSpeed.Value := round(jn.AsNumber);
  if jroot.Find('MPRAMOFFS', jn) then edMPRAMOffs.Value := round(jn.AsNumber);
  if jroot.Find('READCOUNT', jn) then edReadCount.Value := round(jn.AsNumber);

  if jroot.Find('DEVADDR',   jn) then edDevAddr.Text := jn.AsString;
  if jroot.Find('EXADDR',    jn) then edExAddr.Text := jn.AsString;
  if jroot.Find('WRITEDATA', jn) then edWriteData.Text := jn.AsString;

  jroot.Free;
end;

procedure TfrmI2c.SaveSetup;
var
  jroot : TJsonNode;
begin
  jroot := TJsonNode.Create;
  jroot.Add('SPEED',      edSpeed.Value);
  jroot.Add('DEVADDR',    edDevAddr.Text);
  jroot.Add('EXADDR',     edExAddr.Text);
  jroot.Add('MPRAMOFFS',  edMPRAMOffs.Value);
  jroot.Add('ISWRITE',    rbWrite.Checked);
  jroot.Add('READCOUNT',  edReadCount.Value);
  jroot.Add('WRITEDATA',  edWriteData.Text);

  jroot.SaveToFile('i2c.conf');
  jroot.Free;
end;


initialization
begin
  frmI2c := nil;
end;

end.

