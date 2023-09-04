unit form_more_info;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, ExtCtrls, StdCtrls,
  Buttons, udo_comm, udo_utils;

type

  { TfrmMoreInfo }

  TfrmMoreInfo = class(TForm)
    pnl : TPanel;
    memo : TMemo;
    btnClose : TBitBtn;
    procedure FormClose(Sender : TObject; var CloseAction : TCloseAction);
    procedure FormShow(Sender : TObject);
  private

  public
    pincnt   : uint32;
    portpins : uint32;

    function UnitsString(amask : cardinal) : string;
    function FormatPinConfig(d : cardinal) : string;
    function FormatPinName(n : cardinal) : string;

  end;

var
  frmMoreInfo : TfrmMoreInfo;

procedure ShowMoreInfo();

implementation

{$R *.lfm}

procedure ShowMoreInfo();
begin
  if frmMoreInfo = nil then
  begin
    Application.CreateForm(TfrmMoreInfo, frmMoreInfo);
  end;
  frmMoreInfo.Show;
end;

{ TfrmMoreInfo }

procedure TfrmMoreInfo.FormClose(Sender : TObject; var CloseAction : TCloseAction);
begin
  CloseAction := caFree;
  frmMoreInfo := nil
end;

procedure TfrmMoreInfo.FormShow(Sender : TObject);
var
  d, confbits : cardinal;
  s : string;
  n : integer;
begin
  memo.Clear;
  memo.Append('Device ID  : "'+UdoReadString($0181)+'"');
  memo.Append('Serial Num.: "'+UdoReadString($0185)+'"');
  memo.Append('Device Implementation ID: "'+UdoReadString($0101)+'", version: '+IntToHex(udocomm.ReadU32($0102, 0)));
  memo.Append('Manufacturer: "'+UdoReadString($0184)+'"');
  memo.Append(format('USB IDs: VID=%.4X, PID=%.4X', [
     udocomm.ReadU32($0182, 0), udocomm.ReadU32($0183, 0)
  ]));
  memo.Append(format('MPRAM: %u Bytes', [udocomm.ReadU32($0112, 0)]));

  memo.Append('Enabled Units:');
  memo.Append('  DIN    :'+UnitsString(udocomm.ReadU32($0E01, 0)));
  memo.Append('  DOUT   :'+UnitsString(udocomm.ReadU32($0E02, 0)));
  memo.Append('  ADC    :'+UnitsString(udocomm.ReadU32($0E03, 0)));
  memo.Append('  DAC    :'+UnitsString(udocomm.ReadU32($0E04, 0)));
  memo.Append('  PWM    :'+UnitsString(udocomm.ReadU32($0E05, 0)));
  memo.Append('  LEDBLP :'+UnitsString(udocomm.ReadU32($0E06, 0)));

  confbits := udocomm.ReadU32($0E00, 0);
  s := '';
  if 0 <> (confbits and $0001) then s += ' CLKOUT';
  if 0 <> (confbits and $0002) then s += ' UART';
  if 0 <> (confbits and $0004) then s += ' SPI';
  if 0 <> (confbits and $0008) then s += ' I2C';
  memo.Append('  SPEC.  :'+s);

  memo.Append('');
  memo.Append('Pin Configuration:');
  pincnt   := udocomm.ReadU32($0110, 0);
  portpins := udocomm.ReadU32($0111, 0);

  for n := 0 to pincnt - 1 do
  begin
    d := udocomm.ReadU32($0200 + n, 0);
    memo.Append(format('  Pin %s: %s', [FormatPinName(n), FormatPinConfig(d)]));
  end;

end;

function TfrmMoreInfo.UnitsString(amask : cardinal) : string;
var
  n : integer;
begin
  result := '';
  for n := 0 to 31 do
  begin
    if (amask and (1 shl n)) <> 0 then
    begin
      result += ' '+IntToStr(n);
    end;
  end;
end;

function TfrmMoreInfo.FormatPinConfig(d : cardinal) : string;
var
  ptype : integer;
  unitnum : integer;
  flags : word;
begin
  ptype := d and $FF;
  unitnum := (d shr 8) and $FF;
  flags := (d shr 16) and $FFFF;

  result := '?';
  if 0 = ptype then result := '-'
  else if 1 = ptype then result := 'DIG_IN'
  else if 2 = ptype then result := 'DIG_OUT'
  else if 3 = ptype then result := 'ANA_IN'
  else if 4 = ptype then result := 'ANA_OUT'
  else if 5 = ptype then result := 'PWM_OUT'
  else if 6 = ptype then result := 'LEDBLP'
  else if 7 = ptype then result := 'SPI'
  else if 8 = ptype then result := 'I2C'
  else if 9 = ptype then result := 'UART'
  else if 10 = ptype then result := 'CLKOUT'
  else result := '?';

  if ptype in [1,2,3,4,5,6] then
  begin
    result += '-'+IntToStr(unitnum);
    if flags <> 0 then result += format(' flags=%.4X', [flags]);
  end;
end;

function TfrmMoreInfo.FormatPinName(n: cardinal): string;
const
  PORT_CHARS : string[16] = 'ABCDEFGHIJKLMNOP';
var
  pnum : uint32;
begin
  if pincnt <= portpins then
  begin
    result := IntToStr(n);
  end
  else
  begin
    pnum := n div portpins;
    result := PORT_CHARS[pnum + 1] + IntToStr(n mod portpins);
  end;
end;


initialization
begin
  frmMoreInfo := nil;
end;

end.

