#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdio.h>

#ifdef DEBUG
	#define DEBUG_PRINT(x) printf(x)
	#define DEBUG_CALL(x) x;
#else
	#define DEBUG_PRINT(x)
	#define DEBUG_CALL(x)
#endif

#endif