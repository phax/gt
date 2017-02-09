#include "gt_exe_types.hxx"

#include "gt_direntry.hxx"
#include "gt_filebuffer.hxx"
#include "gt_utils.hxx"  // _min

namespace GT {

//--------------------------------------------------------------------
EXEHeader::EXEHeader (FileBuffer *pBuffer)
//--------------------------------------------------------------------
  : BasicAnalyzer  (pBuffer),
    m_bIsValid     (false),
    m_nStartOffset (0)
{
  if (m_pBuffer->GetScanOffset () == 0)
  {
    // read DOS header at start of file..
    m_bIsValid = m_pBuffer->GetBuffer (0, &m_aDOSHeader, EXE_DOS_HEADER_SIZE);
  }
  else
  {
    // we're somewhere in the file (overlay detection)
    // check all MAX_TRIES bytes...
    const size_t MAX_TRIES = 256;
    gtuint8 aLocalBuffer[sizeof (EXE_DOS_Header) + MAX_TRIES];

    // determine maximum number of tries
    // Possibility: filesize < MAX_TRIES!!
    size_t nTries = _min (size_t (m_pBuffer->GetFileSize ()),
                          sizeof (aLocalBuffer));

    // filesize < sizeof (EXE_DOS_Header) are ignored
    if (nTries <= sizeof (EXE_DOS_Header))
      return;

    // remove size of EXE DOS header so that only the tries are left
    nTries -= sizeof (EXE_DOS_Header);
    ASSERT (nTries > 0);

    // and now read the desired amount!
    m_pBuffer->GetBuffer (0, aLocalBuffer, sizeof (EXE_DOS_Header) + nTries);

    for (file_t i = 0; i < nTries; i++)
    {
      // fill the member variable from the local buffer (to avoid unncessary
      // file accesses!
      memcpy (&m_aDOSHeader, aLocalBuffer + i, sizeof (EXE_DOS_Header));

      // is the current header valid?
      if (IsValidHeaderID ())
      {
        // yep, get start offset and notify out buffer handler
        // that he needs to advance!
        m_nStartOffset = i;
        m_pBuffer->IncScanOffset (i);
        m_bIsValid = true;
        break;
      }
    }
  }
}

//--------------------------------------------------------------------
bool EXEHeader::IsValidCOM () const
//--------------------------------------------------------------------
{
  DirEntry aDE (m_pBuffer->GetFileName ());

  // ensure the extension is .COM or .EXE
  if (!aDE.HasFileNameExtension (_T ("com")) &&
      !aDE.HasFileNameExtension (_T ("exe")))
  {
    return false;
  }

  // the filesize must be les than 64k - 16 = 65520 bytes / 0xFFF0 bytes
  if (m_pBuffer->GetFileSize () >= 0xfff0)
    return false;

  // and DOS COMs cannot be executed in overlays
  if (m_pBuffer->GetScanOffset () > 0)
    return false;

  return true;
}

//--------------------------------------------------------------------
gtuint32 EXEHeader::GetAbsoluteIP () const
//--------------------------------------------------------------------
{
  const gtuint32 MAX_IP = EXE_PARAGRAPH_TO_BYTE (65536);
  gtuint32 i = GetHeaderSize () + GetCS () + GetIP ();
  return i >= MAX_IP
           ? i - MAX_IP
           : i;
}

//--------------------------------------------------------------------
file_t EXEHeader::GetOverlaySize () const
//--------------------------------------------------------------------
{
  return (m_pBuffer->GetFileSize () - GetSizeInHeader ());
}

}  // namespace
