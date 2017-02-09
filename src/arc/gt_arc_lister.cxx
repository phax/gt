#include "gt_arc_lister.hxx"
#include "gt_arc_structs.hxx"

#include "gt_crc32.hxx"
#include "gt_direntry.hxx"
#include "gt_filebuffer.hxx"
#include "gt_output.hxx"
#include "gt_table.hxx"
#include "gt_time.hxx"
#include "gt_utils.hxx"

namespace GT {

//--------------------------------------------------------------------
static void _GetDTStr (gtuint64 nDT, int nDTType, pstring& sRes)
//--------------------------------------------------------------------
{
  ASSERT (nDTType >= 1 && nDTType <= 3);
  DateTime aDT;
  if (nDTType == ARC_DT_DOS)
    aDT.SetDOSTime ((gtuint32) nDT);
  else
  if (nDTType == ARC_DT_WIN)
    aDT.SetWindowsTime (nDT);
  else
    aDT.SetUnixTime ((gtuint32) nDT);
  aDT.DateToString (sRes, true);  // true - long format
}

//--------------------------------------------------------------------
void ArchiveContent::Display ()
//--------------------------------------------------------------------
{
  // print version info?
  if (m_aVersion.Use ())
    out_format (rc (HRC_ARCLIB_ARCFLAG_VERSION),
                m_aVersion.m_nMajor, m_aVersion.m_nMinor);

  if (_IsArchiveFlag (ARC_ENCRYPTED))
    out_append (rc (HRC_ARCLIB_ARCFLAG_ENCRYPTED));

  if (_IsArchiveFlag (ARC_SOLID))
    out_append (rc (HRC_ARCLIB_ARCFLAG_SOLID));

  if (_IsArchiveFlag (ARC_COMMENT))
    out_append (rc (HRC_ARCLIB_ARCFLAG_COMMENT));

  if (_IsArchiveFlag (ARC_MULTI_VOLUME))
    out_append (rc (HRC_ARCLIB_ARCFLAG_MULTI_VOLUME));

  if (_IsArchiveFlag (ARC_LOCKED))
    out_append (rc (HRC_ARCLIB_ARCFLAG_LOCKED));

  if (_IsArchiveFlag (ARC_AUTHENTIFICATION))
    out_append (rc (HRC_ARCLIB_ARCFLAG_AUTHENTIFICATION));

  if (_IsArchiveFlag (ARC_RECOVERY_RECORD))
    out_append (rc (HRC_ARCLIB_ARCFLAG_RECOVERY_RECORD));

  // print AddOnString
  if (!m_sAddOnString.empty ())
    out_append (m_sAddOnString.c_str ());

  // any files to print?
  if (!m_aFiles.empty ())
  {
    pstring sHelpStr;
    int nCol = 0;
    file_t nTotalPacked = 0;
    file_t nTotalUnpacked = 0;
    gtuint32 cols = 0;

    // packed size column?
    if (!_IsArchiveFlag (ARC_NO_PACKED_SIZE))
      cols |= BITS[0];

    // unpacked size column?
    if (!_IsArchiveFlag (ARC_NO_UNPACKED_SIZE))
      cols |= BITS[1];

    // percentage col
    if (!_IsArchiveFlag (ARC_NO_PACKED_SIZE) && !_IsArchiveFlag (ARC_NO_UNPACKED_SIZE))
      cols |= BITS[2];

    // date/time column?
    if (_HasDT ())
      cols |= BITS[3];

    // filename column
    cols |= BITS[4];

    // build table with all available columns
    Table aTable; // (cols.count (), m_aFiles.size () + 1);
    if (cols & BITS[0])
      aTable.AddColumn (rc (HRC_ARCLIB_COLUMN_PACKED), NUM_SPACE, 12, 10);
    if (cols & BITS[1])
      aTable.AddColumn (rc (HRC_ARCLIB_COLUMN_UNPACKED), NUM_SPACE, 12, 10);
    if (cols & BITS[2])
      aTable.AddColumn (rc (HRC_ARCLIB_COLUMN_PERCENTAGE), NUM_DOUBLE, 6, 1);
    if (cols & BITS[3])
      aTable.AddColumn (rc (HRC_ARCLIB_COLUMN_DATETIME), STR, 12, ALIGN_LEFT);
    if (cols & BITS[4])
      aTable.AddColumn (rc (HRC_ARCLIB_COLUMN_FILENAME), STR, 1, ALIGN_LEFT);

    ArchiveFileList::const_iterator cit = m_aFiles.begin ();
    for (; !(cit == m_aFiles.end ()); ++cit)
    {
      nCol = 0;

      // packed size present?
      if (cols & BITS[0])
      {
        if ((*cit)->IsDirectory ())
          aTable.AddInt (nCol++, file_t (0));
        else
        {
          aTable.AddInt (nCol++, (*cit)->GetPackedSize ());
          nTotalPacked += (*cit)->GetPackedSize ();
        }
      }

      // unpacked size present?
      if (cols & BITS[1])
      {
        if ((*cit)->IsDirectory ())
          aTable.AddInt (nCol++, file_t (0));
        else
        {
          aTable.AddInt (nCol++, (*cit)->GetUnpackedSize ());
          nTotalUnpacked += (*cit)->GetUnpackedSize ();
        }
      }

      // percentage present
      if (cols & BITS[2])
      {
        if ((*cit)->IsDirectory ())
          aTable.AddDbl (nCol++, 0.0);
        else
        {
          double dPerc = (*cit)->GetUnpackedSize ()
                            ? double ((*cit)->GetPackedSize ()) / (*cit)->GetUnpackedSize () * 100
                            : 0.0;
          aTable.AddDbl (nCol++, dPerc);
        }
      }

      // date/time str present
      if (cols & BITS[3])
      {
        _GetDTStr ((*cit)->GetDT (), m_nFlags & 3, sHelpStr);
        aTable.AddStr (nCol++, sHelpStr);
      }

      // special flags
      if (cols & BITS[4])
      {
        sHelpStr = (*cit)->GetFilename ();
        if ((*cit)->IsPWProtected ())
          str_append_from_res (sHelpStr, HRC_ARCLIB_FILEFLAG_PASSWORD);
        if ((*cit)->IsDirectory ())
          str_append_from_res (sHelpStr, HRC_ARCLIB_FILEFLAG_DIR);
        if ((*cit)->IsContinuedFromLast ())
          str_append_from_res (sHelpStr, HRC_ARCLIB_FILEFLAG_FROMLAST);
        if ((*cit)->IsContinuedOnNext ())
          str_append_from_res (sHelpStr, HRC_ARCLIB_FILEFLAG_ONNEXT);
        if ((*cit)->HasComment ())
          str_append_from_res (sHelpStr, HRC_ARCLIB_FILEFLAG_COMMENT);
        aTable.AddStr (nCol++, sHelpStr);
      }
    }

    // add the summary
    nCol = 0;
    if (cols & BITS[0])
      aTable.AddInt (nCol++, nTotalPacked);
    if (cols & BITS[1])
      aTable.AddInt (nCol++, nTotalUnpacked);
    if (cols & BITS[2])
      aTable.AddDbl (nCol++, nTotalUnpacked ? double (nTotalPacked) / nTotalUnpacked * 100 : 0.0);
    if (cols & BITS[3])
      aTable.AddStr (nCol++, pstring ());
    if (cols & BITS[4])
    {
      str_assign_format (sHelpStr, rc (HRC_ARCLIB_TOTAL), m_aFiles.size ());
      aTable.AddStr (nCol++, sHelpStr);
    }

    // print it :)
    out_table (&aTable);
  }

  // print error messages (if any)
  switch (m_eErrorCode)
  {
    case ARCERR_SUCCESS:
      // success
      break;
    case ARCERR_UNEXPECTED_EOF:
      out_error_append (rc (HRC_ARCLIB_ERROR_UNEXPECTED_EOF));
      break;
    case ARCERR_UNEXPECTED_VALUE:
      out_error_append (rc (HRC_ARCLIB_ERROR_UNEXPECTED_VALUE));
      break;
    case ARCERR_MISSING_DATA:
      out_error_append (rc (HRC_ARCLIB_ERROR_MISSING_DATA));
      break;
    case ARCERR_UNKNOWN_METHOD:
      out_error_append (rc (HRC_ARCLIB_ERROR_UNKNOWN_METHOD));
      break;
    case ARCERR_EXCEPTION:
      out_error_append (rc (HRC_ARCLIB_ERROR_EXCEPTION));
      break;
  }
}

//--------------------------------------------------------------------
BasicArchiveLister::BasicArchiveLister
                                        (      FileBuffer* pBuffer,
                                         const file_t      nOffset)
//--------------------------------------------------------------------
  : m_pBuffer      (pBuffer),
    m_nLocalOffset (nOffset)
{}

//--------------------------------------------------------------------
void BasicArchiveLister::ListFiles ()
//--------------------------------------------------------------------
{
  /* empty */
}

#include "gt_777.hxx"
#include "gt_ace.hxx"
#include "gt_ain.hxx"
#include "gt_akt7.hxx"
#include "gt_akt9.hxx"
#include "gt_amg.hxx"
#include "gt_arc.hxx"
#include "gt_arg.hxx"
#include "gt_arj.hxx"
#include "gt_arq.hxx"
#include "gt_arx.hxx"
#include "gt_asd.hxx"
#include "gt_bix.hxx"
#include "gt_bli.hxx"
#include "gt_boa.hxx"
#include "gt_bsa.hxx"
#include "gt_bsn.hxx"
#include "gt_cab.hxx"
#include "gt_dpa.hxx"
#include "gt_gz.hxx"
#include "gt_ha.hxx"
#include "gt_inno.hxx"
#include "gt_imp.hxx"
#include "gt_ins.hxx"
#include "gt_isc.hxx"
#include "gt_iso.hxx"
#include "gt_lg.hxx"
#include "gt_lzh.hxx"
#include "gt_lzo.hxx"
#include "gt_nero.hxx"
#include "gt_nero5.hxx"
#include "gt_nsis.hxx"
#include "gt_paq.hxx"
#include "gt_paq8.hxx"
#include "gt_ppz.hxx"
#include "gt_rar.hxx"
#include "gt_sbc.hxx"
#include "gt_sony.hxx"
#include "gt_sti.hxx"
#include "gt_syn.hxx"
#include "gt_tar.hxx"
#include "gt_xpa.hxx"
#include "gt_yc.hxx"
#include "gt_zet.hxx"
#include "gt_zip.hxx"
#include "gt_zoo.hxx"

}  // namespace
