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

class MidiDevicesImp : public MidiDevices, private Timer, private Thread
{
private:
  struct CompareDevices
  {
    static int compareElements (Device const* const lhs, Device const* const rhs)
    {
      return lhs->getName ().compare (rhs->getName ());
    }

    static int compareElements (String const s, Device const* const rhs)
    {
      return s.compare (rhs->getName ());
    }
  };

  //============================================================================

  class InputImp : public Input
  {
  private:
    InputImp& operator= (InputImp const&);

    String const m_name;
    int m_deviceIndex;

  public:
    InputImp (String const name, int deviceIndex)
      : m_name (name)
      , m_deviceIndex (deviceIndex)
    {
    }

    String getName () const
    {
      return m_name;
    }

    int getDeviceIndex () const
    {
      return m_deviceIndex;
    }

    void setDeviceIndex (int newDeviceIndex)
    {
      m_deviceIndex = newDeviceIndex;
    }
  };

  //============================================================================

  class OutputImp : public Output
  {
  private:
    OutputImp& operator= (OutputImp const&);

    String const m_name;
    int m_deviceIndex;

  public:
    OutputImp (String const name, int deviceIndex)
      : m_name (name)
      , m_deviceIndex (deviceIndex)
    {
    }

    String getName () const
    {
      return m_name;
    }

    int getDeviceIndex () const
    {
      return m_deviceIndex;
    }

    void setDeviceIndex (int newDeviceIndex)
    {
      m_deviceIndex = newDeviceIndex;
    }
  };

private:
  //============================================================================

  struct State
  {
    OwnedArray <InputImp> inputs;
    OwnedArray <OutputImp> outputs;
  };

  typedef ConcurrentState <State> StateType;

  StateType m_state;
  vf::Listeners <Listener> m_listeners;

public:
  //============================================================================

  MidiDevicesImp () : Thread ("MidiDevices")
  {
    // This object must be created from the JUCE Message Thread.
    //
    jassert (MessageManager::getInstance()->isThisTheMessageThread());

    //startTimer (1000);
    startThread ();
  }

  ~MidiDevicesImp ()
  {
    stopThread (-1);
  }

  void addListener (Listener* listener, CallQueue& thread)
  {
    m_listeners.add (listener, thread);
  }

  void removeListener (Listener* listener)
  {
    m_listeners.remove (listener);
  }

private:
  //----------------------------------------------------------------------------

  void scanInputDevices ()
  {
#if 0
    StateType::WriteAccess state (m_state);

    // Make a copy of the currently connected devices.
    Array <InputImp*> disconnected;
    disconnected.ensureStorageAllocated (state->inputs.size ());
    for (int i = 0; i < state->inputs.size (); ++i)
    {
      InputImp* const device = state->inputs [i];
      if (device->getDeviceIndex () != -1)
        disconnected.add (device);
    }

    // Enumerate connected devices.
    StringArray const devices (juce::MidiInput::getDevices ());
    for (int deviceIndex = 0; deviceIndex < devices.size (); ++deviceIndex)
    {
      CompareDevices comp;
      String const deviceName (devices [deviceIndex]);

      // Remove this device from list of disconnected devices.
      {
        int const index = disconnected.indexOfSorted (comp, deviceName);
        if (index != -1)
          disconnected.remove (index);
      }

      // Find this device in our list.
      int const index = state->inputs.indexOfSorted (comp, deviceName);

      if (index != -1)
      {
        // Device already exists
        InputImp* const device = state->inputs [index];

        // Notify listeners with connected status.
        if (device->getDeviceIndex () == -1)
        {
          m_listeners.queue (&Listener::onMidiDevicesStatus, device, true);

          Logger::outputDebugString (device->getName () + ": connected");
        }

        device->setDeviceIndex (deviceIndex);
      }
      else
      {
        InputImp* const device = new InputImp (deviceName, deviceIndex);

        state->inputs.addSorted (comp, device);
      }
    }

    // Notify listeners with disconnected status.
    for (int i = 0; i < disconnected.size (); ++i)
    {
      InputImp* const device = disconnected [i];
      device->setDeviceIndex (-1);

      m_listeners.queue (&Listener::onMidiDevicesStatus, device, false);

      Logger::outputDebugString (device->getName () + ": disconnected");
    }
#else
    jassertfalse;
#endif
  }

  //----------------------------------------------------------------------------

  void scanOutputDevices ()
  {
    StateType::WriteAccess state (m_state);

    StringArray const devices (juce::MidiOutput::getDevices ());
  }

  //----------------------------------------------------------------------------

  void timerCallback ()
  {
    scanInputDevices ();
    scanOutputDevices ();
  }

  void run ()
  {
    do
    {
      wait (1000);

      scanInputDevices ();
      scanOutputDevices ();
    }
    while (!threadShouldExit ());
  }
};

//==============================================================================

MidiDevices* MidiDevices::createInstance ()
{
  return new MidiDevicesImp;
}
