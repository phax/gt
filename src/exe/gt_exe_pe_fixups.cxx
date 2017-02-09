#include "gt_exe_pe.hxx"

#include "gt_filebuffer.hxx"
#include "gt_output.hxx"
#include "gt_utils.hxx"

namespace GT {

//--------------------------------------------------------------------
LPCTSTR GT_CALL EXE_PE_FixupsAnalyzer::_GetFixupEntryText
                                        (gtuint16 nEntryType)
//--------------------------------------------------------------------
{
  static const LPCTSTR EntryTypes [16] = {
    _T ("IMAGE_REL_BASED_ABSOLUTE"),
    _T ("IMAGE_REL_BASED_HIGH"),
    _T ("IMAGE_REL_BASED_LOW"),
    _T ("IMAGE_REL_BASED_HIGHLOW"),
    _T ("IMAGE_REL_BASED_HIGHADJ"),
    _T ("IMAGE_REL_BASED_MIPS_JMPADDR"),
    _T ("IMAGE_REL_BASED_SECTION"),
    _T ("IMAGE_REL_BASED_REL32"),
    _T ("[unknown 8]"),
    _T ("IMAGE_REL_BASED_MIPS_JMPADDR16"),
    _T ("IMAGE_REL_BASED_DIR64"),
    _T ("IMAGE_REL_BASED_HIGH3ADJ"),
    _T ("[unknown 12]"),
    _T ("[unknown 13]"),
    _T ("[unknown 14]"),
    _T ("[unknown 15]"),
  };

  return EntryTypes[nEntryType];
}

//--------------------------------------------------------------------
void GT_CALL EXE_PE_FixupsAnalyzer::List ()
//--------------------------------------------------------------------
{
  EXE_PE_FixupBlock aFixupBlock;
  gtuint32 nTotalFixupSizeLeft = m_pDataDir->nSize;
  gtuint32 nBlockSizeLeft;
  gtuint16 nEntry;

  out_append (_T ("\n"));

  if (m_nOffset < 0 || m_nOffset >= m_pBuffer->GetFileSize ())
  {
    out_error_format (rc (HRC_EXELIB_PE_FIXUPS_INVALID),
                      m_nOffset);
    return;
  }

  // header
  out_format (rc (HRC_EXELIB_PE_FIXUPS_AT_OFFSET),
              m_nOffset,
              RVA_VAL (m_pDataDir->nRVA),
              m_pDataDir->nSize);

  out_incindent ();

  // 'm_nTotalSize == 8' in "C:\Program Files\iview380full\Languages\Polski3.dll"
  // -> ASPack'ed resource table with a total size of 8 and invalid entries!
  if (m_pDataDir->nSize > 8)
  {
    // write table header
    out_append (rc (HRC_EXELIB_PE_FIXUPS_LIST_HEADER));

    // start reading
    m_pBuffer->SetActFilePos (m_nOffset);

    while (nTotalFixupSizeLeft > 0)
    {
      // read current fixup block
      if (!m_pBuffer->GetBuffer (&aFixupBlock, EXE_PE_FIXUPBLOCK_SIZE))
      {
        out_error_append (rc (HRC_EXELIB_PE_FIXUPS_ERROR_READ_BLOCK));
        break;
      }

      // dec rest size
      nTotalFixupSizeLeft -= aFixupBlock.nBlockSize;

      // #HACK# for UPX packed DLLs:
      // the last fixup block does not contain the terminating uint16!
      if (aFixupBlock.nBlockSize == 8 && nTotalFixupSizeLeft == 2)
      {
        aFixupBlock.nBlockSize += 2;
        nTotalFixupSizeLeft -= 2;
      }

      // write entry
      out_format (rc (HRC_EXELIB_PE_FIXUPS_LIST_BLOCK),
                  RVA_VAL (aFixupBlock.nPageRVA),
                  aFixupBlock.nBlockSize,
                  (aFixupBlock.nBlockSize - EXE_PE_FIXUPBLOCK_SIZE) / 2);

      // now write the entries
      nBlockSizeLeft = aFixupBlock.nBlockSize - EXE_PE_FIXUPBLOCK_SIZE;

      out_incindent ();

      // maybe 0 in UPX packed DLLs
      while (nBlockSizeLeft > 0 && m_pBuffer->ReadUint16 (nEntry))
      {
        nBlockSizeLeft -= 2;

        // lower 4 bit: type, upper 12 bit: offset
        out_format (rc (HRC_EXELIB_PE_FIXUPS_LIST_ENTRY),
                    _GetFixupEntryText (nEntry >> 12), nEntry & 0x0fff);
      }

      out_decindent ();

      // any error whilke reading?
      if (nBlockSizeLeft > 0)
      {
        out_error_append (rc (HRC_EXELIB_PE_FIXUPS_ERROR_READ_ENTRY));
        break;
      }
    }
  }
  else
  {
    out_info_append (rc (HRC_EXELIB_PE_FIXUPS_EMPTY));
  }

  out_decindent ();
}

}  // namespace
