//--------------------------------------------------------------------
ARC_DEFINE_LISTMETHOD (SBC)
//--------------------------------------------------------------------
{
  SBC_Header aH;
  gtuint16 nVersion;

  // read header
  m_pBuffer->GetBufferX (m_nLocalOffset + 0, &aH, GT_SBC_H_SIZE);

  // get version
  nVersion = aH.nVersionAndCipherID >> 3;
  switch (nVersion)
  {
    case 0x1C12:
      _SetArchiveVersion (0, 700);
      break;
    case 0x0013:
      _SetArchiveVersion (0, 800);
      break;
    case 0x1E13:
      _SetArchiveVersion (0, 830);
      break;
    case 0x0313:
      _SetArchiveVersion (0, 835);
      break;
    case 0x0903:
      _SetArchiveVersion (0, 969);
      break;
    default:
      _SetArchiveVersion (HIBYTE (nVersion), LOBYTE (nVersion));
      break;
  }

  // done
  m_pBuffer->SetToEOF ();
}
