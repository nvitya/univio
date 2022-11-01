unit form_more_info;

{$mode ObjFPC}{$H+}

interface

uses
  Classes, SysUtils, Forms, Controls, Graphics, Dialogs, ExtCtrls, StdCtrls,
  Buttons;

type

  { TfrmMoreInfo }

  TfrmMoreInfo = class(TForm)
    pnl : TPanel;
    memo : TMemo;
    btnClose : TBitBtn;
  private

  public

  end;

var
  frmMoreInfo : TfrmMoreInfo;

implementation

{$R *.lfm}

end.

