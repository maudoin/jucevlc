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

InterruptibleThread::ThreadHelper::ThreadHelper (String name,
                                                 InterruptibleThread* owner)
  : Thread (name)
  , m_owner (owner)
{
}

InterruptibleThread* InterruptibleThread::ThreadHelper::getOwner () const
{
  return m_owner;
}

void InterruptibleThread::ThreadHelper::run ()
{
  m_owner->run ();
}

//------------------------------------------------------------------------------

InterruptibleThread::InterruptibleThread (String name)
  : m_thread (name, this)
  , m_state (stateRun)
{
}

InterruptibleThread::~InterruptibleThread ()
{
  m_runEvent.signal ();

  join ();
}

void InterruptibleThread::start (const Function <void (void)>& f)
{
  m_function = f;

  m_thread.startThread ();

  // prevents data race with member variables
  m_runEvent.signal ();
}

void InterruptibleThread::join ()
{
  m_thread.stopThread (-1);
}

bool InterruptibleThread::wait (int milliSeconds)
{
  // Can only be called from the current thread
  jassert (isTheCurrentThread ());

  bool interrupted = false;

  for (;;)
  {
    jassert (m_state != stateWait);

    // See if we are interrupted
    if (m_state.tryChangeState (stateInterrupt, stateRun))
    {
      // We were interrupted, state is changed to Run.
      // Caller must run now.
      interrupted = true;
      break;
    }
    else if (m_state.tryChangeState (stateRun, stateWait) ||
      m_state.tryChangeState (stateReturn, stateWait))
    {
      // Transitioned to wait.
      // Caller must wait now.
      interrupted = false;
      break;
    }
  }

  if (!interrupted)
  {
    interrupted = m_thread.wait (milliSeconds);

    if (!interrupted)
    {
      if (m_state.tryChangeState (stateWait, stateRun))
      {
        interrupted = false;
      }
      else
      {
        jassert (m_state == stateInterrupt);

        interrupted = true;
      }
    }
  }

  return interrupted;
}

void InterruptibleThread::interrupt ()
{
  for (;;)
  {
    int const state = m_state;

    if (state == stateInterrupt ||
      state == stateReturn ||
      m_state.tryChangeState (stateRun, stateInterrupt))
    {
      // Thread will see this at next interruption point.
      break;
    }
    else if (m_state.tryChangeState (stateWait, stateRun))
    {
      m_thread.notify ();
      break;
    }
  }
}

bool InterruptibleThread::interruptionPoint ()
{
  // Can only be called from the current thread
  jassert (isTheCurrentThread ());

  if (m_state == stateWait)
  {
    // It is impossible for this function
    // to be called while in the wait state.
    Throw (Error().fail (__FILE__, __LINE__));
  }
  else if (m_state == stateReturn)
  {
    // If this goes off it means the thread called the
    // interruption a second time after already getting interrupted.
    Throw (Error().fail (__FILE__, __LINE__));
  }

  // Switch to Return state if we are interrupted
  //bool const interrupted = m_state.tryChangeState (stateInterrupt, stateReturn);
  bool const interrupted = m_state.tryChangeState (stateInterrupt, stateRun);

  return interrupted;
}

InterruptibleThread::id InterruptibleThread::getId () const
{
  return m_threadId;
}

bool InterruptibleThread::isTheCurrentThread () const
{
  return m_thread.getCurrentThreadId () == m_threadId;
}

void InterruptibleThread::setPriority (int priority)
{
  m_thread.setPriority (priority);
}

InterruptibleThread* InterruptibleThread::getCurrentThread ()
{
  Thread* const thread = Thread::getCurrentThread();

  // This doesn't work for the message thread!
  jassert (thread != nullptr);

  ThreadHelper* const helper = dynamic_cast <ThreadHelper*> (thread);

  jassert (helper != nullptr);

  return helper->getOwner ();
}

void InterruptibleThread::run ()
{
  m_threadId = m_thread.getThreadId ();

  m_runEvent.wait ();

  CatchAny (m_function);
}

//------------------------------------------------------------------------------

bool CurrentInterruptibleThread::interruptionPoint ()
{
  bool interrupted = false;

#if 1
  interrupted = InterruptibleThread::getCurrentThread ()->interruptionPoint ();

#else
  Thread* const thread = Thread::getCurrentThread();

  // Can't use interruption points on the message thread
  jassert (thread != nullptr);

  if (thread)
  {
    InterruptibleThread* const interruptibleThread = dynamic_cast <InterruptibleThread*> (thread);

    jassert (interruptibleThread != nullptr);

    if (interruptibleThread != nullptr)
    {
      interrupted = interruptibleThread->interruptionPoint ();
    }
    else
    {
      interrupted = false;
    }
  }
  else
  {
    interrupted = false;
  }
#endif

  return interrupted;
}
