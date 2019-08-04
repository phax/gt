{$i aflags.inc}

{ if it is BP 7, disabled the typesafe @ operator in this file }
{$ifdef VER70}
{$T-}
{$endif}
{$ifdef WIN32}
{$T-}
{$endif}

unit GTDataNE;

interface

uses
     GTCompar;

const
      COUNT_NE = 5;
      COUNT_NE_01 = 0;

      STR_NE_1:string [21] = 'WinZIP Self Extractor';
      DAT_NE_1:array [1..80] of byte = (
        $33, $ED, $55, $9A, $FF, $FF, $00, $00, $0B, $C0, $74, $EE, $81, $C1,
        $00, $01, $72, $E8, $89, $0E, $90, $03, $89, $36, $92, $03, $89, $3E,
        $94, $03, $89, $1E, $96, $03, $8C, $06, $98, $03, $89, $16, $9A, $03,
        $33, $C0, $50, $9A, $FF, $FF, $00, $00, $FF, $36, $94, $03, $9A, $FF,
        $FF, $00, $00, $0B, $C0, $74, $BB, $FF, $36, $94, $03, $FF, $36, $92,
        $03, $FF, $36, $98, $03, $FF, $36, $96, $03, $FF
      );

      STR_NE_2:string [11] = 'PKLite 2.01';
      DAT_NE_2:array [1..78] of byte = (
        $41, $30, $00, $00, $FF, $FF, $00, $00, $FF, $FF, $00, $00, $00, $00,
        $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $FF, $FF, $00, $00,
        $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00,
        $FF, $2F, $00, $00, $FF, $2F, $00, $00, $FF, $2F, $00, $00, $FF, $2F,
        $00, $00, $FF, $2F, $00, $00, $FF, $2F, $00, $00, $FF, $2F, $00, $00,
        $FF, $2F, $FF, $FF, $00, $00, $FF, $FF
      );

      STR_NE_3:string [12] = 'Shrinker 3.2';
      DAT_NE_3:array [1..68] of byte = (
        $41, $30, $00, $00, $E8, $03, $FF, $FF, $54, $01, $06, $00, $00, $00,
        $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $E8, $00, $0A, $00,
        $00, $00, $00, $00, $00, $00, $00, $00, $53, $48, $52, $33, $01, $00,
        $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $00, $80, $00, $00,
        $01, $00, $00, $00, $09, $00, $00, $80, $01, $00, $01, $81
      );

      STR_NE_4:string [24] = 'Borland C++ Win16 (1991)';
      DAT_NE_4:array [1..80] of byte = (
        $9A, $FF, $FF, $00, $00, $0B, $C0, $75, $03, $E9, $D5, $00, $8C, $06,
        $16, $00, $89, $1E, $1C, $00, $89, $36, $1A, $00, $89, $3E, $18, $00,
        $89, $16, $1E, $00, $B8, $FF, $FF, $50, $9A, $FF, $FF, $00, $00, $33,
        $C0, $1E, $07, $BF, $DE, $03, $B9, $7E, $0A, $2B, $CF, $FC, $F3, $AA,
        $33, $C0, $50, $9A, $FF, $FF, $00, $00, $FF, $36, $18, $00, $9A, $FF,
        $FF, $00, $00, $0B, $C0, $75, $03, $E9, $91, $00
      );

      STR_NE_5:string [23] = 'PC Guard 1.50 for Win16';
      DAT_NE_5:array [1..80] of byte = (
        $50, $53, $51, $52, $56, $57, $55, $1E, $06, $06, $33, $C0, $50, $2E,
        $FF, $1E, $01, $0F, $0B, $C0, $74, $F4, $50, $8C, $CB, $53, $50, $2E,
        $FF, $1E, $09, $0F, $0B, $C0, $75, $03, $58, $EB, $EF, $8E, $D8, $58,
        $07, $A3, $ED, $0E, $89, $3E, $EB, $0E, $8C, $06, $EF, $0E, $B8, $D7,
        $8A, $BF, $EA, $0E, $2D, $62, $8A, $8B, $D0, $B9, $75, $0E, $96, $FD,
        $F6, $C1, $1F, $75, $02, $8B, $F2, $AC, $BB, $88
      );


      REC_NE:array [1..COUNT_NE] of TCompPtrRec = (
      (DataLen:80;FileType:NEEXE;NameType:NORMAL;   Name:@STR_NE_1;Data:@DAT_NE_1),
      (DataLen:78;FileType:NEEXE;NameType:PACKER;   Name:@STR_NE_2;Data:@DAT_NE_2),
      (DataLen:68;FileType:NEEXE;NameType:PACKER;   Name:@STR_NE_3;Data:@DAT_NE_3),
      (DataLen:80;FileType:NEEXE;NameType:COMPILER; Name:@STR_NE_4;Data:@DAT_NE_4),
      (DataLen:80;FileType:NEEXE;NameType:ENCRYPTER;Name:@STR_NE_5;Data:@DAT_NE_5)
      );

implementation

end.
