unit univio_connection;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils,
  udo_common, udo_comm, commh_udosl;

type

  { TUnivioConnection }

  TUnivioConnection = class
  public
    ict_serno : string;

    constructor Create;

    procedure Connect(aict_id : ansistring);

    function GetConnectStr : string;
  end;

var
  univio_conn : TUnivioConnection;

implementation

uses
  com_port_lister;

{ TUnivioConnection }

constructor TUnivioConnection.Create;
begin
  ict_serno := '';
end;

procedure CutAtFirstZero(var s : ansistring);
var
  p : integer;
begin
  p := Pos(#0, s);
  if p > 0 then SetLength(s, p-1);  // keep only up to char before #0
end;

procedure TUnivioConnection.Connect(aict_id : ansistring);
var
  cp : TComPortListItem;
  sarr : TStringArray;
  sno : ansistring = '';
  label try_again;
begin
try_again:
  ict_serno := '';
  comportlister.CollectComPorts;
  for cp in comportlister.items do
  begin
    sarr := cp.serial_number.Split('#');
    if (length(sarr) >= 2) and (sarr[0] = aict_id) and (0 = cp.interfacenum) then
    begin
      ict_serno := cp.serial_number;

      // open the serial communication
      udosl_commh.devstr := cp.devstr;
      udocomm.SetHandler(udosl_commh);

      try
        udocomm.Open;
      except
        on e : EUdoAbort do
        begin
          if e.ecode <> UDOERR_CONNECTION then raise;
        end;
      end;

      EXIT;
    end;
  end;

  raise Exception.Create(aict_id+' is not connected.');
end;

function TUnivioConnection.GetConnectStr : string;
var
  s : string;
begin
  if udosl_commh.Opened then s := udosl_commh.devstr
                        else s := udocomm.commh.ConnString;
  result := ict_serno + ' at '+s;
end;

initialization
begin
  univio_conn := TUnivioConnection.Create;
end;

end.

