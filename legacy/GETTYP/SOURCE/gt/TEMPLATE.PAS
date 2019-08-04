procedure List_SKY (nStartPos:longint); far;
type
     SKY_GlobalHeader = record
     end;

     SKY_LocalHeader = record
     end;
const
      SKY_GH_SIZE = SizeOf (SKY_GlobalHeader);
      SKY_LH_SIZE = SizeOf (SKY_LocalHeader);
var
    GH:SKY_GlobalHeader;
    LH:SKY_LocalHeader;
    sFilename:string;
begin
  TArchive_Init (nStartPos);
  TFile_ReadActBuf (GH, SKY_GH_SIZE);

  TArchive_NoteHeaderX;

  repeat
    if (TFile_ReadActBuf (LH, SKY_LH_SIZE) = SKY_LH_SIZE) then
    begin
      sFilename := TFile_ReadActString (LH.nFilenameLen);
      _SetFileInfo (LH.nOldSize,
                    LH.nNewSize,
                    LH.nDT,
                    sFilename,
                    false);
      TFile_IncFilePos (LH.nNewSize);
    end;
  until (not TFile_IsOkay);

  TArchive_Notefooter;
end;
