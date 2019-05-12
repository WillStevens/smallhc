#ifndef MACROS_H
#define MACROS_H

#define _STRINGIFY(x) #x
#define STRINGIFY(x) _STRINGIFY(x)

#define min(a,b) ((a)>(b)?(b):(a))
#define max(a,b) ((a)>(b)?(a):(b))

#endif