/*****************************************************************************

        conf.h
        Author: Laurent de Soras, 2005

Depending on your CPU, define/undef symbols in this file.

--- Legal stuff ---

This program is free software. It comes without any warranty, to
the extent permitted by applicable law. You can redistribute it
and/or modify it under the terms of the Do What The Fuck You Want
To Public License, Version 2, as published by Sam Hocevar. See
http://sam.zoy.org/wtfpl/COPYING for more details.

*Tab=3***********************************************************************/



#if ! defined (hiir_test_conf_HEADER_INCLUDED)
#define hiir_test_conf_HEADER_INCLUDED

#if defined (_MSC_VER)
	#pragma once
	#pragma warning (4 : 4250) // "Inherits via dominance."
#endif



/*\\\ INCLUDE FILES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/



namespace hiir
{
namespace test
{



// CPU configuration (check and modify this, depending on your CPU)
#define hiir_test_3DNOW
#define hiir_test_SSE
#define hiir_test_NEON



// Removes code that isn't available for compilers/architectures
#if defined (__POWERPC__) || defined (__powerpc) || defined (_powerpc)

	#undef hiir_test_3DNOW
	#undef hiir_test_SSE
	#undef hiir_test_NEON

#elif defined (__arm__) || defined (__arm) || defined (__arm64__) || defined (__arm64) || defined (_M_ARM) || defined (__aarch64__)

	#undef hiir_test_3DNOW
	#undef hiir_test_SSE

#elif defined (__i386__) || defined (_M_IX86) || defined (_X86_) || defined (_M_X64) || defined (__x86_64__) || defined (__INTEL__)

	#undef hiir_test_NEON

	#if (! defined (_MSC_VER) || defined (_WIN64))
		#undef hiir_test_3DNOW
	#endif

#else

	#undef hiir_test_3DNOW
	#undef hiir_test_SSE
	#undef hiir_test_NEON

#endif



// Testing options
#undef  hiir_test_SAVE_RESULTS
#define hiir_test_LONG_FUNC_TESTS
#define hiir_test_LONG_SPEED_TESTS



}  // namespace test
}  // namespace hiir



#endif   // hiir_test_conf_HEADER_INCLUDED



/*\\\ EOF \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/
