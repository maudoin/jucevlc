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

#ifndef VF_COMPONENTBROADCAST_VFHEADER
#define VF_COMPONENTBROADCAST_VFHEADER

/*============================================================================*/
/**
  Broadcast to a Component and its children, recursively.

  This class functor will traverse a Component and its children recursively and
  call a member function for each Component that exposes the desired interface.
  A Component exposes the interface by deriving from the class containing the
  member function of interest. The implementation uses `dynamic_cast` to
  determine if the Component is eligible, so the interface must have a virtual
  table.

  This provides robust assistance for enforcing separation of concerns, and
  decentralizing program logic into only the areas that need it.

  In this example we will define an interface for saving and restoring
  window settings into an XmlElement, declare a Component that supports
  the interface, and finally provide a function that calls all windows
  to save their settings:

  @code

  // Interface for saving and loading Component settings across launches

  struct SerializableComponent
  {
    virtual void onSaveSettings (XmlElement* xml) { }
    virtual void onLoadSettings (XmlElement* xml) { }
  };

  // Declare our component

  class PersistentComponent : public SerializableComponent
  {
  public:
    void onSaveSettings (XmlElement* xmlTree)
    {
      // Save our state into xmlTree
    }
      
    void onLoadSettings (XmlElement* xmlTree)
    {
      // Retrieve our saved state from xmlTree if it exists
    }
  };

  // This will tell every component in every window to save
  // the settings if it supports the interface.

  void saveAllWindowSettings (XmlElement* xmlTree)
  {
    // Go through all application windows on the desktop.

    for (int i = 0; i < Desktop::getInstance().getNumComponents(); ++i)
    {
      Component* c = Desktop::getInstance().getComponent (i);

      // Call onSaveSettings for every Component that supports the
      // interface, on the window and all its children, recursively.

      componentBroadcast (c, &PersistentComponent::onSaveSettings, xmlTree);
    }
  }

  // Create a ResizableWindow and tell it to load it's settings

  void createWindowFromSettings (XmlElement* xmlTree)
  {
    // First create the window

    ResizableWindow* w = new ResizableWindow (...);

    // Now call onLoadSettings for every Component in the window
    // that supports the interface, recursively.

    componentBroadcast (w, &PersistentComponent::onLoadSettings, xmlTree);
  }

  @endcode

  componentBroadcast can even be used to call member functions of
  already existing interfaces, without changing any Component code! This
  example will automatically clear the contents of every child TextEditor
  Component in a DocumentWindow:

  @code

  void clearAllTextEditors (DocumentWindow* window)
  {
    componentBroadcast (window, &TextEditor::clear);
  }

  @endcode

  Wow!

  These are some of the benefits of using this system:

  - Code that broadcasts to the interface is generic. It doesn't need to
    know who is being called.

  - Components implementing the interface don't need to know about who
    is performing the broadcast.

  - Components don't care where they are in the TopLevelWindow's hierarchy,
    they can be added, removed, or reparented without affecting a broadcaster
    or other recipients.

  - If an individual child Component does not support the interface,
    it is simply skipped. This makes it easy to add broadcasting features
    to already written Component hierarchies.

  - Broadcast interfaces can be broken up into any number of individual
    classes, they don't need to know about each other. A Component can
    expose zero, one, or any number of interfaces to receive broadcasts.

  @ingroup vf_gui
*/
class componentBroadcast
{
public:
  /** Call a member function on a Component and all of it's children.

      @param c  The Component hierarchy to broadcast to.

      @param f  Any non-static class member to call. This may be followed by up
                to eight parameters. The parameters must match the signature
                of the specified function.
  */
  /** @{ */
  template <class C>
  componentBroadcast (Component* c, void (C::*f)())
    { call <C> (c, vf::bind (f, vf::_1)); }

  template <class C, class T1>
  componentBroadcast (Component* c, void (C::*f)(T1), T1 t1)
    { call <C> (c, vf::bind (f, vf::_1, t1)); }

  template <class C, class T1, class T2>
  componentBroadcast (Component* c, void (C::*f)(T1, T2), T1 t1, T2 t2)
    { call <C> (c, vf::bind (f, vf::_1, t1, t2)); }

  template <class C, class T1, class T2, class T3>
  componentBroadcast (Component* c, void (C::*f)(T1, T2, T3), T1 t1, T2 t2, T3 t3)
    { call <C> (c, vf::bind (f, vf::_1, t1, t2, t3)); }

  template <class C, class T1, class T2, class T3, class T4>
  componentBroadcast (Component* c, void (C::*f)(T1, T2, T3, T4), T1 t1, T2 t2, T3 t3, T4 t4)
    { call <C> (c, vf::bind (f, vf::_1, t1, t2, t3, t4)); }

  template <class C, class T1, class T2, class T3, class T4, class T5>
  componentBroadcast (Component* c, void (C::*f)(T1, T2, T3, T4, T5), T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
    { call <C> (c, vf::bind (f, vf::_1, t1, t2, t3, t4, t5)); }

  template <class C, class T1, class T2, class T3, class T4, class T5, class T6>
  componentBroadcast (Component* c, void (C::*f)(T1, T2, T3, T4, T5, T6),
             T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
    { call <C> (c, vf::bind (f, vf::_1, t1, t2, t3, t4, t5, t6)); }

  template <class C, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
  componentBroadcast (Component* c, void (C::*f)(T1, T2, T3, T4, T5, T6, T7),
             T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
    { call <C> (c, vf::bind (f, vf::_1, t1, t2, t3, t4, t5, t6, t7)); }

  template <class C, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
  componentBroadcast (Component* c, void (C::*f)(T1, T2, T3, T4, T5, T6, T7, T8),
             T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)
    { call <C> (c, vf::bind (f, vf::_1, t1, t2, t3, t4, t5, t6, t7, t8)); }
  /** @} */

private:
  template <class Interface, class Functor>
  void call (Component* component, Functor const& f)
  {
    Interface* const object = dynamic_cast <Interface*> (component);

    if (object != nullptr)
      f (object);

    for (int i = 0; i < component->getNumChildComponents (); ++i)
      call <Interface> (component->getChildComponent (i), f);
  }
};

#endif
