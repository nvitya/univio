program uiogui;

{$mode objfpc}{$H+}

uses
  {$IFDEF UNIX}
  cthreads,
  {$ENDIF}
  {$IFDEF HASAMIGA}
  athreads,
  {$ENDIF}
  Interfaces, // this includes the LCL widgetset
  Forms, form_main, frame_dout, iohandler_base, frame_din, frame_ain, frame_pwm,
  frame_ledblp, form_more_info, form_i2c, udo_utils, form_spi, form_connect,
  version_uio_gui;

{$R *.res}

begin
  RequireDerivedFormResource := True;
  Application.Scaled := True;
  Application.Initialize;
  Application.CreateForm(TfrmConnect, frmConnect);
  Application.Run;
end.

