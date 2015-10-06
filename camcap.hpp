// The MIT License (MIT)
// 
// Copyright (c) 2015 Manu Marin / @gyakoo
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/*
DOCUMENTATION

*/

#ifndef _CAMCAP_H_
#define _CAMCAP_H_

/////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4244 4100 4996 4706 4091 4310)
#endif

// Error codes



#ifdef CAMCAP_IMPLEMENTATION
#if defined(__x86_64__) || defined(_M_X64)  ||  defined(__aarch64__)   || defined(__64BIT__) || \
  defined(__mips64)     || defined(__powerpc64__) || defined(__ppc64__)
#	define CAMCAP_PLATFORM_64
# define CAMCAP_ALIGNMENT 16
#else
#	define CAMCAP_PLATFORM_32
# define CAMCAP_ALIGNMENT 8
#endif //

#if defined(_DEBUG) || defined(DEBUG)
#define CAMCAP_BREAK { __debugbreak(); }
#define CAMCAP_ASSERT(c) { if (!(c)){ CAMCAP_BREAK; } }
#else
#define CAMCAP_BREAK {(void*)0;}
#define CAMCAP_ASSERT(c)
#endif

#define CAMCAP_BREAK_ALWAYS { __debugbreak(); }

#endif // CAMCAP_IMPLEMENTATION

#endif // _CAMCAP_H_