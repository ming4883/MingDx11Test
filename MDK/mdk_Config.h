#ifndef MDK_CONFIG_H_INCLUDED
#define MDK_CONFIG_H_INCLUDED


#include <AppConfig.h>
#include <cassert>

#define m_stringify(x) #x
#define m_tostr(x) m_stringify(x)

#define m_assert(x) assert(x)

#define m_static_assert(x) static_assert(x)

#define m_noncopyable(x) private: x(const x&); void operator = (const x&);

namespace mdk
{

//! Used by some of the containers, user can add specialization of this function template for each new type to control how it is swapped.
template<typename DataType>
static void swap (DataType& a, DataType& b)
{
    DataType tmp = a;
    a = b;
    b = tmp;
}

template<typename T, int N>
inline int countof (T (&plainArray)[N])
{
    (void) plainArray;
    (void) sizeof (0[plainArray]); // This line should cause an error if you pass an object with a user-defined subscript operator
    return N;
}

}

#endif // MDK_CONFIG_H_INCLUDED
