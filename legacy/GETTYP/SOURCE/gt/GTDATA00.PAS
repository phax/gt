{$i aflags.inc}

{ if it is BP 7, disabled the typesafe @ operator in this file }
{$ifdef VER70}
{$T-}
{$endif}
{$ifdef WIN32}
{$T-}
{$endif}

unit GTData00;

interface

uses
     GTCompar;

const

      COUNT_C2E = 15;

      REC_C2E:array[1..COUNT_C2E] of TC2ERec = (
      (
        Name:'Stefan Esser';
        Data:(
          $02, $00, $00, $00, $FF, $FF,
          $F0, $FF, $FE, $FF, $00, $00, $00, $01, $F0, $FF, $1C, $00, $00, $00,
          $00, $00, $00, $00
        )
      ),
      (
        Name:'Fabrice Bellard';
        Data:(
          $02, $00, $AA, $AA, $FF, $FF,
          $F0, $FF, $FE, $FF, $00, $00, $00, $01, $F0, $FF, $1C, $00, $00, $00,
          $00, $00, $00, $00
        )
      ),
      (
        Name:'cINOgEN';
        Data:(
          $02, $00, $00, $00, $FF, $FF,
          $F0, $FF, $FE, $FF, $00, $00, $00, $01, $F0, $FF, $1C, $00, $00, $00,
          $20, $20, $20, $20
        )
      ),
      (
        Name:'COMSEC';
        Data:(
          $05, $00, $AA, $AA, $FF, $FF,
          $F0, $FF, $FE, $FF, $00, $00, $00, $01, $F0, $FF, $1C, $00, $00, $00,
          $00, $00, $00, $00
        )
      ),
      (
        Name:'DblStar Software';
        Data:(
          $04, $00, $AA, $AA, $FF, $FF,
          $00, $00, $FE, $FF, $00, $00, $00, $01, $F0, $FF, $40, $00, $00, $00,
          $53, $54, $55, $42
        )
      ),
      (
        Name:'Elite x';
        Data:(
          $02, $00, $AA, $AA, $FF, $FF,
          $F0, $FF, $FE, $FF, $00, $00, $00, $01, $F0, $FF, $00, $00, $45, $4C,
          $49, $54, $45, $00
        )
      ),
      (
        Name:'HENDR­X';
        Data:(
          $02, $00, $00, $10, $FF, $FF,
          $F0, $FF, $FE, $FF, $00, $00, $00, $01, $F0, $FF, $1C, $00, $00, $00,
          $00, $00, $00, $00
        )
      ),
      (
        Name:'kaot';
        Data:(
          $02, $00, $00, $00, $FF, $FF,
          $F0, $FF, $FE, $FF, $00, $00, $00, $01, $F0, $FF, $1C, $00, $00, $00,
          $00, $00, $92, $4B
        )
      ),
      (
        Name:'Mess /E';
        Data:(
          $02, $00, $00, $00, $FF, $FF,
          $F0, $FF, $FE, $FF, $00, $00, $00, $01, $F0, $FF, $1C, $00, $00, $00,
          $4D, $45, $53, $53
        )
      ),
      (
        Name:'PHaX';
        Data:(
          $02, $00, $AA, $AA, $FF, $FF,
          $F0, $FF, $00, $00, $00, $00, $00, $01, $F0, $FF, $1C, $00, $00, $00,
          $50, $48, $41, $58
        )
      ),
      (
        Name:'SCRb2e';
        Data:(
          $20, $00, $AA, $00, $FF, $FF,
          $AA, $AA, $00, $AA, $00, $00, $00, $01, $F0, $FF, $00, $00, $00, $00,
          $00, $00, $00, $00
        )
      ),
      (
        Name:'Rose';
        Data:(
          $02, $00, $8F, $0D, $AA, $AA,
          $F0, $FF, $FE, $FF, $00, $00, $00, $01, $F0, $FF, $1C, $00, $00, $00,
          $00, $00, $52, $4F
        )
      ),
      (
        Name:'Trills nT';
        Data:(
          $02, $00, $D7, $0D, $FF, $FF,
          $F0, $FF, $FE, $FF, $00, $00, $00, $01, $F0, $FF, $1C, $00, $00, $54,
          $72, $69, $6C, $6C
        )
      ),
      (
        Name:'UNP x';
        Data:(
          $02, $00, $00, $00, $FF, $FF,
          $F0, $FF, $00, $00, $00, $00, $00, $01, $F0, $FF, $1C, $00, $00, $00,
          $00, $00, $00, $00
        )
      ),
      (
        Name:'Comvert 1.0';
        Data:(
          $04, $00, $01, $00, $FF, $FF, $F0, $FF, $FE, $FF, $AA, $AA, $AA, $02,
          $00, $00, $1C, $00, $00, $00, $AA, $02, $00, $00
        )
      )
      );

implementation

end.
