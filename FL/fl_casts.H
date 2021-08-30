// experimental inline cast functions for the Fast Light Toolkit (FLTK)

#include <FL/platform_types.h>

inline char fl_char(void *v) { return (char)(fl_intptr_t)v; }
inline int  fl_int(void *v)  { return (int)(fl_intptr_t)v; }
inline long fl_long(void *v) { return (long)(fl_intptr_t)v; }

inline unsigned char fl_uchar(void *v) { return (unsigned char)(fl_uintptr_t)v; }
inline unsigned int  fl_uint(void *v)  { return (unsigned int)(fl_uintptr_t)v; }
inline unsigned long fl_ulong(void *v) { return (unsigned long)(fl_uintptr_t)v; }

#if 0 /* not necessary */

inline char fl_char(size_t v) { return (char)v; }
inline int  fl_int(size_t v)  { return (int)v; }
inline long fl_long(size_t v) { return (long)v; }

inline unsigned char fl_uchar(size_t v) { return (unsigned char)v; }
inline unsigned int  fl_uint(size_t v)  { return (unsigned int)v; }
inline unsigned long fl_ulong(size_t v) { return (unsigned long)v; }

#endif /* not necessary */
