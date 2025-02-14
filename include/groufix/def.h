/**
 * This file is part of groufix.
 * Copyright (c) Stef Velzel. All rights reserved.
 *
 * groufix : graphics engine produced by Stef Velzel.
 * www     : <www.vuzzel.nl>
 */


#ifndef GFX_DEF_H
#define GFX_DEF_H

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#if defined (__STDC_NO_ATOMICS__)
	#error "Host platform does not support atomics."
#endif

#include <stdatomic.h>


/**
 * Identification of the host platform.
 */
#if defined (__unix) || defined (__unix__)
	#define GFX_UNIX
#elif defined (_WIN32) || defined (__WIN32__) || defined (WIN32) || defined (__MINGW32__)
	#define GFX_WIN32
#else
	#error "Host platform not supported by groufix."
#endif


/**
 * Windows XP minimum.
 */
#if defined (GFX_WIN32)
	#if WINVER < 0x0501
		#undef WINVER
		#define WINVER 0x0501
	#endif
	#if _WIN32_WINNT < 0x0501
		#undef _WIN32_WINNT
		#define _WIN32_WINNT 0x0501
	#endif
#endif


/**
 * DLL import/export interface.
 */
#if defined (GFX_WIN32)
	#if defined (GFX_BUILD_LIB)
		#define GFX_LIB __declspec(dllexport)
	#else
		#define GFX_LIB __declspec(dllimport)
	#endif
#else
	#define GFX_LIB
#endif


/**
 * groufix API linkage.
 */
#if defined (__cplusplus)
	#define GFX_API extern "C" GFX_LIB
#else
	#define GFX_API extern GFX_LIB
#endif


/**
 * Platform agnostic size_t print format.
 */
#if defined (GFX_WIN32)
	#define GFX_PRIs "Iu"
#else
	#define GFX_PRIs "zu"
#endif


/**
 * General usefulness.
 */
#define GFX_MIN(x, y) \
	((x) < (y) ? (x) : (y))

#define GFX_MAX(x, y) \
	((x) > (y) ? (x) : (y))

#define GFX_DIFF(x, y) \
	((x) > (y) ? (x) - (y) : (y) - (x))

#define GFX_CLAMP(x, l, u) \
	((x) < (l) ? (l) : (x) > (u) ? (u) : (x))


#define GFX_IS_POWER_OF_TWO(x) /* 0 counts. */ \
	(((x) & ((x) - 1)) == 0)

#define GFX_ALIGN_UP(offset, align) /* align must be a power of 2. */ \
	(((offset) + (align) - 1) & ~((align) - 1))

#define GFX_ALIGN_DOWN(offset, align) /* align must be a power of 2. */ \
	((offset) & ~((align) - 1))


#endif
