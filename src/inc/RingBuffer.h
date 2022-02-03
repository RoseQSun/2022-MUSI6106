#if !defined(__RingBuffer_hdr__)
#define __RingBuffer_hdr__

#include <cassert>
#include <algorithm>
#include <cmath>
#include <cstring>

/*! \brief implement a circular buffer of type T
*/
template <class T> 
class CRingBuffer
{
public:
    explicit CRingBuffer(int iBufferLengthInSamples) :
        m_iBuffLength(iBufferLengthInSamples),
        m_iReadIndex(0),
        m_iWriteIndex(0),
        m_ptBuff(0)
    {
        assert(iBufferLengthInSamples > 0);

        // allocate and init

        m_ptBuff = new T[m_iBuffLength];
    }

    virtual ~CRingBuffer()
    {
        // free memory
        delete [] m_ptBuff;
        m_ptBuff = nullptr;
    }

    /*! add a new value of type T to write index and increment write index
    \param tNewValue the new value
    \return void
    */
    void putPostInc (T tNewValue)
    {
        put(tNewValue);
        m_iWriteIndex = (m_iWriteIndex + 1 ) % m_iBuffLength;
    }

    /*! add a new value of type T to write index
    \param tNewValue the new value
    \return void
    */
    void put(T tNewValue)
    {
        m_ptBuff[m_iWriteIndex] = tNewValue;
    }
    
    /*! return the value at the current read index and increment the read pointer
    \return float the value from the read index
    */
    T getPostInc()
    {
        T currentValue = get();
        m_iReadIndex = (m_iReadIndex + 1 ) % m_iBuffLength;
        return currentValue;
    }

    /*! return the value at the current read index
    \return float the value from the read index
    */
    T get() const
    {
        return m_ptBuff[m_iReadIndex];
    }
    
    /*! set buffer content and indices to 0
    \return void
    */
    void reset()
    {
        memset (m_ptBuff, 0, sizeof(T) * m_iBuffLength);
        m_iReadIndex  = 0;
        m_iWriteIndex = 0;
    }

    /*! return the current index for writing/put
    \return int
    */
    int getWriteIdx() const
    {
        return m_iWriteIndex;
    }

    /*! move the write index to a new position
    \param iNewWriteIdx: new position
    \return void
    */
    void setWriteIdx(int iNewWriteIdx)
    {
        m_iWriteIndex = iNewWriteIdx;
    }

    /*! return the current index for reading/get
    \return int
    */
    int getReadIdx() const
    {
        return m_iReadIndex;
    }

    /*! move the read index to a new position
    \param iNewReadIdx: new position
    \return void
    */
    void setReadIdx(int iNewReadIdx)
    {
        m_iReadIndex = iNewReadIdx;
    }

    /*! returns the number of values currently buffered (note: 0 could also mean the buffer is full!)
    \return int
    */
    int getNumValuesInBuffer() const
    {
        return (m_iWriteIndex - m_iReadIndex + m_iBuffLength) % m_iBuffLength;
    }

    /*! returns the length of the internal buffer
    \return int
    */
    int getLength() const
    {
        return m_iBuffLength;
    }
private:
    CRingBuffer();
    CRingBuffer(const CRingBuffer& that);

    int m_iBuffLength;              //!< length of the internal buffer
    int m_iReadIndex;
    int m_iWriteIndex;

    T* m_ptBuff;
};
#endif // __RingBuffer_hdr__
