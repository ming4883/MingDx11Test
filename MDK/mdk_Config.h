#ifndef MDK_CONFIG_H_INCLUDED
#define MDK_CONFIG_H_INCLUDED


#include <AppConfig.h>
#include <cassert>

#define m_stringify(x) #x
#define m_tostr(x) m_stringify(x)

#define m_assert(x) assert(x)

#define m_static_assert(x) static_assert(x)

#define m_noncopyable(x) private: x(const x&); void operator = (const x&);

#endif // MDK_CONFIG_H_INCLUDED
