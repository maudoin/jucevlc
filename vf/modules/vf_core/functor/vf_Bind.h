/*============================================================================*/
/*
  VFLib: https://github.com/vinniefalco/VFLib

  Copyright (C) 2008 by Vinnie Falco <vinnie.falco@gmail.com>

  This library contains portions of other open source products covered by
  separate licenses. Please see the corresponding source files for specific
  terms.
  
  VFLib is provided under the terms of The MIT License (MIT):

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/
/*============================================================================*/

#ifndef VF_BIND_VFHEADER
#define VF_BIND_VFHEADER

/* Brings functional support into our namespace, based on environment.
*/
#if JUCE_MSVC
  // Visual Studio has these in std.
  using std::ref;
  using std::bind;
  using std::function;
  using std::placeholders::_1;
  using std::placeholders::_2;

#elif JUCE_IOS
  #if VF_USE_BOOST
    /* If boost is activated, use it. This works
       around a bug with the iOS implementation of bind.
    */
    using boost::ref
    using boost::bind;
    using boost::function;
    using ::_1;
    using ::_2;
  #else
    #if _LIBCPP_VERSION // libc++
      using std::ref;
      using std::bind;
      using std::function;
      using std::placeholders::_1;
      using std::placeholders::_2;
    #else // libstdc++ (GNU)
      using std::tr1::ref;
      using std::tr1::bind;
      using std::tr1::function;
      using std::tr1::placeholders::_1;
      using std::tr1::placeholders::_2;
    #endif
  #endif

#elif JUCE_MAC
  #if _LIBCPP_VERSION // libc++
    using std::ref;
    using std::bind;
    using std::function;
    using std::placeholders::_1;
    using std::placeholders::_2;
  #else // libstdc++ (GNU)
    using std::tr1::ref;
    using std::tr1::bind;
    using std::tr1::function;
    using std::tr1::placeholders::_1;
    using std::tr1::placeholders::_2;
  #endif

#elif JUCE_LINUX || JUCE_MINGW
  using std::tr1::bind;
  using std::tr1::placeholders::_1;
  using std::tr1::placeholders::_2;

#else
  #error Unknown platform in vf_Bind.h

#endif

#endif
