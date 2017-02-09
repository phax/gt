/*
procedure List_BSN (nStartPos:longint); far;
type
const
      BSN_H1_SIZE = SizeOf (BSN_Header1);
      BSN_H2_SIZE = SizeOf (BSN_Header2);
var
    H1:BSN_Header1;
    H2:BSN_Header2;
    sFilename:string;
    nOldFilePos:longint;
    length:word;
begin
  TArchive_Init (nStartPos + 6);
  TArchive_NoteHeaderX;

  repeat
    nOldFilePos := TFile_GetFilePos;
    length := TFile_ReadActBuf (H1, BSN_H1_SIZE);
    if (length = BSN_H1_SIZE) then
    begin
      sFilename := TFile_Read_To (#0);
      if (TFile_ReadActBuf (H2, BSN_H2_SIZE) = BSN_H2_SIZE) then
      begin
        SwapLongSelf (H2.nOldSize);
        SwapLongSelf (H2.nNewSize);
        SwapLongSelf (H1.nDT);

        _SetFileInfo (H2.nOldSize,
                      H2.nNewSize,
                      H1.nDT,
                      sFilename,
                      false);

        TFile_GotoFilePos (nOldFilePos +
                           BSN_H1_SIZE +
                           1 + length (sFilename) +
                           BSN_H2_SIZE +
                           H2.nNewSize);
      end
    end
    else
      if (length = 2) then
        TFile_Finish;
  until (not TFile_IsOkay);
  TArchive_NoteFooter;
end;
*/
//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (BSN)
//--------------------------------------------------------------------
{
  BSN_Header1 aH1;
  BSN_Header2 aH2;
  pstring sFilename;

  _SetArchiveFlag (ARC_DT_DOS);

  m_pBuffer->SetActFilePos (m_nLocalOffset + 6);

  for (;;)
  {
    // read first header
    m_pBuffer->GetBufferX (&aH1, GT_BSN_H1_SIZE);

    // read filename
    m_pBuffer->GetASCIIZ_AString (sFilename, GT_MAX_SIZE);

    // read second header
    m_pBuffer->GetBufferX (&aH2, GT_BSN_H2_SIZE);

    // add file
    ArchiveFile* pArcFile = new ArchiveFile (sFilename);
    pArcFile->SetPackedSize (LONG_SWAP_BYTE (aH2.nPackedSize));
    pArcFile->SetUnpackedSize (LONG_SWAP_BYTE (aH2.nOriginalSize));
    pArcFile->SetDT (LONG_SWAP_BYTE (aH1.nDT));
    _AddFile (pArcFile);

    // skip data
    m_pBuffer->IncActFilePos (LONG_SWAP_BYTE (aH2.nPackedSize));

    // eof?
    if (m_pBuffer->GetActFilePos () + 2 == m_pBuffer->GetFileSize ())
    {
      m_pBuffer->SetToEOF ();
      break;
    }
  }
}
