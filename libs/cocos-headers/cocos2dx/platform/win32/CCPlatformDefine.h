#ifndef __CCPLATFORMDEFINE_H__
#define __CCPLATFORMDEFINE_H__

#ifdef __MINGW32__
#include <string.h>
#endif



/*#if defined(_USRDLL)
    #define CC_DLL     __declspec(dllexport)
#else         // use a DLL library 
    #define CC_DLL     __declspec(dllimport)
#endif*/

/** CC_DLL
* Since we are linking a lib containing the functions
* provided by libcocos2d.dll used by GeometryDash.exe, 
* we do not need to define this macro to do anything.
*/
#define CC_DLL

#include <assert.h>

#if CC_DISABLE_ASSERT > 0
#define CC_ASSERT(cond)
#else
#define CC_ASSERT(cond)    assert(cond)
#endif
#define CC_UNUSED_PARAM(unusedparam) (void)unusedparam

/* Define NULL pointer value */
#ifndef NULL
#ifdef __cplusplus
#define NULL    0
#else
#define NULL    ((void *)0)
#endif
#endif



#endif /* __CCPLATFORMDEFINE_H__*/
