unit univio;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils;

const
  UNIVIO_UART_BAUDRATE       = 1000000;
  UNIVIO_MAX_DATA_LEN        = 4096;

const
  UIOERR_CONNECTION          = $1001;  // not connected, send / receive error
  UIOERR_CRC                 = $1002;
  UIOERR_TIMEOUT             = $1003;
  UIOERR_DATA_TOO_BIG        = $1004;  // the provided buffer was too small to store the read response
                                       // the provided buffer for write was too big for the request

  UIOERR_WRONG_ADDR          = $2001;  // address not existing
  UIOERR_WRONG_ACCESS        = $2002;
  UIOERR_READ_ONLY           = $2003;
  UIOERR_WRITE_ONLY          = $2004;
  UIOERR_VALUE               = $2005;  // invalid value
  UIOERR_RUN_MODE            = $2006;  // config mode required
  UIOERR_UNITSEL             = $2007;  // the referenced unit is not existing

type
  TUnivioRequest = packed record
  //
    iswrite   : byte;
    metalen   : byte;
    result    : uint16;
    address   : uint16;
    length    : uint16;
    metadata  : array[0..7] of byte;
    data      : array[0..UNIVIO_MAX_DATA_LEN-1] of byte;
  end;

function univio_calc_crc(acrc : byte; adata : byte) : byte;

implementation

// CRC8 table with the standard polynom of 0x07:
const univio_crc_table : array[0..255] of byte =
(
  $00, $07, $0e, $09, $1c, $1b, $12, $15, $38, $3f, $36, $31, $24, $23, $2a, $2d,
  $70, $77, $7e, $79, $6c, $6b, $62, $65, $48, $4f, $46, $41, $54, $53, $5a, $5d,
  $e0, $e7, $ee, $e9, $fc, $fb, $f2, $f5, $d8, $df, $d6, $d1, $c4, $c3, $ca, $cd,
  $90, $97, $9e, $99, $8c, $8b, $82, $85, $a8, $af, $a6, $a1, $b4, $b3, $ba, $bd,
  $c7, $c0, $c9, $ce, $db, $dc, $d5, $d2, $ff, $f8, $f1, $f6, $e3, $e4, $ed, $ea,
  $b7, $b0, $b9, $be, $ab, $ac, $a5, $a2, $8f, $88, $81, $86, $93, $94, $9d, $9a,
  $27, $20, $29, $2e, $3b, $3c, $35, $32, $1f, $18, $11, $16, $03, $04, $0d, $0a,
  $57, $50, $59, $5e, $4b, $4c, $45, $42, $6f, $68, $61, $66, $73, $74, $7d, $7a,
  $89, $8e, $87, $80, $95, $92, $9b, $9c, $b1, $b6, $bf, $b8, $ad, $aa, $a3, $a4,
  $f9, $fe, $f7, $f0, $e5, $e2, $eb, $ec, $c1, $c6, $cf, $c8, $dd, $da, $d3, $d4,
  $69, $6e, $67, $60, $75, $72, $7b, $7c, $51, $56, $5f, $58, $4d, $4a, $43, $44,
  $19, $1e, $17, $10, $05, $02, $0b, $0c, $21, $26, $2f, $28, $3d, $3a, $33, $34,
  $4e, $49, $40, $47, $52, $55, $5c, $5b, $76, $71, $78, $7f, $6a, $6d, $64, $63,
  $3e, $39, $30, $37, $22, $25, $2c, $2b, $06, $01, $08, $0f, $1a, $1d, $14, $13,
  $ae, $a9, $a0, $a7, $b2, $b5, $bc, $bb, $96, $91, $98, $9f, $8a, $8d, $84, $83,
  $de, $d9, $d0, $d7, $c2, $c5, $cc, $cb, $e6, $e1, $e8, $ef, $fa, $fd, $f4, $f3
);

function univio_calc_crc(acrc : byte; adata : byte) : byte;
var
  idx : byte;
begin
  idx := (acrc xor adata);
  result := univio_crc_table[idx];
end;

end.

