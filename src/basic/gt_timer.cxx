#include "gt_timer.hxx"

#include <time.h>

namespace GT {

/*! Start the timer.
    The timer should be stopped.
 */
//--------------------------------------------------------------------
void GT_CALL Timer::Start ()
//--------------------------------------------------------------------
{
  ASSERT (!m_bRunning);
  m_bRunning = true;
  m_nStart = clock ();
}

/*! Stop the timer.
    The timer should be running to do it.
 */
//--------------------------------------------------------------------
void GT_CALL Timer::Stop ()
//--------------------------------------------------------------------
{
  ASSERT (m_bRunning);
  m_nTicks += (clock () - m_nStart);
  m_bRunning = false;
}

/*! Get the number of elapsed second.
    The timer should be stoped to do this.
 */
//--------------------------------------------------------------------
double GT_CALL Timer::GetSeconds () const
//--------------------------------------------------------------------
{
  ASSERT (!m_bRunning);
  return double (m_nTicks) / CLOCKS_PER_SEC;
}

}  // namespace
