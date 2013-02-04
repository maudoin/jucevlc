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

#ifndef VF_ONCEPERSECOND_VFHEADER
#define VF_ONCEPERSECOND_VFHEADER

#include "../containers/vf_List.h"

/*============================================================================*/
/** 
    Provides a once per second notification.

    Derive your class from OncePerSecond and override doOncePerSecond(). Then,
    call startOncePerSecond() to begin receiving the notifications. No clean-up
    or other actions are required.

    @ingroup vf_core
*/
class OncePerSecond : Uncopyable
{
public:
  OncePerSecond ();
  virtual ~OncePerSecond ();

  /** Begin receiving notifications. */
  void startOncePerSecond ();

  /** Stop receiving notifications. */
  void endOncePerSecond ();

protected:
  /** Called once per second. */
  virtual void doOncePerSecond () = 0;

private:
  class TimerSingleton;
  typedef ReferenceCountedObjectPtr <TimerSingleton> TimerPtr;

  struct Elem : List <Elem>::Node
  {
    TimerPtr instance;
    OncePerSecond* object;
  };

  Elem m_elem;
};

#endif
