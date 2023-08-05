unit form_spi;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, ExtCtrls, StdCtrls,
  Spin, jsontools, strparseobj, udo_comm;

type

  { TfrmSpi }

  TfrmSpi = class(TForm)
    pnlTop : TPanel;
    Bevel1 : TBevel;
    pnlBot : TPanel;
    memoLog : TMemo;
    memoWData : TMemo;
    Label1 : TLabel;
    edSpeed : TSpinEdit;
    Label5 : TLabel;
    edWriteOffs : TSpinEdit;
    timerStatus : TTimer;
    btnStart : TButton;
    Label6 : TLabel;
    edReadOffs : TSpinEdit;
    Label2 : TLabel;
    Memo1 : TMemo;
    procedure FormClose(Sender : TObject; var CloseAction : TCloseAction);
    procedure FormCreate(Sender : TObject);
    procedure btnStartClick(Sender : TObject);
    procedure FormDestroy(Sender : TObject);
    procedure timerStatusTimer(Sender : TObject);
  private

  public

    sp : TStrParseObj;
    wdata : TBytes;

    function ParseWriteData : TBytes;

    procedure LoadSetup;
    procedure SaveSetup;

  end;

var
  frmSpi : TfrmSpi;

procedure ShowSpiWindow();

implementation

{$R *.lfm}

procedure ShowSpiWindow();
begin
  if frmSpi = nil then
  begin
    Application.CreateForm(TfrmSpi, frmSpi);
  end;
  frmSpi.Show;
end;

{ TfrmSpi }

procedure TfrmSpi.FormClose(Sender : TObject; var CloseAction : TCloseAction);
begin
  SaveSetup;
  CloseAction := caFree;
  frmSpi := nil
end;

procedure TfrmSpi.FormCreate(Sender : TObject);
begin
  wdata := [];

  LoadSetup;
end;

procedure TfrmSpi.FormDestroy(Sender : TObject);
begin
  wdata := [];
end;

procedure TfrmSpi.timerStatusTimer(Sender : TObject);
var
  st : byte;
  rdata : array of byte;
  rlen : integer;
  bcnt : integer;
  s : string;
  b : byte;
begin

  timerStatus.Enabled := false;
  try
    st := udocomm.ReadU16($1602, 0);
  except
    on e : Exception do
    begin
      memoLog.Append('Status query error! :'+e.Message);
      timerStatus.Enabled := false;
    end;
  end;

  //memoLog.Append(format('  ... st=%.4X', [st]));

  if st <> 0 then  // still running
  begin
    timerStatus.Interval := 250;
    timerStatus.Enabled := true;
    exit; // keep on querying
  end;

  rdata := [];
  SetLength(rdata, length(wdata));
  rlen := udocomm.ReadBlob($C000, edReadOffs.Value, rdata[0], length(rdata));
  SetLength(rdata, rlen);

  s := '';
  bcnt := 0;
  for b in rdata do
  begin
    s := s + format(' %.2X', [b]);
    inc(bcnt);
    if bcnt >= 16 then
    begin
      memoLog.Append(' <<< '+s);
      s := '';
      bcnt := 0;
    end;
  end;
  if s <> '' then memoLog.Append(' <<< '+s);
end;

function TfrmSpi.ParseWriteData : TBytes;
var
  i, n, m : integer;
begin
  result := [];

  while sp.readptr < sp.bufend do
  begin
    sp.SkipSpaces();
    if sp.ReadAlphaNum() then
    begin
      i := sp.PrevHexToInt();
      insert(i, result, length(result));
    end
    else if sp.CheckSymbol('*') then
    begin
      sp.SkipSpaces();
      if sp.ReadAlphaNum() then
      begin
        m := sp.PrevToInt();
        for n := 1 to m - 1 do // one times already added
        begin
          insert(i, result, length(result));
        end;
      end;
    end
    else
    begin
      // unknown token
      exit;
    end;
  end;
end;

procedure TfrmSpi.btnStartClick(Sender : TObject);
var
  wdatastr : string;
  b : byte;
  bcnt : dword;
  s : string;
begin
  SaveSetup;

  wdatastr := memoWData.Text;
  sp.Init(wdatastr);

  wdata := ParseWriteData;

  memoLog.Append(format('%u bytes SPI R+W @ %u Hz', [length(wdata), edSpeed.Value]));

  s := '';
  bcnt := 0;
  for b in wdata do
  begin
    s := s + format(' %.2X', [b]);
    inc(bcnt);
    if bcnt >= 16 then
    begin
      memoLog.Append(' >>> '+s);
      s := '';
      bcnt := 0;
    end;
  end;
  if s <> '' then memoLog.Append(' >>> '+s);

  udocomm.WriteU32($1600, 0, edSpeed.Value);
  udocomm.WriteU16($1604, 0, edWriteOffs.Value);
  udocomm.WriteU16($1605, 0, edReadOffs.Value);

  udocomm.WriteU16($1601, 0, length(wdata));

  // upload write data
  udocomm.WriteBlob($C000, edWriteOffs.Value, wdata[0], length(wdata));

  udocomm.WriteU8($1602, 0, 1); // start the transaction

  memoLog.Append(' ---');

  timerStatus.Interval := 50;  // shorter wait time first
  timerStatus.Enabled := True;
end;

procedure TfrmSpi.LoadSetup;
var
  jroot, jn : TJsonNode;
begin
  jroot := TJsonNode.Create;
  try
    jroot.LoadFromFile('spi.conf');
  except
    ;
  end;

  if jroot.Find('SPEED',     jn) then edSpeed.Value := round(jn.AsNumber);
  if jroot.Find('WRITEOFFS', jn) then edWriteOffs.Value := round(jn.AsNumber);
  if jroot.Find('READOFFS',  jn) then edReadOffs.Value := round(jn.AsNumber);
  if jroot.Find('WRITEDATA', jn) then memoWData.Text := jn.AsString;
  jroot.Free;
end;

procedure TfrmSpi.SaveSetup;
var
  jroot : TJsonNode;
begin
  jroot := TJsonNode.Create;
  jroot.Add('SPEED',      edSpeed.Value);
  jroot.Add('WRITEOFFS',  edWriteOffs.Value);
  jroot.Add('READOFFS',   edReadOffs.Value);
  jroot.Add('WRITEDATA',  memoWData.Text);

  jroot.SaveToFile('spi.conf');
  jroot.Free;
end;

initialization
begin
  frmSpi := nil;
end;

end.

