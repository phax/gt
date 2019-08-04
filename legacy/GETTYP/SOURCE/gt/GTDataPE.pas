{$i aflags.inc}

{ if it is BP 7, disabled the typesafe @ operator in this file }
{$ifdef VER70}
{$T-}
{$endif}
{$ifdef WIN32}
{$T-}
{$endif}

unit GTDataPE;

interface

uses
     GTCompar;

const

      STR_WWPack32_100:string[13] = 'WWPack32 1.0';
      DAT_WWPack32_100:array[1..80] of byte = (
        $00, $00, $58, $2D, $6D, $00, $00, $00, $8B, $E8, $50, $60, $FD, $2E,
        $2B, $85, $73, $02, $00, $00, $8B, $F0, $2E, $03, $B5, $7F, $02, $00,
        $00, $8B, $F8, $2E, $03, $BD, $77, $02, $00, $00, $2E, $8B, $8D, $7B,
        $02, $00, $00, $F3, $A5, $8B, $F7, $83, $C6, $04, $8B, $F8, $FC, $AD,
        $8B, $E8, $B3, $20, $EB, $37, $33, $C0, $0F, $A5, $E8, $2A, $D9, $7E,
        $03, $D3, $E5, $C3, $74, $13, $F6, $DB, $8A, $CB
      );
      REC_WWPack32_100:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:PACKER;
        Name:@STR_WWPack32_100;
        Data:@DAT_WWPack32_100
      );

      STR_WWPack32_109:string[14] = 'WWPack32 1.09';
      DAT_WWPack32_109:array[1..80] of byte = (
        $58, $2D, $6D, $00, $00, $00, $8B, $E8, $50, $FD, $2E, $2B, $85, $6F,
        $02, $00, $00, $8B, $F0, $2E, $03, $B5, $7B, $02, $00, $00, $8B, $F8,
        $2E, $03, $BD, $73, $02, $00, $00, $2E, $8B, $8D, $77, $02, $00, $00,
        $F3, $A5, $8B, $F7, $83, $C6, $04, $8B, $F8, $FC, $AD, $8B, $E8, $B3,
        $20, $EB, $37, $33, $C0, $0F, $A5, $E8, $2A, $D9, $7E, $03, $D3, $E5,
        $C3, $74, $13, $F6, $DB, $8A, $CB, $D3, $E8, $95
     );
      REC_WWPack32_109:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:PACKER;
        Name:@STR_WWPack32_109;
        Data:@DAT_WWPack32_109
      );

      STR_WWPack32_111:string[21] = 'WWPack32 1.10 - 1.11';
      DAT_WWPack32_111:array[1..80] of byte = (
        $33, $C9, $E8, $00, $00, $00, $00, $58, $2D, $6F, $00, $00, $00, $8B,
        $E8, $50, $51, $FD, $2E, $2B, $84, $0D, $9B, $02, $00, $00, $8B, $F0,
        $2E, $03, $B4, $0D, $A7, $02, $00, $00, $8B, $F8, $2E, $03, $BC, $0D,
        $9F, $02, $00, $00, $57, $2E, $8B, $8C, $0D, $A3, $02, $00, $00, $F3,
        $A5, $8B, $F7, $83, $C6, $04, $8B, $F8, $FC, $AD, $8B, $E8, $B3, $20,
        $EB, $37, $33, $C0, $0F, $A5, $E8, $2A, $D9, $7E
      );
      REC_WWPack32_111:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:PACKER;
        Name:@STR_WWPack32_111;
        Data:@DAT_WWPack32_111
      );

      STR_WWPack32_112:string[14] = 'WWPack32 1.12';
      DAT_WWPack32_112:array[1..80] of byte = (
        $00, $00, $58, $2D, $6D, $00, $00, $00, $50, $60, $33, $C9, $50, $58,
        $50, $50, $8B, $E8, $51, $FD, $2E, $2B, $84, $0D, $A1, $02, $00, $00,
        $8B, $F0, $2E, $03, $B4, $0D, $AD, $02, $00, $00, $8B, $F8, $2E, $03,
        $BC, $0D, $A5, $02, $00, $00, $57, $2E, $8B, $8C, $0D, $A9, $02, $00,
        $00, $F3, $A5, $8B, $F7, $83, $C6, $04, $8B, $F8, $FC, $AD, $8B, $E8,
        $B3, $20, $EB, $37, $33, $C0, $0F, $A5, $E8, $2A
      );
      REC_WWPack32_112:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:PACKER;
        Name:@STR_WWPack32_112;
        Data:@DAT_WWPack32_112
      );

      STR_Delphi4:string[16] = 'Borland Delphi 4';
      DAT_Delphi4:array[1..80] of byte = (
        $50, $6A, $00, $E8, $F8, $FE, $FF, $FF, $BA, $00, $00, $00, $00, $52,
        $89, $05, $D8, $00, $00, $00, $89, $42, $04, $C7, $42, $08, $00, $00,
        $00, $00, $C7, $42, $0C, $00, $00, $00, $00, $E8, $8A, $FF, $FF, $FF,
        $5A, $58, $E8, $00, $00, $FF, $FF, $C3, $8B, $C0, $55, $8B, $EC, $33,
        $C0, $55, $68, $00, $00, $40, $00, $64, $FF, $30, $64, $89, $20, $FF,
        $05, $DC, $00, $00, $00, $33, $C0, $5A, $59, $59
      );
      REC_Delphi4:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:COMPILER;
        Name:@STR_Delphi4;
        Data:@DAT_Delphi4
      );

      STR_ASPack104b:string[13] = 'ASPack 1.04b';
      DAT_ASPack104b:array[1..80] of byte = (
        $60, $E8, $00, $00, $00, $00, $5D, $81, $ED, $AE, $98, $43, $00, $B8,
        $A8, $98, $43, $00, $03, $C5, $2B, $85, $12, $9D, $43, $00, $89, $85,
        $1E, $9D, $43, $00, $80, $BD, $08, $9D, $43, $00, $00, $75, $15, $FE,
        $85, $08, $9D, $43, $00, $E8, $1D, $00, $00, $00, $E8, $73, $02, $00,
        $00, $E8, $0A, $03, $00, $00, $8B, $85, $0A, $9D, $43, $00, $03, $85,
        $1E, $9D, $43, $00, $89, $44, $24, $1C, $61, $FF
      );
      REC_ASPack104b:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:PACKER;
        Name:@STR_ASPack104b;
        Data:@DAT_ASPack104b
      );

      STR_ASPack105b:string[13] = 'ASPack 1.05b';
      DAT_ASPack105b:array[1..80] of byte = (
        $60, $E8, $00, $00, $00, $00, $5D, $81, $ED, $AE, $98, $43, $00, $B8,
        $A8, $98, $43, $00, $03, $C5, $2B, $85, $18, $9D, $43, $00, $89, $85,
        $24, $9D, $43, $00, $80, $BD, $0E, $9D, $43, $00, $00, $75, $15, $FE,
        $85, $0E, $9D, $43, $00, $E8, $1D, $00, $00, $00, $E8, $73, $02, $00,
        $00, $E8, $0A, $03, $00, $00, $8B, $85, $10, $9D, $43, $00, $03, $85,
        $24, $9D, $43, $00, $89, $44, $24, $1C, $61, $FF
      );
      REC_ASPack105b:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:PACKER;
        Name:@STR_ASPack105b;
        Data:@DAT_ASPack105b
      );

      STR_ASPack1061b:string[13] = 'ASPack 1.061b';
      DAT_ASPack1061b:array[1..80] of byte = (
        $60, $E8, $00, $00, $00, $00, $5D, $81, $ED, $EA, $A8, $43, $00, $B8,
        $E4, $A8, $43, $00, $03, $C5, $2B, $85, $78, $AD, $43, $00, $89, $85,
        $84, $AD, $43, $00, $80, $BD, $6E, $AD, $43, $00, $00, $75, $15, $FE,
        $85, $6E, $AD, $43, $00, $E8, $1D, $00, $00, $00, $E8, $73, $02, $00,
        $00, $E8, $0A, $03, $00, $00, $8B, $85, $70, $AD, $43, $00, $03, $85,
        $84, $AD, $43, $00, $89, $44, $24, $1C, $61, $FF
      );
      REC_ASPack1061b:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:PACKER;
        Name:@STR_ASPack1061b;
        Data:@DAT_ASPack1061b
      );

      STR_ASPack107b:string[12] = 'ASPack 1.07b';
      DAT_ASPack107b:array[1..80] of byte = (
        $60, $E8, $00, $00, $00, $00, $5D, $81, $ED, $3E, $D9, $43, $00, $B8,
        $38, $D9, $43, $00, $03, $C5, $2B, $85, $0B, $DE, $43, $00, $89, $85,
        $17, $DE, $43, $00, $80, $BD, $01, $DE, $43, $00, $00, $75, $15, $FE,
        $85, $01, $DE, $43, $00, $E8, $1D, $00, $00, $00, $E8, $79, $02, $00,
        $00, $E8, $12, $03, $00, $00, $8B, $85, $03, $DE, $43, $00, $03, $85,
        $17, $DE, $43, $00, $89, $44, $24, $1C, $61, $FF
      );
      REC_ASPack107b:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:PACKER;
        Name:@STR_ASPack107b;
        Data:@DAT_ASPack107b
      );

      STR_ASPack108:string[11] = 'ASPack 1.08';
      DAT_ASPack108:array[1..80] of byte = (
        $60, $EB, $0A, $5D, $EB, $02, $FF, $25, $45, $FF, $E5, $E8, $E9, $E8,
        $F1, $FF, $FF, $FF, $E9, $81, $ED, $23, $EA, $44, $00, $BB, $10, $EA,
        $44, $00, $03, $DD, $2B, $9D, $72, $EF, $44, $00, $8D, $8D, $DE, $F0,
        $44, $00, $51, $FF, $95, $C4, $F1, $44, $00, $8D, $8D, $04, $F1, $44,
        $00, $51, $50, $FF, $95, $C0, $F1, $44, $00, $6A, $00, $54, $6A, $04,
        $68, $14, $08, $00, $00, $8D, $8D, $10, $EA, $44
      );
      REC_ASPack108:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:PACKER;
        Name:@STR_ASPack108;
        Data:@DAT_ASPack108
      );

      STR_ASPack1081:string[12] = 'ASPack 1.081';
      DAT_ASPack1081:array[1..80] of byte = (
        $60, $EB, $0A, $5D, $EB, $02, $FF, $25, $45, $FF, $E5, $E8, $E9, $E8,
        $F1, $FF, $FF, $FF, $E9, $81, $ED, $23, $6A, $44, $00, $BB, $10, $6A,
        $44, $00, $03, $DD, $2B, $9D, $72, $6F, $44, $00, $8D, $8D, $DE, $70,
        $44, $00, $51, $FF, $95, $C4, $71, $44, $00, $8D, $8D, $04, $71, $44,
        $00, $51, $50, $FF, $95, $C0, $71, $44, $00, $6A, $00, $54, $6A, $04,
        $68, $14, $08, $00, $00, $8D, $8D, $10, $6A, $44
      );
      REC_ASPack1081:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:PACKER;
        Name:@STR_ASPack1081;
        Data:@DAT_ASPack1081
      );

      STR_ASPack1082:string[12] = 'ASPack 1.082';
      DAT_ASPack1082:array[1..80] of byte = (
        $60, $EB, $0A, $5D, $EB, $02, $FF, $25, $45, $FF, $E5, $E8, $E9, $E8,
        $F1, $FF, $FF, $FF, $E9, $81, $ED, $23, $6A, $44, $00, $BB, $10, $6A,
        $44, $00, $03, $DD, $2B, $9D, $46, $6F, $44, $00, $8D, $8D, $EA, $6F,
        $44, $00, $51, $FF, $95, $D0, $70, $44, $00, $8D, $8D, $10, $70, $44,
        $00, $51, $50, $FF, $95, $CC, $70, $44, $00, $6A, $00, $54, $6A, $04,
        $68, $20, $07, $00, $00, $8D, $8D, $10, $6A, $44
      );
      REC_ASPack1082:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:PACKER;
        Name:@STR_ASPack1082;
        Data:@DAT_ASPack1082
      );

      STR_ASPack1084:string[12] = 'ASPack 1.084';
      DAT_ASPack1084:array[1..80] of byte = (
        $8B, $2C, $24, $81, $ED, $1A, $29, $44, $00, $C3, $55, $8B, $EC, $60,
        $55, $8B, $75, $08, $8B, $7D, $0C, $FC, $B2, $80, $8A, $06, $46, $88,
        $07, $47, $02, $D2, $75, $05, $8A, $16, $46, $12, $D2, $73, $EF, $02,
        $D2, $75, $05, $8A, $16, $46, $12, $D2, $73, $4A, $33, $C0, $02, $D2,
        $75, $05, $8A, $16, $46, $12, $D2, $0F, $83, $D6, $00, $00, $00, $02,
        $D2, $75, $05, $8A, $16, $46, $12, $D2, $13, $C0
      );
      REC_ASPack1084:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:PACKER;
        Name:@STR_ASPack1084;
        Data:@DAT_ASPack1084
      );

      STR_ASPack2000:string[12] = 'ASPack 2.000';
      DAT_ASPack2000:array[1..80] of byte = (
        $8B, $2C, $24, $81, $ED, $AB, $39, $44, $00, $C3, $8B, $44, $24, $10,
        $81, $EC, $54, $03, $00, $00, $8D, $4C, $24, $04, $50, $E8, $A8, $03,
        $00, $00, $8B, $8C, $24, $5C, $03, $00, $00, $8B, $94, $24, $58, $03,
        $00, $00, $51, $52, $8D, $4C, $24, $0C, $E8, $0D, $04, $00, $00, $84,
        $C0, $75, $0A, $83, $C8, $FF, $81, $C4, $54, $03, $00, $00, $C3, $8B,
        $8C, $24, $60, $03, $00, $00, $8D, $04, $24, $50
      );
      REC_ASPack2000:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:PACKER;
        Name:@STR_ASPack2000;
        Data:@DAT_ASPack2000
      );

      STR_ASPack2001:string[12] = 'ASPack 2.001';
      DAT_ASPack2001:array[1..80] of byte = (
        $8B, $2C, $24, $81, $ED, $83, $29, $44, $00, $C3, $8B, $44, $24, $10,
        $81, $EC, $54, $03, $00, $00, $8D, $4C, $24, $04, $50, $E8, $A8, $03,
        $00, $00, $8B, $8C, $24, $5C, $03, $00, $00, $8B, $94, $24, $58, $03,
        $00, $00, $51, $52, $8D, $4C, $24, $0C, $E8, $0D, $04, $00, $00, $84,
        $C0, $75, $0A, $83, $C8, $FF, $81, $C4, $54, $03, $00, $00, $C3, $8B,
        $8C, $24, $60, $03, $00, $00, $8D, $04, $24, $50
      );
      REC_ASPack2001:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:PACKER;
        Name:@STR_ASPack2001;
        Data:@DAT_ASPack2001
      );

      STR_ASPack2100:string[12] = 'ASPack 2.100';
      DAT_ASPack2100:array[1..80] of byte = (
        $8B, $2C, $24, $81, $ED, $37, $39, $44, $00, $C3, $8B, $44, $24, $10,
        $81, $EC, $54, $03, $00, $00, $8D, $4C, $24, $04, $50, $E8, $A8, $03,
        $00, $00, $8B, $8C, $24, $5C, $03, $00, $00, $8B, $94, $24, $58, $03,
        $00, $00, $51, $52, $8D, $4C, $24, $0C, $E8, $0D, $04, $00, $00, $84,
        $C0, $75, $0A, $83, $C8, $FF, $81, $C4, $54, $03, $00, $00, $C3, $8B,
        $8C, $24, $60, $03, $00, $00, $8D, $04, $24, $50
      );
      REC_ASPack2100:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:PACKER;
        Name:@STR_ASPack2100;
        Data:@DAT_ASPack2100
      );

      STR_Dylan12:string[19] = 'Harlequin Dylan 1.2';
      DAT_Dylan12:array[1..80] of byte = (
        $55, $89, $E5, $9C, $53, $56, $57, $FC, $E8, $A9, $FF, $FF, $FF, $68,
        $0C, $00, $00, $00, $68, $00, $00, $00, $00, $B8, $08, $00, $00, $00,
        $E8, $00, $00, $FF, $FF, $E8, $00, $00, $FF, $FF, $8D, $65, $F0, $5F,
        $5E, $5B, $9D, $5D, $C3, $8B, $44, $24, $04, $8B, $54, $24, $08, $83,
        $FA, $01, $74, $17, $83, $FA, $02, $74, $0A, $83, $FA, $03, $74, $05,
        $83, $FA, $00, $74, $2A, $B8, $FF, $FF, $FF, $FF
      );
      REC_Dylan12:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:COMPILER;
        Name:@STR_Dylan12;
        Data:@DAT_Dylan12
      );

      STR_PKLite32_11:string[12] = 'PKLite32 1.1';
      DAT_PKLite32_11:array[1..80] of byte = (
        $55, $8B, $EC, $A1, $00, $00, $00, $00, $85, $C0, $74, $09, $B8, $01,
        $00, $00, $00, $5D, $C2, $0C, $00, $8B, $45, $0C, $57, $56, $53, $8B,
        $5D, $10, $BF, $00, $00, $00, $00, $2B, $D8, $C7, $05, $00, $00, $00,
        $00, $01, $00, $00, $00, $81, $C3, $00, $00, $00, $00, $2B, $F8, $89,
        $7D, $08, $81, $3B, $44, $33, $22, $11, $74, $05, $90, $90, $CC, $90,
        $90, $8B, $43, $04, $85, $C0, $74, $01, $CC, $8B
      );
      REC_PKLite32_11:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:PACKER;
        Name:@STR_PKLite32_11;
        Data:@DAT_PKLite32_11
      );

      STR_VGCrypt_075:string[12] = 'VGCrypt 0.75';
      DAT_VGCrypt_075:array[1..80] of byte = (
        $E8, $1B, $00, $00, $00, $8B, $64, $24, $08, $E8, $DF, $FF, $FF, $FF,
        $C6, $85, $CC, $26, $40, $00, $C3, $E8, $4B, $FF, $FF, $FF, $E9, $16,
        $FF, $FF, $FF, $B7, $64, $67, $FF, $36, $00, $00, $64, $67, $89, $26,
        $00, $00, $8D, $B5, $47, $26, $40, $00, $8B, $FE, $B9, $CA, $00, $00,
        $00, $8A, $A5, $11, $27, $40, $00, $AC, $32, $C4, $FE, $C4, $C0, $C4,
        $02, $80, $C4, $90, $AA, $E2, $F2, $E9, $CB, $FE
      );
      REC_VGCrypt_075:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:PROTECTOR;
        Name:@STR_VGCrypt_075;
        Data:@DAT_VGCrypt_075
      );

      STR_VPascal_21:string[18] = 'Virtual Pascal 2.1';
      DAT_VPascal_21:array[1..80] of byte = (
        $83, $EC, $10, $FC, $51, $E8, $D2, $FF, $FF, $FF, $E8, $00, $00, $FF,
        $FF, $A3, $00, $00, $00, $00, $5A, $E8, $96, $FE, $FF, $FF, $E8, $7B,
        $03, $00, $00, $A3, $00, $00, $00, $00, $6A, $00, $E8, $00, $03, $00,
        $00, $A3, $00, $00, $00, $00, $A3, $00, $00, $00, $00, $E8, $00, $00,
        $FF, $FF, $83, $F8, $03, $0F, $92, $05, $00, $00, $00, $00, $8D, $04,
        $24, $33, $D2, $64, $8B, $0A, $64, $89, $02, $89
      );
      REC_VPascal_21:TCompPtrRec = (
        DataLen:80;
        FileType:PEEXE;
        NameType:COMPILER;
        Name:@STR_VPascal_21;
        Data:@DAT_VPascal_21
      );


implementation

end.
