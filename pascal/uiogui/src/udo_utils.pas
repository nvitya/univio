unit udo_utils;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, udo_comm;

function UdoReadString(aobj : uint16) : string;

implementation

function UdoReadString(aobj : uint16) : string;
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


end.

