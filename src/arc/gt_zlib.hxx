#ifndef _GT_ZLIB_HXX_
#define _GT_ZLIB_HXX_

gtint32 UpdateCRC32 (gtint32 nCurCRC, void *pBuf, gtuint32 nBufSize)
{
  static bool bInited = false;
  static gtint32 aCRC32Table[256];

  if (!bInited)
  {
    for (int i = 0; i < 256; i++)
    {
      gtint32 nCRC = i;
      for (int n = 0; n < 8; n++)
        if (nCRC & 1)
          nCRC = (nCRC >> 1) ^ gtint32 (0xEDB88320);
        else
          nCRC = nCRC >> 1;
      aCRC32Table[i] = nCRC;
    }
    bInited = true;
  }

  char *pBuf1 = (char*) pBuf;
  while (nBufSize > 0)
  {
    nCurCRC = aCRC32Table[(nCurCRC ^ *pBuf1) & 0xFF] ^ (nCurCRC >> 8);
    nBufSize--;
    pBuf1++;
  }

  return nCurCRC;
}

inline gtint32 GetCRC32 (void *pBuf, gtuint32 nBufSize)
{
  return UpdateCRC32 (gtint32 (0xFFFFFFFF), pBuf, nBufSize) ^ gtint32 (0xFFFFFFFF);
}

#endif
