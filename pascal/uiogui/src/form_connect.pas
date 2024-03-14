unit form_connect;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, StdCtrls, Grids,
  Buttons, ExtCtrls, udo_comm, commh_udosl, commh_udoip, com_port_lister, jsontools;

type

  { TfrmConnect }

  TfrmConnect = class(TForm)
    Label1 : TLabel;
    grid : TStringGrid;
    Label2 : TLabel;
    edIpAddr : TEdit;
    btnRefresh : TBitBtn;
    btnConnectSerial : TBitBtn;
    Bevel1 : TBevel;
    btnConnectIp : TBitBtn;
    btnClose : TBitBtn;
    procedure FormCreate(Sender : TObject);
    procedure btnConnectSerialClick(Sender : TObject);
    procedure btnConnectIpClick(Sender : TObject);
    procedure gridDblClick(Sender : TObject);
    procedure btnRefreshClick(Sender : TObject);
  private

  public

    last_serial_port : string;

    procedure LoadSetup;
    procedure SaveSetup;

    procedure RefreshSerialDevices;
  end;

var
  frmConnect : TfrmConnect;

implementation

uses
  form_main;

{$R *.lfm}

{ TfrmConnect }

procedure TfrmConnect.FormCreate(Sender : TObject);
begin
  last_serial_port := '';
  LoadSetup;
  RefreshSerialDevices;
end;

procedure TfrmConnect.btnConnectSerialClick(Sender : TObject);
begin
  if udocomm.Opened then udocomm.Close;

  udosl_commh.devstr := grid.Cells[0, grid.Row];
  udocomm.SetHandler(udosl_commh);

  try
    udocomm.Open();
  except on e : Exception do
    begin
      MessageDlg('Comm Error', 'Error opening UnivIO Device at '+udosl_commh.devstr+': '+e.Message, mtError, [mbAbort], 0);
      EXIT;
    end;
  end;

  last_serial_port := udosl_commh.devstr;

  SaveSetup;

  Application.CreateForm(Tfrm_main, frm_main);
  frm_main.txtDevAddr.Caption := udosl_commh.devstr;
  frm_main.Show;

  self.Hide;
end;

procedure TfrmConnect.btnConnectIpClick(Sender : TObject);
begin
  if udocomm.Opened then udocomm.Close;

  udoip_commh.ipaddrstr := edIpAddr.Text;
  udocomm.SetHandler(udoip_commh);

  try
    udocomm.Open();
  except on e : Exception do
    begin
      MessageDlg('Comm Error', 'Error opening UnivIO Device at '+udoip_commh.ipaddrstr+': '+e.Message, mtError, [mbAbort], 0);
      EXIT;
    end;
  end;

  SaveSetup;

  Application.CreateForm(Tfrm_main, frm_main);
  frm_main.txtDevAddr.Caption := udoip_commh.ipaddrstr;
  frm_main.Show;

  self.Hide;
end;

procedure TfrmConnect.gridDblClick(Sender : TObject);
begin
  btnConnectSerial.Click;
end;

procedure TfrmConnect.btnRefreshClick(Sender : TObject);
begin
  RefreshSerialDevices;
end;

procedure TfrmConnect.LoadSetup;
var
  jroot, jn : TJsonNode;
begin
  jroot := TJsonNode.Create;
  try
    jroot.LoadFromFile('uiogui.conf');
  except
    ;
  end;

  if jroot.Find('IPADDR', jn) then edIpAddr.Text := jn.AsString;
  if jroot.Find('LAST_SERIAL_PORT', jn) then last_serial_port := jn.AsString;
  jroot.Free;
end;

procedure TfrmConnect.SaveSetup;
var
  jroot : TJsonNode;
begin
  jroot := TJsonNode.Create;
  try
    jroot.LoadFromFile('uiogui.conf');
  except
    ;
  end;
  jroot.Add('IPADDR', edIpAddr.Text);
  jroot.Add('LAST_SERIAL_PORT', last_serial_port);
  jroot.SaveToFile('uiogui.conf');
  jroot.Free;
end;

procedure TfrmConnect.RefreshSerialDevices;
var
  cli : TComPortListItem;
  row : integer;
  desc : string;
begin
  grid.RowCount := comportlister.CollectComPorts + 1;
  row := 1;
  for cli in comportlister.items do
  begin
    grid.Cells[0, row] := cli.devstr;
    grid.Cells[1, row] := IntToStr(cli.interfacenum);
    grid.Cells[2, row] := cli.serial_number;
    grid.Cells[3, row] := IntToHex(cli.usb_vid, 4) + ':' + IntToHex(cli.usb_pid, 4);
    desc := cli.interfacename;
    if desc = '' then desc := cli.description;
    grid.Cells[4, row] := desc;

    if cli.devstr = last_serial_port then grid.Row := row;

    Inc(row);
  end;
end;


end.

