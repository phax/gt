#ifndef _GT_TIMER_HXX_
#define _GT_TIMER_HXX_

namespace GT {

/*! This enum contains the possible states of the timer.
 */
//--------------------------------------------------------------------
enum ETimerState
//--------------------------------------------------------------------
{
  TIMER_START = 889,  //!< timer running
  TIMER_STOP,         //!< timer stopped
};

//--------------------------------------------------------------------
class GT_EXPORT_BASIC Timer
//--------------------------------------------------------------------
{
private:
  gtint32 m_nTicks;
  gtint32 m_nStart;    //!< start tick count
  bool    m_bRunning;

public:
  //------------------------------------------------------------------
  explicit Timer ()
  //------------------------------------------------------------------
    : m_nTicks   (0),
      m_nStart   (0),
      m_bRunning (false)
  {}

  //! start the timer
  void   GT_CALL Start ();

  //! stop the timer
  void   GT_CALL Stop ();

  //! get the elapsed seconds - the timer must be stopped to do this
  double GT_CALL GetSeconds () const;
};

}  // namespace GT

#endif
