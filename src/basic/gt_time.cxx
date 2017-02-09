#include "gt_time.hxx"
#include "gt_stringhelper.hxx"

#define GTLIB_TIME_ISLEAP(y) (!((y) % 4) && ((y) % 100 || !((y) % 400)))

namespace GT {

//--------------------------------------------------------------------
void GT_CALL DateTime::SetDOSTime
                                        (const gtuint32 nDOSDT)
//--------------------------------------------------------------------
{
  gtuint16 *pDOSDT = (gtuint16*) &nDOSDT;

  nYear  = (pDOSDT[1] >> 9) + 1980;
  nMonth = (pDOSDT[1] >> 5) & 15;
  nDay   = (pDOSDT[1] & 31);
  nHour  = (pDOSDT[0] >> 11);
  nMin   = (pDOSDT[0] >> 5) & 63;
  nSec   = (pDOSDT[0] & 31) << 1;
}

//--------------------------------------------------------------------
void GT_CALL DateTime::SetWindowsTime
                                        (const gtuint64 dWinDT)
//--------------------------------------------------------------------
{
  WORD nDate, nTime;
  FileTimeToDosDateTime ((FILETIME*) &dWinDT, &nDate, &nTime);  // kernel32.dll
  SetDOSTime ((nDate << 16) + nTime);
}

//--------------------------------------------------------------------
void GT_CALL DateTime::SetUnixTime
                                        (gtuint32 nUnixDT)
//--------------------------------------------------------------------
{
  const int DAYS_PER_MONTH [12]     = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  const int DAYS_PER_LEAPMONTH [12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  const int DAYS_PER_YEAR [12]      = { 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
  const int DAYS_PER_LEAPYEAR [12]  = { 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366 };

  const gtuint32 SECS_PER_MINUTE   = 60;
  const gtuint32 SECS_PER_HOUR     = 3600;
  const gtuint32 SECS_PER_DAY      = 86400;
  const gtuint32 SECS_PER_YEAR     = 31536000;
  const gtuint32 SECS_PER_LEAPYEAR = 31622400;

  nYear = 1970;
  nMonth = 1;
  nDay = 1;

  bool bDone = false;
  while (!bDone)
  {
    if (nUnixDT >= SECS_PER_LEAPYEAR && GTLIB_TIME_ISLEAP (nYear + 1))
    {
      nYear++;
      nUnixDT -= SECS_PER_LEAPYEAR;
    }
    else
    if (nUnixDT >= SECS_PER_YEAR)
    {
      nYear++;
      nUnixDT -= SECS_PER_YEAR;
    }
    else
      bDone = true;
  }

  bDone = false;
  const gtint32 nTotalDays = nUnixDT / SECS_PER_DAY;
  if (GTLIB_TIME_ISLEAP (nYear))
  {
    while (!bDone)
    {
      if (nTotalDays <= DAYS_PER_LEAPYEAR [nMonth - 1])
      {
        bDone = true;
        nUnixDT -= (nTotalDays * SECS_PER_DAY);
        nDay = DAYS_PER_LEAPMONTH [nMonth - 1] - (DAYS_PER_LEAPYEAR [nMonth - 1] - nTotalDays) + 1;
      }
      else
        nMonth++;
    }
  }
  else
  {
    while (!bDone)
    {
      if (nTotalDays <= DAYS_PER_YEAR [nMonth - 1])
      {
        bDone = true;
        nUnixDT -= (nTotalDays * SECS_PER_DAY);
        nDay = DAYS_PER_MONTH [nMonth - 1] - (DAYS_PER_YEAR [nMonth - 1] - nTotalDays) + 1;
      }
      else
        nMonth++;
    }
  }

  nHour = nUnixDT / SECS_PER_HOUR;
  nUnixDT -= nHour * SECS_PER_HOUR;

  nMin = nUnixDT / SECS_PER_MINUTE;
  nUnixDT -= nMin * SECS_PER_MINUTE;

  nSec = nUnixDT;
}

/*! Get the current date as string.
    \param bLongFormat if true, the year is printed with 4 digits.
 */
//--------------------------------------------------------------------
void GT_CALL DateTime::DateToString
                                        (      pstring& sRes,
                                         const bool     bLongFormat)
//--------------------------------------------------------------------
{
  if (bLongFormat)
    str_assign_format (sRes, _T ("%02d.%02d.%04d"), nDay, nMonth, nYear);
  else
    str_assign_format (sRes, _T ("%02d.%02d.%02d"), nDay, nMonth, nYear % 100);
}

/*! Get the current time as string.
 */
//--------------------------------------------------------------------
void GT_CALL DateTime::TimeToString
                                        (pstring& sRes)
//--------------------------------------------------------------------
{
  str_assign_format (sRes, _T ("%02d:%02d:%02d"), nHour, nMin, nSec);
}

}  // namespace
