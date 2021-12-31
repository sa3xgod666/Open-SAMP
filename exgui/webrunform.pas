unit unit_webrunform;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls, Buttons;

type
  Twnd_webrunform = class(TForm)
    Label1: TLabel;
    BitBtn1: TBitBtn;
    BitBtn2: TBitBtn;
    BitBtn3: TBitBtn;
    procedure BitBtn2Click(Sender: TObject);
    procedure BitBtn3Click(Sender: TObject);
    procedure BitBtn1Click(Sender: TObject);

  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  wnd_webrunform: Twnd_webrunform;

implementation

{$R *.dfm}

procedure Twnd_webrunform.BitBtn2Click(Sender: TObject);
begin
     // TODO: BitBtn2Click
end;

procedure Twnd_webrunform.BitBtn3Click(Sender: TObject);
begin
    // TODO: BitBtn3Click
end;

procedure Twnd_webrunform.BitBtn1Click(Sender: TObject);
begin
    // TODO: BitBtn1Click
end;

end.
