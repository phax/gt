#ifndef _GT_TIME_HXX_
#define _GT_TIME_HXX_

#ifdef _MSC_VER
#pragma once
#endif

namespace GT {

//--------------------------------------------------------------------
class GT_EXPORT_BASIC DateTime
//--------------------------------------------------------------------
{
private:
  int nYear;
  int nMonth;
  int nDay;
  int nHour;
  int nMin;
  int nSec;

public:
  DateTime ()
    : nYear  (-1),
      nMonth (-1),
      nDay   (-1),
      nHour  (-1),
      nMin   (-1),
      nSec   (-1)
  {}

  void GT_CALL SetDOSTime (const gtuint32 nDOSDT);

  void GT_CALL SetWindowsTime (const gtuint64 dWindowsDT);

  void GT_CALL SetUnixTime (gtuint32 nUnixDT);

  void GT_CALL DateToString (      pstring& sRes,
                             const bool     bLongFormat);

  void GT_CALL TimeToString (pstring& sRes);

  int GT_CALL GetYear  () const { return nYear; }
  int GT_CALL GetMonth () const { return nMonth; }
  int GT_CALL GetDay   () const { return nDay; }
  int GT_CALL GetHour  () const { return nHour; }
  int GT_CALL GetMin   () const { return nMin; }
  int GT_CALL GetSec   () const { return nSec; }
};

}  // namespace

#endif
