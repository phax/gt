#ifndef _GEE_PERMGEN_HXX_
#define _GEE_PERMGEN_HXX_

namespace GEE {

class PermutationGenerator
{
private:
  size_t   m_nWorkSize;
  int     *m_pWork;
  gtint64  m_aNumLeft;
  gtint64  m_aTotal;

public:
  // -----------------------------------------------------------
  // Constructor. WARNING: Don't make n too large.
  // Recall that the number of permutations is n!
  // which can be very large, even when n is as small as 20 --
  // 20! = 2,432,902,008,176,640,000 and
  // 21! is too big to fit into a Java long, which is
  // why we use BigInteger instead.
  // ----------------------------------------------------------

  PermutationGenerator (const size_t n)
    : m_nWorkSize (n)
  {
    ASSERT (n >= 1 && n<= 20); // "Minimum is 1"
    m_pWork = new int [n];
    m_aTotal = getFactorial (n);
    reset ();
  }

  virtual ~PermutationGenerator ()
  {
    delete [] m_pWork;
  }

  // ------
  // Reset
  // ------

  void reset ()
  {
    for (size_t i = 0; i < m_nWorkSize; ++i)
      m_pWork[i] = (int) i;
    m_aNumLeft = m_aTotal;
  }

  // ------------------------------------------------
  // Return number of permutations not yet generated
  // ------------------------------------------------

  gtint64 getNumLeft ()
  {
    return m_aNumLeft;
  }

  // ------------------------------------
  // Return total number of permutations
  // ------------------------------------

  gtint64 getTotal ()
  {
    return m_aTotal;
  }

  // -----------------------------
  // Are there more permutations?
  // -----------------------------

  bool hasMore ()
  {
    return m_aNumLeft > 1;
  }

  // ------------------
  // Compute factorial
  // ------------------

  static gtint64 getFactorial (const size_t n)
  {
    gtint64 ret = 1, prev;
    for (size_t i = n; i > 1; i--)
    {
      prev = ret;
      ret = ret * i;
      ASSERT (prev < ret);  // else overflow
    }
    return ret;
  }

  size_t getWorkItemCount ()
  {
    return m_nWorkSize;
  }

  // --------------------------------------------------------
  // Generate next permutation (algorithm from Rosen p. 284)
  // --------------------------------------------------------

  int* getNext ()
  {
    // For the first time, just decrement by one
    if (m_aNumLeft == m_aTotal)
    {
      --m_aNumLeft;
    }
    else
    {
      // Find largest index j with a[j] < a[j+1]
      size_t j = m_nWorkSize - 2;
      while (m_pWork[j] > m_pWork[j + 1])
      {
        j--;
      }

      // Find index k such that a[k] is smallest integer
      // greater than a[j] to the right of a[j]
      size_t k = m_nWorkSize - 1;
      while (m_pWork[j] > m_pWork[k])
      {
        k--;
      }

      // Interchange a[j] and a[k]
      ASSERT (k != j);
      int temp = m_pWork[k];
      m_pWork[k] = m_pWork[j];
      m_pWork[j] = temp;

      // Put tail end of permutation after jth position in increasing order
      size_t r = m_nWorkSize - 1;
      size_t s = j + 1;

      while (r > s)
      {
        temp = m_pWork[s];
        m_pWork[s] = m_pWork[r];
        m_pWork[r] = temp;
        r--;
        s++;
      }

      --m_aNumLeft;
    }
    return m_pWork;
  }
};

}  // namespace

#endif