#include <math.h>

#ifdef WIN32
// things missing from <math.h>:

#define M_PI            3.14159265358979323846
#define M_PI_2          1.57079632679489661923
#define M_PI_4          0.78539816339744830962
#define M_1_PI          0.31830988618379067154
#define M_2_PI          0.63661977236758134308
#define M_SQRT2         1.41421356237309504880
#define M_SQRT1_2       0.70710678118654752440

#else

#ifdef __EMX__
#include <float.h>
#endif

#endif

#if defined(WIN32) || defined(CRAY)

inline double rint(double v) {return floor(v+.5);}
inline double copysign(double a, double b) {return b<0 ? -a : a;}

#endif
