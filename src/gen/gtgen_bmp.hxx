MAKE_ANALYZER (BMP);

//--------------------------------------------------------------------
enum BMP_Version
//--------------------------------------------------------------------
{
  BMP_None,
  BMP_Win2,
  BMP_Win3,
  BMP_WinNT,
  BMP_Win95,
  BMP_OS2,
};

/*! Text or binary files.
 */
//--------------------------------------------------------------------
MAKE_IS_A (BMP)
//--------------------------------------------------------------------
{
  // minimum filesize :)
  if (aGI.m_pBuffer->GetFileSize () <= BMP_HM_SIZE)
    return false;

  const gtuint16 w0  = aGI.m_pBuffer->GetUint16 (0);
  const gtuint32 l14 = aGI.m_pBuffer->GetInt32 (14);

  //              "BM"            "IC"
  return ((w0 == 0x4D42 || w0 == 0x4349) &&
          (l14 == 12 || l14 == 40 || l14 == 64 || l14 == 108));
}

//--------------------------------------------------------------------
MAKE_DISPLAY (BMP)
//--------------------------------------------------------------------
{
  BMP_MainHeader aHM;
  BMP_Version eVersion;

  out_append (rc (HRC_GENLIB_BMP_BMP));

  // read main header - should always work (we check for the size in IsA)
  m_pBuffer->GetBufferX (0, &aHM, BMP_HM_SIZE);

  // read header into memory
  gtuint8 *pBufPtr = (gtuint8*) _alloca (aHM.nHeaderSize);
  m_pBuffer->GetBufferX (BMP_HM_SIZE, pBufPtr, aHM.nHeaderSize);

  switch (aHM.nHeaderSize)
  {
    case 12:
    {
      eVersion = BMP_Win2;
      break;
    }
    case 40:
    {
      if (((BMP_Win3_Header*) pBufPtr)->nCompression == 3)
        eVersion = BMP_WinNT;
      else
        eVersion = BMP_Win3;
      break;
    }
    case 64:
    {
      eVersion = BMP_OS2;
      break;
    }
    case 108:
    {
      eVersion = BMP_Win95;
      break;
    }
    default:
    {
      eVersion = BMP_None;
      break;
    }
  }

  ASSERT (eVersion != BMP_None);

  gtuint32 nWidth = 0;
  gtuint32 nHeight = 0;
  gtuint32 nBitsPerPixel = 0;
  bool bHasResolution = true;
  gtuint32 nXResolution = 0;
  gtuint32 nYResolution = 0;
  switch (eVersion)
  {
    case BMP_Win2:
    {
      nBitsPerPixel  = ((BMP_Win2_Header*) pBufPtr)->nBitsPerPixel;
      nHeight        = ((BMP_Win2_Header*) pBufPtr)->nHeight;
      nWidth         = ((BMP_Win2_Header*) pBufPtr)->nWidth;
      bHasResolution = false;
      break;
    }
    case BMP_Win3:
    case BMP_WinNT:
    {
      nBitsPerPixel = ((BMP_Win3_Header*) pBufPtr)->nBitsPerPixel;
      nHeight       = ((BMP_Win3_Header*) pBufPtr)->nHeight;
      nWidth        = ((BMP_Win3_Header*) pBufPtr)->nWidth;
      nXResolution  = ((BMP_Win3_Header*) pBufPtr)->nHorizRes;
      nYResolution  = ((BMP_Win3_Header*) pBufPtr)->nVertRes;
      break;
    }
    case BMP_Win95:
    {
      nBitsPerPixel = ((BMP_Win95_Header*) pBufPtr)->nBitsPerPixel;
      nWidth        = ((BMP_Win95_Header*) pBufPtr)->nWidth;
      nHeight       = ((BMP_Win95_Header*) pBufPtr)->nHeight;
      nXResolution  = ((BMP_Win95_Header*) pBufPtr)->nHorizRes;
      nYResolution  = ((BMP_Win95_Header*) pBufPtr)->nVertRes;
      break;
    }
    case BMP_OS2:
    {
      nBitsPerPixel = ((BMP_OS2_Header*) pBufPtr)->nBitsPerPixel;
      nWidth        = ((BMP_OS2_Header*) pBufPtr)->nWidth;
      nHeight       = ((BMP_OS2_Header*) pBufPtr)->nHeight;
      nXResolution  = ((BMP_OS2_Header*) pBufPtr)->nHorizRes;
      nYResolution  = ((BMP_OS2_Header*) pBufPtr)->nVertRes;
      break;
    }
    case BMP_None:
    {
      ASSERT (0);
      break;
    }
  }

  // get BMP type string
  pstring sType;
  str_assign_from_res (sType, eVersion == BMP_Win2  ? HRC_GENLIB_BMP_TYPE2 :
                              eVersion == BMP_Win3  ? HRC_GENLIB_BMP_TYPE3_3X :
                              eVersion == BMP_WinNT ? HRC_GENLIB_BMP_TYPE3_NT :
                              eVersion == BMP_Win95 ? HRC_GENLIB_BMP_TYPE4 :
                              eVersion == BMP_OS2   ? HRC_GENLIB_BMP_TYPE3_OS2 :
                                                      HRC_GENLIB_BMP_TYPE_UNKNOWN);
  // print type, resolution etc
  out_format (rc (HRC_GENLIB_BMP_SPEC),
              sType.c_str (),
              nWidth,
              nHeight,
              1 << nBitsPerPixel);

  if (bHasResolution)
  {
    out_format (rc (HRC_GENLIB_BMP_RES),
                nXResolution,
                nYResolution);
  }

  out_append (_T ("\n"));
}
