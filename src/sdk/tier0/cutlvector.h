inline int UtlMemory_CalcNewAllocationCount( int nAllocationCount, int nGrowSize, int nNewSize, int nBytesItem )
{
    if ( nGrowSize )
        nAllocationCount = ( ( 1 + ( ( nNewSize - 1 ) / nGrowSize ) ) * nGrowSize );
    else
    {
        if ( !nAllocationCount )
            nAllocationCount = ( 31 + nBytesItem ) / nBytesItem;

        while ( nAllocationCount < nNewSize )
            nAllocationCount *= 2;
    }

    return nAllocationCount;
}

template< class TI, class I = int >
class CUtlMemory
{
public:
    TI& operator[]( I i )
    {
        return m_pMemory[i];
    }

    TI* Base()
    {
        return m_pMemory;
    }

    int NumAllocated() const
    {
        return m_nAllocationCount;
    }

    void Grow( int num = 1 )
    {
        if ( IsExternallyAllocated() )
            return;

        int nAllocationRequested = m_nAllocationCount + num;
        int nNewAllocationCount = UtlMemory_CalcNewAllocationCount( m_nAllocationCount, m_nGrowSize, nAllocationRequested, sizeof( TI ) );

        if ( (int) (I) nNewAllocationCount < nAllocationRequested )
        {
            if ( (int) (I) nNewAllocationCount == 0 && (int) (I) ( nNewAllocationCount - 1 ) >= nAllocationRequested )
                --nNewAllocationCount;
            else
            {
                if ( (int) (I) nAllocationRequested != nAllocationRequested )
                    return;

                while ( (int) (I) nNewAllocationCount < nAllocationRequested )
                    nNewAllocationCount = ( nNewAllocationCount + nAllocationRequested ) / 2;
            }
        }

        m_nAllocationCount = nNewAllocationCount;

        /*if ( m_pMemory )
            m_pMemory = (T*) g_pMemAlloc->Realloc( m_pMemory, m_nAllocationCount * sizeof( T ) );
        else
            m_pMemory = (T*) g_pMemAlloc->Alloc( m_nAllocationCount * sizeof( T ) );*/
    }

    bool IsExternallyAllocated() const
    {
        return m_nGrowSize < 0;
    }

protected:
    TI* m_pMemory;
    int m_nAllocationCount;
    int m_nGrowSize;
};

template <class T>
inline T* Construct( T* pMemory )
{
    return ::new( pMemory ) T;
}

template <class T>
inline void Destruct( T* pMemory )
{
    pMemory->~T();
}

template< class T, class A = CUtlMemory<T> >
class CUtlVector
{
    typedef A CAllocator;
public:
    T& operator[]( int i )
    {
        return m_Memory[i];
    }

    T& Element( int i )
    {
        return m_Memory[i];
    }

    T* Base()
    {
        return m_Memory.Base();
    }

    int Count() const
    {
        return m_Size;
    }

    void RemoveAll()
    {
        for ( int i = m_Size; --i >= 0; )
            Destruct( &Element( i ) );

        m_Size = 0;
    }

    int AddToTail()
    {
        return InsertBefore( m_Size );
    }

    int InsertBefore( int elem )
    {
        GrowVector();
        ShiftElementsRight( elem );
        Construct( &Element( elem ) );

        return elem;
    }

protected:
    void GrowVector( int num = 1 )
    {
        if ( m_Size + num > m_Memory.NumAllocated() )
            m_Memory.Grow( m_Size + num - m_Memory.NumAllocated() );

        m_Size += num;
        ResetDbgInfo();
    }

    void ShiftElementsRight( int elem, int num = 1 )
    {
        int numToMove = m_Size - elem - num;
        if ( ( numToMove > 0 ) && ( num > 0 ) )
            memmove( &Element( elem + num ), &Element( elem ), numToMove * sizeof( T ) );
    }

    CAllocator m_Memory;
    int m_Size;

    T* m_pElements;

    inline void ResetDbgInfo()
    {
        m_pElements = Base();
    }
};  