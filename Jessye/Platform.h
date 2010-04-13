#ifndef PLATFORM_H
#define PLATFORM_H

#ifndef nullptr
#	define nullptr 0
#endif

#include <cstdlib>

#define js_safe_release(x) if(x) {x->Release(); x = nullptr;}
#define js_assert(x) assert(x && ##x);


#define js_decl_non_copyable(className)\
	className(const className&);\
	className& operator = (const className&);

#define js_max(a,b)            (((a) > (b)) ? (a) : (b))

#define js_min(a,b)            (((a) < (b)) ? (a) : (b))

#endif	// PLATFORM_H